// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : deye_bms_decoder.h
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#pragma once

#include <Arduino.h>
#include "battery_data.h"

/**
 * ============================================================================
 * Deye / RPT BMS Protocol Decoder (Modular Architecture for Phase 2)
 *
 * In Phase 1 (Scanner), this class logs candidate BMS CAN-IDs.
 * In Phase 2, once the real CAN log is analyzed from the installation,
 * exact scaling factors and bitfields will be populated here.
 * ============================================================================
 */
class DeyeBmsDecoder {
public:
    static DeyeBmsDecoder& getInstance();

    // Initialize decoder
    void begin();

    // Process a raw incoming frame and update BatteryData model
    bool decodeFrame(const CanFrameRaw& frame);

    // Get thread-safe snapshot of decoded battery data
    bool getBatteryData(BatteryData& out_data);

    // Update battery data directly (used by RS485 poller)
    void setBatteryData(const BatteryData& in_data);

    // Reset / mark communication lost if no telegram received in timeout_ms
    void checkWatchdog(uint32_t timeout_ms = 5000);

private:
    DeyeBmsDecoder();
    ~DeyeBmsDecoder();

    void updatePackTelemetry();

    BatteryData _data;
    bool _pack2_soc_direct;
    SemaphoreHandle_t _mutex;
};
