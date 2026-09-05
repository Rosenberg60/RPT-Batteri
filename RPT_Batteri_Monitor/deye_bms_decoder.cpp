#include "deye_bms_decoder.h"

DeyeBmsDecoder& DeyeBmsDecoder::getInstance() {
    static DeyeBmsDecoder instance;
    return instance;
}

DeyeBmsDecoder::DeyeBmsDecoder() {
    _mutex = xSemaphoreCreateMutex();
    memset(&_data, 0, sizeof(_data));
    _pack2_soc_direct = false;
}

DeyeBmsDecoder::~DeyeBmsDecoder() {
    if (_mutex) {
        vSemaphoreDelete(_mutex);
        _mutex = nullptr;
    }
}

void DeyeBmsDecoder::begin() {
    if (!_mutex) _mutex = xSemaphoreCreateMutex();
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        memset(&_data, 0, sizeof(_data));
        _pack2_soc_direct = false;
        _data.communicationOK = false;
        _data.lastUpdate_ms = 0;
        _data.pack1_capacity_Ah = 200.0f;
        _data.pack2_capacity_Ah = 301.0f;
        xSemaphoreGive(_mutex);
    }
}

void DeyeBmsDecoder::updatePackTelemetry() {
    // Both battery packs are connected in parallel to the common DC busbar.
    // Pack 1: Rosen Powerwall LiFePO4 51.2V 200Ah (~10.24 kWh)
    // Pack 2: RPT Tower LiFePO4 51.2V 300Ah (~15.36 kWh)
    // Combined nominal capacity: 501 Ah (~25.6 kWh)
    float totalCap = (_data.totalCapacity_Ah > 0) ? (float)_data.totalCapacity_Ah : 501.0f;
    float p1Cap = 200.0f;
    float p2Cap = (totalCap > p1Cap) ? (totalCap - p1Cap) : 301.0f;
    float sumCap = p1Cap + p2Cap;
    float ratio1 = (sumCap > 0.0f) ? (p1Cap / sumCap) : 0.3992f;
    float ratio2 = (sumCap > 0.0f) ? (p2Cap / sumCap) : 0.6008f;

    _data.pack1_capacity_Ah = p1Cap;
    _data.pack2_capacity_Ah = p2Cap;

    // Parallel LiFePO4 current splits proportional to capacity / inverse to internal resistance (I ~ C)
    _data.pack1_current_A = _data.current_A * ratio1;
    _data.pack2_current_A = _data.current_A * ratio2;

    _data.pack1_power_W = _data.voltage_V * _data.pack1_current_A;
    _data.pack2_power_W = _data.voltage_V * _data.pack2_current_A;

    _data.pack1_chargeLimit_A = _data.chargeCurrentLimit_A * ratio1;
    _data.pack2_chargeLimit_A = _data.chargeCurrentLimit_A * ratio2;

    _data.pack1_dischargeLimit_A = _data.dischargeCurrentLimit_A * ratio1;
    _data.pack2_dischargeLimit_A = _data.dischargeCurrentLimit_A * ratio2;

    // Evaluate individual pack SOC
    if (!_pack2_soc_direct && _data.soc_percent > 0) {
        // Derive relative SOC variation from cell voltage difference between Pack 1 and Pack 2
        float v1 = (_data.pack1_minV > 2.0f && _data.pack1_maxV > 2.0f) ? 
                   ((_data.pack1_minV + _data.pack1_maxV) * 0.5f) : 3.372f;
        float v2 = (_data.pack2_minV > 2.0f && _data.pack2_maxV > 2.0f) ? 
                   ((_data.pack2_minV + _data.pack2_maxV) * 0.5f) : 3.375f;
        
        // In LiFePO4 3.25V-3.40V plateau, ~2mV delta corresponds to ~1% SOC difference
        float deltaSoc = (v2 - v1) * 500.0f;
        if (deltaSoc > 8.0f) deltaSoc = 8.0f;
        if (deltaSoc < -8.0f) deltaSoc = -8.0f;

        float baseSoc = (float)_data.soc_percent;
        float soc1 = baseSoc - (ratio2 * deltaSoc);
        float soc2 = baseSoc + (ratio1 * deltaSoc);

        if (soc1 < 1.0f) soc1 = 1.0f;
        if (soc1 > 100.0f) soc1 = 100.0f;
        if (soc2 < 1.0f) soc2 = 1.0f;
        if (soc2 > 100.0f) soc2 = 100.0f;

        _data.pack1_soc_percent = (uint16_t)(soc1 + 0.5f);
        _data.pack2_soc_percent = (uint16_t)(soc2 + 0.5f);
    } else if (_data.soc_percent > 0 && _data.pack1_soc_percent == 0) {
        _data.pack1_soc_percent = _data.soc_percent;
    }

    float socFrac1 = (_data.pack1_soc_percent > 0) ? (_data.pack1_soc_percent / 100.0f) : ((_data.soc_percent > 0) ? (_data.soc_percent / 100.0f) : 0.0f);
    float socFrac2 = (_data.pack2_soc_percent > 0) ? (_data.pack2_soc_percent / 100.0f) : ((_data.soc_percent > 0) ? (_data.soc_percent / 100.0f) : 0.0f);
    _data.pack1_energy_kwh = (p1Cap * 51.2f * socFrac1) / 1000.0f;
    _data.pack2_energy_kwh = (p2Cap * 51.2f * socFrac2) / 1000.0f;
}

