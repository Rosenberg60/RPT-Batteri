#pragma once

// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : deye_can_gateway.h (BMS CAN Gateway to Deye Inverter)
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include <Arduino.h>
#include "battery_data.h"

class DeyeCanGateway {
public:
    static DeyeCanGateway& getInstance();

    void begin();
    void update();

    void setEnabled(bool en) { _enabled = en; }
    bool isEnabled() const { return _enabled; }
    uint32_t getTxCount() const { return _tx_count; }

private:
    DeyeCanGateway();
    ~DeyeCanGateway();

    void sendTelemetryHeartbeat();

    bool     _enabled;
    uint32_t _last_heartbeat_ms;
    uint32_t _tx_count;
};
