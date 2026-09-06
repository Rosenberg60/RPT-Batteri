#pragma once

// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : rs485_battery_manager.h (Multidrop RS485 Battery Poller)
// DATO/TID: 2026-09-06 20:05:00
// =============================================================================

#include <Arduino.h>
#include "board_config.h"
#include "battery_data.h"

enum Rs485ProtocolType {
    RS485_PROTO_AUTO = 0,
    RS485_PROTO_PYLON_ASCII = 1,
    RS485_PROTO_MODBUS_RTU  = 2
};

class Rs485BatteryManager {
public:
    static Rs485BatteryManager& getInstance();

    // Initialize UART1 on GPIO 15 (TX) and GPIO 16 (RX)
    void begin(uint32_t baudrate = 9600);

    // Call regularly in main loop / FreeRTOS task
    void update();

    // Check communication health
    bool isPack1Online() const { return _pack1_online; }
    bool isPack2Online() const { return _pack2_online; }
    const char* getPack1Name() const { return _pack1_name; }
    const char* getPack2Name() const { return _pack2_name; }
    Rs485ProtocolType getActiveProtocol() const { return _active_protocol; }
    uint32_t getBaudrate() const { return _baudrate; }
    uint32_t getTxQueries() const { return _tx_queries; }
    uint32_t getRawRxBytes() const { return _raw_rx_bytes; }
    uint32_t getRxCount() const { return _rx_count; }
    uint32_t getErrCount() const { return _err_count; }
    const char* getLastTxString() const { return _last_tx_str; }
    const char* getLastRxHexString() const { return _last_rx_hex_str; }

    void setBaudrate(uint32_t baud);
    void cycleBaudrate();
    void setProtocol(Rs485ProtocolType proto);
    void triggerImmediatePoll();

private:
    Rs485BatteryManager();
    ~Rs485BatteryManager();

    void sendPylonAsciiQuery(uint8_t addr, uint8_t cid2);
    void sendModbusRtuQuery(uint8_t addr, uint16_t startReg, uint16_t regCount);

    void processIncomingBytes();
    bool parsePylonAsciiFrame(const char* frame, size_t len);
    bool parseModbusRtuFrame(const uint8_t* buffer, size_t len);

    uint16_t calcPylonChecksum(const char* buf, size_t len);
    uint16_t calcModbusCrc(const uint8_t* buf, size_t len);

    void aggregateTelemetry();

    bool _initialized;
    uint32_t _baudrate;
    Rs485ProtocolType _active_protocol;

    // Polling state machine
    uint8_t  _poll_state;       // 0=Query P1, 1=Wait P1, 2=Query P2, 3=Wait P2
    uint32_t _last_poll_ms;
    uint32_t _last_p1_rx_ms;
    uint32_t _last_p2_rx_ms;

    // RX Ring buffer
    uint8_t  _rx_buffer[512];
    size_t   _rx_len;
    uint32_t _rx_last_byte_ms;

    // Status
    bool     _pack1_online;
    bool     _pack2_online;
    char     _pack1_name[16];
    char     _pack2_name[16];
    uint32_t _tx_queries;
    uint32_t _raw_rx_bytes;
    uint32_t _rx_count;
    uint32_t _err_count;
    uint8_t  _detect_try_count;
    char     _last_tx_str[40];
    char     _last_rx_hex_str[48];
};