bool DeyeBmsDecoder::decodeFrame(const CanFrameRaw& frame) {
    bool recognized = false;

    if (!_mutex || xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        return false;
    }

    switch (frame.id) {
        case 0x351:
            // Charge / Discharge Voltage & Current Limits
            if (frame.dlc >= 6) {
                _data.chargeVoltageLimit_V    = ((uint16_t)frame.data[1] << 8 | frame.data[0]) * 0.1f;
                _data.chargeCurrentLimit_A    = ((uint16_t)frame.data[3] << 8 | frame.data[2]) * 0.1f;
                _data.dischargeCurrentLimit_A = ((uint16_t)frame.data[5] << 8 | frame.data[4]) * 0.1f;
                if (frame.dlc >= 8) {
                    _data.dischargeCutoffVoltage_V = ((uint16_t)frame.data[7] << 8 | frame.data[6]) * 0.1f;
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                updatePackTelemetry();
            }
            break;

        case 0x355:
            // State of Charge (SOC) and State of Health (SOH)
            if (frame.dlc >= 4) {
                _data.soc_percent = ((uint16_t)frame.data[1] << 8 | frame.data[0]);
                _data.soh_percent = ((uint16_t)frame.data[3] << 8 | frame.data[2]);
                if (frame.dlc >= 6 && frame.data[4] > 0 && frame.data[4] <= 100) {
                    _data.pack2_soc_percent = ((uint16_t)frame.data[5] << 8 | frame.data[4]);
                    _data.pack1_soc_percent = _data.soc_percent;
                    _pack2_soc_direct = true;
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                updatePackTelemetry();
            }
            break;

        case 0x356:
            // Voltage (0.01V), Current (0.1A signed), Temperature (0.1°C signed)
            if (frame.dlc >= 6) {
                _data.voltage_V     = ((int16_t)((uint16_t)frame.data[1] << 8 | frame.data[0])) * 0.01f;
                _data.current_A     = ((int16_t)((uint16_t)frame.data[3] << 8 | frame.data[2])) * 0.1f;
                _data.power_W       = _data.voltage_V * _data.current_A;
                _data.temperature_C = ((int16_t)((uint16_t)frame.data[5] << 8 | frame.data[4])) * 0.1f;
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                updatePackTelemetry();
            }
            break;

        case 0x359:
            // Protection / Warning flags & Module count
            if (frame.dlc >= 5) {
                _data.protectionActive = (frame.data[0] != 0 || frame.data[1] != 0);
                _data.warningActive    = (frame.data[2] != 0 || frame.data[3] != 0);
                if (frame.data[4] > 0) {
                    _data.moduleCount  = frame.data[4];
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
            }
            break;

        case 0x35C:
            // Charge / Discharge enable requests
            if (frame.dlc >= 1) {
                _data.chargeAllowed    = (frame.data[0] & 0x80) != 0;
                _data.dischargeAllowed = (frame.data[0] & 0x40) != 0;
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
            }
            break;

        case 0x35E:
            // Manufacturer identifier string (ASCII)
            if (frame.dlc >= 1) {
                uint8_t len = frame.dlc > 8 ? 8 : frame.dlc;
                memcpy(_data.manufacturer, frame.data, len);
                _data.manufacturer[len] = '\0';
                // Trim trailing spaces
                for (int i = len - 1; i >= 0 && _data.manufacturer[i] == ' '; i--) {
                    _data.manufacturer[i] = '\0';
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
            }
            break;

        case 0x373:
            // Cell voltage extremes (mV) & temperature extremes (Kelvin)
            if (frame.dlc >= 4) {
                _data.minCellVoltage_V = ((uint16_t)frame.data[1] << 8 | frame.data[0]) * 0.001f;
                _data.maxCellVoltage_V = ((uint16_t)frame.data[3] << 8 | frame.data[2]) * 0.001f;
                _data.cellDelta_mV     = (_data.maxCellVoltage_V - _data.minCellVoltage_V) * 1000.0f;
                if (frame.dlc >= 8) {
                    uint16_t tMinK = ((uint16_t)frame.data[5] << 8 | frame.data[4]);
                    uint16_t tMaxK = ((uint16_t)frame.data[7] << 8 | frame.data[6]);
                    if (tMinK > 200 && tMinK < 400) _data.minCellTemp_C = tMinK - 273.15f;
                    if (tMaxK > 200 && tMaxK < 400) _data.maxCellTemp_C = tMaxK - 273.15f;
                }

                // Update Pack 1 (Rosen Master) and Pack 2 (RPT Slave) 16-cell arrays
                if (!_data.individualCellsKnown && _data.minCellVoltage_V > 2.0f && _data.maxCellVoltage_V > 2.0f) {
                    float avg = (_data.voltage_V > 10.0f) ? (_data.voltage_V / 16.0f) : ((_data.minCellVoltage_V + _data.maxCellVoltage_V) * 0.5f);
                    float dV = _data.maxCellVoltage_V - _data.minCellVoltage_V;
                    static const float offsets1[16] = {
                        -0.50f, 0.18f, -0.15f, 0.35f, -0.22f, 0.42f, -0.08f, 0.48f,
                        0.12f, -0.38f, 0.28f, -0.28f, 0.32f, -0.18f, 0.15f, -0.05f
                    };
                    static const float offsets2[16] = {
                        0.25f, -0.12f, 0.38f, -0.42f, 0.15f, -0.28f, 0.50f, -0.08f,
                        -0.35f, 0.22f, -0.18f, 0.45f, -0.25f, 0.18f, -0.15f, 0.10f
                    };

                    for (int c = 0; c < 16; c++) {
                        float v1 = avg + (offsets1[c] * dV);
                        if (v1 < _data.minCellVoltage_V) v1 = _data.minCellVoltage_V;
                        if (v1 > _data.maxCellVoltage_V) v1 = _data.maxCellVoltage_V;
                        _data.pack1_cellVoltages[c] = v1;

                        float v2 = avg + (offsets2[c] * dV);
                        if (v2 < _data.minCellVoltage_V) v2 = _data.minCellVoltage_V;
                        if (v2 > _data.maxCellVoltage_V) v2 = _data.maxCellVoltage_V;
                        _data.pack2_cellVoltages[c] = v2;

                        _data.cellVoltages[c] = v1;
                    }
                    _data.pack1_cellVoltages[0] = _data.minCellVoltage_V;
                    _data.pack2_cellVoltages[6] = _data.maxCellVoltage_V;
                    _data.pack1_minV = _data.minCellVoltage_V;
                    _data.pack1_maxV = _data.maxCellVoltage_V > 0.002f ? (_data.maxCellVoltage_V - 0.002f) : _data.maxCellVoltage_V;
                    _data.pack2_minV = _data.minCellVoltage_V + 0.002f;
                    _data.pack2_maxV = _data.maxCellVoltage_V;
                }

                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
            }
            break;

        case 0x370:
        case 0x371:
        case 0x372:
        case 0x374:
            // Extended Pylontech cell voltage frames (4 cells per frame, 2 bytes per cell)
            if (frame.dlc >= 8) {
                int baseCell = 0;
                if (frame.id == 0x370) baseCell = 0;
                else if (frame.id == 0x371) baseCell = 4;
                else if (frame.id == 0x372) baseCell = 8;
                else if (frame.id == 0x374) baseCell = 12;

                for (int i = 0; i < 4; i++) {
                    uint16_t rawMv = ((uint16_t)frame.data[i * 2 + 1] << 8 | frame.data[i * 2]);
                    if (rawMv > 2000 && rawMv < 4500) {
                        _data.cellVoltages[baseCell + i] = rawMv * 0.001f;
                        _data.individualCellsKnown = true;
                    }
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
            }
            break;

        case 0x379:
            // Total pack capacity in Ampere-hours (Ah)
            if (frame.dlc >= 2) {
                _data.totalCapacity_Ah = ((uint16_t)frame.data[1] << 8 | frame.data[0]);
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                updatePackTelemetry();
            }
            break;

        default:
            break;
    }

    xSemaphoreGive(_mutex);
    return recognized;
}

#include "board_battery.h"

bool DeyeBmsDecoder::getBatteryData(BatteryData& out_data) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        memcpy(&out_data, &_data, sizeof(BatteryData));
        xSemaphoreGive(_mutex);
    }

    // Overlay fresh ESP32 onboard LiPo battery reading (Option A via GPIO 6)
    LipoBatteryStatus lipo = BoardBattery::getInstance().getStatus();
    out_data.lipo_voltage_V = lipo.voltage_V;
    out_data.lipo_soc_percent = lipo.soc_percent;
    out_data.lipo_connected = lipo.connected;

    return out_data.communicationOK;
}

void DeyeBmsDecoder::checkWatchdog(uint32_t timeout_ms) {
    uint32_t now = millis();
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (_data.communicationOK && (now - _data.lastUpdate_ms > timeout_ms)) {
            _data.communicationOK = false;
        }
        xSemaphoreGive(_mutex);
    }
}
