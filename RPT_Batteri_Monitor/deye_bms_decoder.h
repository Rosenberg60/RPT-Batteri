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

    // Reset / mark communication lost if no telegram received in timeout_ms
    void checkWatchdog(uint32_t timeout_ms = 5000);

private:
    DeyeBmsDecoder();
    ~DeyeBmsDecoder();

    BatteryData _data;
    portMUX_TYPE _spinlock;
};
