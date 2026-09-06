// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : rs485_battery_manager.cpp (Multidrop RS485 Battery Poller)
// DATO/TID: 2026-09-06 20:05:00
// =============================================================================

#include "rs485_battery_manager.h"
#include "deye_bms_decoder.h"
#include "system_config.h"

Rs485BatteryManager& Rs485BatteryManager::getInstance() {
    static Rs485BatteryManager instance;
    return instance;
}

Rs485BatteryManager::Rs485BatteryManager()
    : _initialized(false),
      _baudrate(9600),
      _active_protocol(RS485_PROTO_AUTO),
      _poll_state(0),
      _last_poll_ms(0),
      _last_p1_rx_ms(0),
      _last_p2_rx_ms(0),
      _rx_len(0),
      _rx_last_byte_ms(0),
      _pack1_online(false),
      _pack2_online(false),
      _tx_queries(0),
      _raw_rx_bytes(0),
      _rx_count(0),
      _err_count(0),
      _detect_try_count(0) {
    strncpy(_pack1_name, "RPT (ID 1)", sizeof(_pack1_name));
    strncpy(_pack2_name, "ROSEN (ID 2)", sizeof(_pack2_name));
    _last_tx_str[0] = '\0';
    _last_rx_hex_str[0] = '\0';
}

Rs485BatteryManager::~Rs485BatteryManager() {
}

void Rs485BatteryManager::begin(uint32_t baudrate) {
    _baudrate = baudrate;
    pinMode(BOARD_RS485_TX_PIN, OUTPUT);
    digitalWrite(BOARD_RS485_TX_PIN, HIGH); // Idle HIGH enables SP3485 receiver via S1
    pinMode(BOARD_RS485_RX_PIN, INPUT_PULLUP);
    Serial1.begin(_baudrate, SERIAL_8N1, BOARD_RS485_RX_PIN, BOARD_RS485_TX_PIN);
    _initialized = true;
    _poll_state = 0;
    _last_poll_ms = millis();
    LOG_PRINTF("[RS485] Initialized on TX=GPIO%d, RX=GPIO%d at %lu baud.\n",
               BOARD_RS485_TX_PIN, BOARD_RS485_RX_PIN, (unsigned long)_baudrate);
    aggregateTelemetry();
}

void Rs485BatteryManager::setBaudrate(uint32_t baud) {
    if (_baudrate == baud && _initialized) return;
    _baudrate = baud;
    Serial1.end();
    pinMode(BOARD_RS485_TX_PIN, OUTPUT);
    digitalWrite(BOARD_RS485_TX_PIN, HIGH);
    pinMode(BOARD_RS485_RX_PIN, INPUT_PULLUP);
    Serial1.begin(_baudrate, SERIAL_8N1, BOARD_RS485_RX_PIN, BOARD_RS485_TX_PIN);
    _rx_len = 0;
    LOG_PRINTF("[RS485] Baudrate set to %lu baud.\n", (unsigned long)_baudrate);
    aggregateTelemetry();
}

void Rs485BatteryManager::cycleBaudrate() {
    if (_baudrate == 9600) setBaudrate(115200);
    else if (_baudrate == 115200) setBaudrate(19200);
    else setBaudrate(9600);
}

void Rs485BatteryManager::setProtocol(Rs485ProtocolType proto) {
    _active_protocol = proto;
    aggregateTelemetry();
}

void Rs485BatteryManager::triggerImmediatePoll() {
    _poll_state = 0;
    _last_poll_ms = 0;
}

uint16_t Rs485BatteryManager::calcPylonChecksum(const char* buf, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += (uint8_t)buf[i];
    }
    sum = sum % 65536;
    sum = (~sum + 1) & 0xFFFF;
    return (uint16_t)sum;
}

uint16_t Rs485BatteryManager::calcModbusCrc(const uint8_t* buf, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Rs485BatteryManager::sendPylonAsciiQuery(uint8_t addr, uint8_t cid2) {
    // Pylontech frame format: ~20 <ADR_HEX> 46 <CID2_HEX> 0000 <CHKSUM_HEX>\r
    char query[32];
    char rawPayload[16];
    snprintf(rawPayload, sizeof(rawPayload), "20%02X46%02X0000", addr, cid2);

    uint16_t chk = calcPylonChecksum(rawPayload, strlen(rawPayload));
    snprintf(query, sizeof(query), "~%s%04X\r", rawPayload, chk);

    while (Serial1.available()) Serial1.read(); // Clear RX FIFO
    _rx_len = 0;

    Serial1.print(query);
    Serial1.flush();

    _tx_queries++;
    strncpy(_last_tx_str, query, sizeof(_last_tx_str) - 1);
    _last_tx_str[sizeof(_last_tx_str) - 1] = '\0';
}

void Rs485BatteryManager::sendModbusRtuQuery(uint8_t addr, uint16_t startReg, uint16_t regCount) {
    uint8_t frame[8];
    frame[0] = addr;
    frame[1] = 0x03; // Read Holding Registers
    frame[2] = (startReg >> 8) & 0xFF;
    frame[3] = startReg & 0xFF;
    frame[4] = (regCount >> 8) & 0xFF;
    frame[5] = regCount & 0xFF;

    uint16_t crc = calcModbusCrc(frame, 6);
    frame[6] = crc & 0xFF;        // CRC Lo
    frame[7] = (crc >> 8) & 0xFF; // CRC Hi

    while (Serial1.available()) Serial1.read(); // Clear RX FIFO
    _rx_len = 0;

    Serial1.write(frame, 8);
    Serial1.flush();

    _tx_queries++;
    snprintf(_last_tx_str, sizeof(_last_tx_str), "%02X 03 %04X %04X %04X", addr, startReg, regCount, crc);
}

void Rs485BatteryManager::update() {
    if (!_initialized) return;

    processIncomingBytes();

    uint32_t now = millis();

    // Check online timeouts (3500ms without response)
    if (_pack1_online && (now - _last_p1_rx_ms > 3500)) {
        _pack1_online = false;
        LOG_PRINTLN("[RS485] Battery Pack 1 (ID 1) communication TIMEOUT.");
        aggregateTelemetry();
    }
    if (_pack2_online && (now - _last_p2_rx_ms > 3500)) {
        _pack2_online = false;
        LOG_PRINTLN("[RS485] Battery Pack 2 (ID 2) communication TIMEOUT.");
        aggregateTelemetry();
    }

    // Periodic update of live diagnostic metrics to BatteryData model
    static uint32_t s_last_diag_agg_ms = 0;
    if (now - s_last_diag_agg_ms >= 300) {
        s_last_diag_agg_ms = now;
        aggregateTelemetry();
    }

    // State machine for polling Pack 1 and Pack 2
    switch (_poll_state) {
        case 0: // Query Pack 1 (ID 1 - RPT 300Ah)
            if (now - _last_poll_ms >= (_pack1_online && !_pack2_online ? 350 : 500)) {
                _last_poll_ms = now;
                if (_active_protocol == RS485_PROTO_MODBUS_RTU) {
                    sendModbusRtuQuery(1, 0x0000, 20);
                } else {
                    sendPylonAsciiQuery(1, 0x42); // Get Analog Data
                }
                _poll_state = 1; // Wait for response
            }
            break;

        case 1: // Wait Pack 1 response
            if (now - _last_poll_ms >= 200) {
                _poll_state = 2;
                _last_poll_ms = now;
            }
            break;

        case 2: // Query Pack 2 (ID 2 - Rosen) or probe alternative ID 0 if offline
            if (now - _last_poll_ms >= 80) {
                _last_poll_ms = now;
                uint8_t targetAddr = 2;
                if (!_pack1_online && !_pack2_online) {
                    static uint8_t s_alt_toggle = 0;
                    s_alt_toggle = (s_alt_toggle == 0) ? 2 : 0;
                    targetAddr = s_alt_toggle;
                }
                if (_active_protocol == RS485_PROTO_MODBUS_RTU) {
                    sendModbusRtuQuery(targetAddr == 0 ? 1 : targetAddr, 0x0000, 20);
                } else {
                    sendPylonAsciiQuery(targetAddr, 0x42);
                }
                _poll_state = 3; // Wait for response
            }
            break;

        case 3: // Wait Pack 2 response
            if (now - _last_poll_ms >= 200) {
                _poll_state = 0; // Ready for next cycle
                _last_poll_ms = now;

                // Auto-protocol and baudrate probing only if zero valid responses ever received
                if (!_pack1_online && !_pack2_online && _rx_count == 0) {
                    _detect_try_count++;
                    if (_detect_try_count >= 8) {
                        _detect_try_count = 0;
                        // Cycle protocol
                        if (_active_protocol == RS485_PROTO_AUTO || _active_protocol == RS485_PROTO_PYLON_ASCII) {
                            _active_protocol = RS485_PROTO_MODBUS_RTU;
                            LOG_PRINTLN("[RS485] Auto-detect: Switching probe to Modbus RTU...");
                        } else {
                            _active_protocol = RS485_PROTO_PYLON_ASCII;
                            LOG_PRINTLN("[RS485] Auto-detect: Switching probe to Pylontech ASCII...");
                            // If also zero raw bytes, try toggling baudrate between 9600 and 115200
                            if (_raw_rx_bytes == 0) {
                                cycleBaudrate();
                            }
                        }
                    }
                }
            }
            break;
    }
}

void Rs485BatteryManager::processIncomingBytes() {
    while (Serial1.available()) {
        uint8_t b = Serial1.read();
        _raw_rx_bytes++;
        _rx_last_byte_ms = millis();

        // Keep last 10 bytes in _last_rx_hex_str for live display on Page 4
        static uint8_t recentBytes[12];
        static uint8_t recentCount = 0;
        if (recentCount < 12) {
            recentBytes[recentCount++] = b;
        } else {
            memmove(recentBytes, recentBytes + 1, 11);
            recentBytes[11] = b;
        }
        char hexDump[48] = "";
        for (uint8_t i = 0; i < recentCount; i++) {
            char hb[5];
            snprintf(hb, sizeof(hb), "%02X ", recentBytes[i]);
            strncat(hexDump, hb, sizeof(hexDump) - strlen(hexDump) - 1);
        }
        strncpy(_last_rx_hex_str, hexDump, sizeof(_last_rx_hex_str) - 1);

        if (_rx_len < sizeof(_rx_buffer) - 1) {
            _rx_buffer[_rx_len++] = b;
            _rx_buffer[_rx_len] = '\0';
        }

        // Check for Pylontech ASCII terminator ('\r' = 0x0D or '\n' = 0x0A)
        if (b == '\r' || b == '\n') {
            const char* tilde = strchr((const char*)_rx_buffer, '~');
            if (tilde) {
                if (parsePylonAsciiFrame(tilde, strlen(tilde))) {
                    _rx_count++;
                    _rx_len = 0;
                    return;
                }
            }
            _rx_len = 0;
        }
    }

    // Check for Modbus RTU frame end by inter-frame silence (>= 15ms silence)
    if (_rx_len >= 5 && (millis() - _rx_last_byte_ms >= 15)) {
        if (parseModbusRtuFrame(_rx_buffer, _rx_len)) {
            _rx_count++;
        }
        _rx_len = 0;
    }
}

static uint8_t hex2nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

static uint16_t parseHex16(const char* s) {
    return ((uint16_t)hex2nibble(s[0]) << 12) |
           ((uint16_t)hex2nibble(s[1]) << 8)  |
           ((uint16_t)hex2nibble(s[2]) << 4)  |
           ((uint16_t)hex2nibble(s[3]));
}

static uint8_t parseHex8(const char* s) {
    return (hex2nibble(s[0]) << 4) | hex2nibble(s[1]);
}

static uint32_t parseHex24(const char* s) {
    return ((uint32_t)hex2nibble(s[0]) << 20) |
           ((uint32_t)hex2nibble(s[1]) << 16) |
           ((uint32_t)hex2nibble(s[2]) << 12) |
           ((uint32_t)hex2nibble(s[3]) << 8)  |
           ((uint32_t)hex2nibble(s[4]) << 4)  |
           ((uint32_t)hex2nibble(s[5]));
}

bool Rs485BatteryManager::parsePylonAsciiFrame(const char* frame, size_t len) {
    // Strip trailing \r and \n
    while (len > 0 && (frame[len - 1] == '\r' || frame[len - 1] == '\n')) {
        len--;
    }
    if (len < 17 || frame[0] != '~') return false;

    // Verify Checksum: raw payload is frame[1] through frame[len - 5], chk is frame[len - 4 .. len - 1]
    const char* raw = &frame[1];
    size_t rawLen = len - 5;
    uint16_t receivedChk = parseHex16(&frame[len - 4]);
    uint16_t calculatedChk = calcPylonChecksum(raw, rawLen);

    if (receivedChk != calculatedChk) {
        _err_count++;
        return false;
    }

    _active_protocol = RS485_PROTO_PYLON_ASCII;

    // Extract Address and Return Code
    uint8_t addr = parseHex8(&frame[3]);
    uint8_t returnCode = parseHex8(&frame[7]);
    if (returnCode != 0x00) {
        return false; // Error response from battery
    }

    // INFO field begins at frame[13]
    if (len < 19) return false;

    // frame[13..14]: INFOFLAG
    uint8_t infoFlag = parseHex8(&frame[13]);

    // frame[15..16]: Module ID / NumberOfModules (01 = Module 1, 02 = Module 2)
    uint8_t moduleId = parseHex8(&frame[15]);

    // frame[17..18]: NumberOfCells (0x10 = 16 cells)
    uint8_t cellCount = parseHex8(&frame[17]);
    if (cellCount == 0 || cellCount > 16) cellCount = 16;

    // Cell voltages start at frame[19]
    const char* p = &frame[19];
    float cellVoltages[16];
    float minV = 5.0f, maxV = 0.0f;
    float cellSum = 0.0f;

    for (int i = 0; i < 16; i++) {
        if (i < cellCount) {
            uint16_t rawMv = parseHex16(p);
            p += 4;
            float v = rawMv * 0.001f;
            if (v < 1.5f || v > 4.5f) {
                v = 3.310f; // LiFePO4 safety fallback
            }
            cellVoltages[i] = v;
            cellSum += v;
            if (v < minV) minV = v;
            if (v > maxV) maxV = v;
        } else {
            cellVoltages[i] = (minV + maxV) * 0.5f;
            cellSum += cellVoltages[i];
        }
    }

    // Number of temperatures (1 byte = 2 hex chars)
    uint8_t tempCount = parseHex8(p);
    p += 2;
    float temps[4] = { 20.0f, 20.0f, 20.0f, 20.0f };
    for (int i = 0; i < tempCount; i++) {
        uint16_t rawT = parseHex16(p);
        p += 4;
        float degC = (rawT - 2731) * 0.1f;
        if (i < 4) temps[i] = degC;
    }

    // Current (signed 16-bit, in 0.1A)
    int16_t rawCurrent = (int16_t)parseHex16(p);
    p += 4;
    float currentA = rawCurrent * 0.1f;

    // Voltage (unsigned 16-bit in mV)
    uint16_t rawV = parseHex16(p);
    p += 4;
    float voltageV = rawV * 0.001f;
    // LiFePO4 16S pack voltage sanity check: if raw voltage is out of range, use exact cell sum
    if (voltageV < 30.0f || voltageV > 65.0f) {
        voltageV = cellSum;
    }

    // Remaining capacity (16-bit, in mAh -> divide by 1000 for Ah)
    uint16_t remMah16 = parseHex16(p);
    p += 4;
    float remainAh = remMah16 * 0.001f;

    // User Defined Items (1 byte = 2 hex chars)
    uint8_t userItems = parseHex8(p);
    p += 2;

    // Total capacity (16-bit, in mAh -> divide by 1000 for Ah)
    uint16_t totalMah16 = parseHex16(p);
    p += 4;
    float totalAh = totalMah16 * 0.001f;

    // Cycle number (16-bit)
    uint16_t cycleCount = parseHex16(p);
    p += 4;

    // High capacity batteries (> 65Ah, such as 300Ah / 200Ah) report 24-bit values
    if (userItems > 2) {
        uint32_t remMah24 = parseHex24(p);
        p += 6;
        uint32_t totalMah24 = parseHex24(p);
        p += 6;
        remainAh = remMah24 * 0.001f;
        totalAh = totalMah24 * 0.001f;
    }

    // Fallback capacity if reported value is zero
    if (totalAh < 10.0f) {
        if (addr == 1 || moduleId == 1) totalAh = 300.0f;
        else totalAh = 200.0f;
    }
    if (remainAh > totalAh) remainAh = totalAh;

    uint16_t soc = 100;
    if (totalAh > 0.0f) {
        soc = (uint16_t)((remainAh / totalAh) * 100.0f + 0.5f);
    }
    if (soc > 100) soc = 100;

    // Pack Identification:
    // ADR=1 or Module=1 or capacity >= 250Ah -> Pack 1 (RPT 300Ah)
    // ADR=2 or Module=2 or capacity < 250Ah  -> Pack 2 (ROSEN 200Ah)
    bool isPack1 = false;
    if (addr == 1 || moduleId == 1) {
        isPack1 = true;
    } else if (addr == 2 || moduleId == 2) {
        isPack1 = false;
    } else if (totalAh >= 250.0f) {
        isPack1 = true;
    } else if (!_pack1_online && _pack2_online) {
        isPack1 = true;
    } else {
        isPack1 = false;
    }

    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    if (isPack1) {
        _pack1_online = true;
        _last_p1_rx_ms = millis();
        memcpy(bData.pack1_cellVoltages, cellVoltages, sizeof(cellVoltages));
        bData.pack1_minV = minV;
        bData.pack1_maxV = maxV;
        bData.pack1_soc_percent = soc;
        bData.pack1_current_A = currentA;
        bData.pack1_power_W = voltageV * currentA;
        bData.pack1_capacity_Ah = totalAh > 50.0f ? totalAh : 300.0f;
        bData.pack1_online = true;
        bData.pack1_cells_valid = true;
        strncpy(_pack1_name, "RPT (300Ah)", sizeof(_pack1_name));
        strncpy(bData.pack1_name, _pack1_name, sizeof(bData.pack1_name));
    } else {
        _pack2_online = true;
        _last_p2_rx_ms = millis();
        memcpy(bData.pack2_cellVoltages, cellVoltages, sizeof(cellVoltages));
        bData.pack2_minV = minV;
        bData.pack2_maxV = maxV;
        bData.pack2_soc_percent = soc;
        bData.pack2_current_A = currentA;
        bData.pack2_power_W = voltageV * currentA;
        bData.pack2_capacity_Ah = totalAh > 50.0f ? totalAh : 200.0f;
        bData.pack2_online = true;
        bData.pack2_cells_valid = true;
        strncpy(_pack2_name, "ROSEN (200Ah)", sizeof(_pack2_name));
        strncpy(bData.pack2_name, _pack2_name, sizeof(bData.pack2_name));
    }

    DeyeBmsDecoder::getInstance().setBatteryData(bData);
    aggregateTelemetry();
    return true;
}

bool Rs485BatteryManager::parseModbusRtuFrame(const uint8_t* buffer, size_t len) {
    if (len < 7) return false;

    // Verify CRC16
    uint16_t receivedCrc = buffer[len - 2] | ((uint16_t)buffer[len - 1] << 8);
    uint16_t calculatedCrc = calcModbusCrc(buffer, len - 2);

    if (receivedCrc != calculatedCrc) {
        _err_count++;
        return false;
    }

    uint8_t addr = buffer[0];
    uint8_t func = buffer[1];
    uint8_t byteCount = buffer[2];

    if (func != 0x03 && func != 0x04) return false;
    if (byteCount < 32 || (len < (size_t)(byteCount + 5))) return false;

    _active_protocol = RS485_PROTO_MODBUS_RTU;

    // Parse 16 cell voltages (each register is 16-bit mV)
    float cellVoltages[16];
    float minV = 5.0f, maxV = 0.0f;
    for (int i = 0; i < 16; i++) {
        uint16_t rawMv = ((uint16_t)buffer[3 + i * 2] << 8) | buffer[4 + i * 2];
        float v = rawMv * 0.001f;
        if (v < 2.0f || v > 4.5f) v = 3.37f;
        cellVoltages[i] = v;
        if (v < minV) minV = v;
        if (v > maxV) maxV = v;
    }

    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    if (addr == 1 || !_pack2_online) {
        _pack1_online = true;
        _last_p1_rx_ms = millis();
        memcpy(bData.pack1_cellVoltages, cellVoltages, sizeof(cellVoltages));
        bData.pack1_minV = minV;
        bData.pack1_maxV = maxV;
        bData.pack1_online = true;
        bData.pack1_cells_valid = true;
        strncpy(_pack1_name, "RPT (300Ah)", sizeof(_pack1_name));
        strncpy(bData.pack1_name, _pack1_name, sizeof(bData.pack1_name));
    } else {
        _pack2_online = true;
        _last_p2_rx_ms = millis();
        memcpy(bData.pack2_cellVoltages, cellVoltages, sizeof(cellVoltages));
        bData.pack2_minV = minV;
        bData.pack2_maxV = maxV;
        bData.pack2_online = true;
        bData.pack2_cells_valid = true;
        strncpy(_pack2_name, "ROSEN (200Ah)", sizeof(_pack2_name));
        strncpy(bData.pack2_name, _pack2_name, sizeof(bData.pack2_name));
    }

    DeyeBmsDecoder::getInstance().setBatteryData(bData);
    aggregateTelemetry();
    return true;
}

void Rs485BatteryManager::aggregateTelemetry() {
    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    bData.rs485_online = (_pack1_online || _pack2_online);
    bData.rs485_baud = _baudrate;
    bData.rs485_tx_queries = _tx_queries;
    bData.rs485_rx_raw_bytes = _raw_rx_bytes;
    bData.rs485_rx_count = _rx_count;
    bData.rs485_err_count = _err_count;
    strncpy(bData.rs485_last_tx, _last_tx_str, sizeof(bData.rs485_last_tx));
    strncpy(bData.rs485_last_rx_hex, _last_rx_hex_str, sizeof(bData.rs485_last_rx_hex));
    if (_active_protocol == RS485_PROTO_MODBUS_RTU) {
        strncpy(bData.rs485_proto_name, "Modbus RTU", sizeof(bData.rs485_proto_name));
    } else if (_active_protocol == RS485_PROTO_PYLON_ASCII) {
        strncpy(bData.rs485_proto_name, "Pylontech ASCII", sizeof(bData.rs485_proto_name));
    } else {
        strncpy(bData.rs485_proto_name, "Auto-Detecting", sizeof(bData.rs485_proto_name));
    }

    if (_pack1_online && _pack2_online) {
        // Both packs online: True 32-cell combined bank metrics
        float globalMin = bData.pack1_minV < bData.pack2_minV ? bData.pack1_minV : bData.pack2_minV;
        float globalMax = bData.pack1_maxV > bData.pack2_maxV ? bData.pack1_maxV : bData.pack2_maxV;
        bData.minCellVoltage_V = globalMin;
        bData.maxCellVoltage_V = globalMax;
        bData.cellDelta_mV     = (globalMax - globalMin) * 1000.0f;

        float vSum1 = 0.0f, vSum2 = 0.0f;
        for (int i = 0; i < 16; i++) {
            vSum1 += bData.pack1_cellVoltages[i];
            vSum2 += bData.pack2_cellVoltages[i];
        }
        bData.voltage_V = (vSum1 > 10.0f && vSum2 > 10.0f) ? (vSum1 + vSum2) * 0.5f : (vSum1 > 10.0f ? vSum1 : vSum2);
        memcpy(bData.cellVoltages, bData.pack1_cellVoltages, sizeof(bData.cellVoltages));

        bData.current_A = bData.pack1_current_A + bData.pack2_current_A;
        bData.power_W   = bData.voltage_V * bData.current_A;

        float cap1 = bData.pack1_capacity_Ah > 0 ? bData.pack1_capacity_Ah : 300.0f;
        float cap2 = bData.pack2_capacity_Ah > 0 ? bData.pack2_capacity_Ah : 200.0f;
        bData.totalCapacity_Ah = (uint16_t)(cap1 + cap2 + 0.5f);
        bData.soc_percent = (uint16_t)(((bData.pack1_soc_percent * cap1) + (bData.pack2_soc_percent * cap2)) / (cap1 + cap2) + 0.5f);
        bData.moduleCount = 2;
        bData.chargeVoltageLimit_V = 56.8f;
        bData.dischargeCutoffVoltage_V = 47.0f;
        bData.chargeCurrentLimit_A = 250.0f;
        bData.dischargeCurrentLimit_A = 250.0f;

        bData.individualCellsKnown = true;
        bData.communicationOK = true;
    } else if (_pack1_online) {
        // Single RPT (300Ah) pack online
        bData.minCellVoltage_V = bData.pack1_minV;
        bData.maxCellVoltage_V = bData.pack1_maxV;
        bData.cellDelta_mV     = (bData.pack1_maxV - bData.pack1_minV) * 1000.0f;

        float vSum = 0.0f;
        for (int i = 0; i < 16; i++) {
            vSum += bData.pack1_cellVoltages[i];
        }
        bData.voltage_V = (vSum > 10.0f) ? vSum : 51.2f;
        memcpy(bData.cellVoltages, bData.pack1_cellVoltages, sizeof(bData.cellVoltages));

        bData.current_A        = bData.pack1_current_A;
        bData.power_W          = bData.voltage_V * bData.current_A;
        bData.soc_percent      = bData.pack1_soc_percent;
        bData.totalCapacity_Ah = bData.pack1_capacity_Ah > 0 ? (uint16_t)bData.pack1_capacity_Ah : 300;
        bData.moduleCount      = 1;
        bData.chargeVoltageLimit_V = 56.8f;
        bData.dischargeCutoffVoltage_V = 47.0f;
        bData.chargeCurrentLimit_A = 150.0f;
        bData.dischargeCurrentLimit_A = 150.0f;

        bData.individualCellsKnown = true;
        bData.communicationOK = true;
    } else if (_pack2_online) {
        // Single Rosen (200Ah) pack online
        bData.minCellVoltage_V = bData.pack2_minV;
        bData.maxCellVoltage_V = bData.pack2_maxV;
        bData.cellDelta_mV     = (bData.pack2_maxV - bData.pack2_minV) * 1000.0f;

        float vSum = 0.0f;
        for (int i = 0; i < 16; i++) {
            vSum += bData.pack2_cellVoltages[i];
        }
        bData.voltage_V = (vSum > 10.0f) ? vSum : 51.2f;
        memcpy(bData.cellVoltages, bData.pack2_cellVoltages, sizeof(bData.cellVoltages));

        bData.current_A        = bData.pack2_current_A;
        bData.power_W          = bData.voltage_V * bData.current_A;
        bData.soc_percent      = bData.pack2_soc_percent;
        bData.totalCapacity_Ah = bData.pack2_capacity_Ah > 0 ? (uint16_t)bData.pack2_capacity_Ah : 200;
        bData.moduleCount      = 1;
        bData.chargeVoltageLimit_V = 56.8f;
        bData.dischargeCutoffVoltage_V = 47.0f;
        bData.chargeCurrentLimit_A = 100.0f;
        bData.dischargeCurrentLimit_A = 100.0f;

        bData.individualCellsKnown = true;
        bData.communicationOK = true;
    }

    DeyeBmsDecoder::getInstance().setBatteryData(bData);
}
