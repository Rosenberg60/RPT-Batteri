#pragma once

#include <Arduino.h>
#include "board_config.h"

/**
 * ============================================================================
 * BoardBattery: ESP32-S3 Onboard 3.7V LiPo Battery Monitor (Option A)
 *
 * Hardware connection (Option A):
 * Jumper wire connecting test point TP1 to J8 Pin 2 (AD / GPIO 6).
 * Voltage divider on PCB: R18 (200k) + R19 (100k).
 * Hardware multiplier: (200k + 100k) / 100k = 3.00.
 * ============================================================================
 */

struct LipoBatteryStatus {
    float   voltage_V;       // Filtered battery voltage (e.g. 3.92V)
    uint8_t soc_percent;     // Calculated state of charge 0-100%
    bool    connected;       // True if voltage >= BOARD_LIPO_MIN_CONNECTED_V (2.0V)
};

class BoardBattery {
public:
    static BoardBattery& getInstance();

    // Initialize ADC pin and settings
    void begin();

    // Read ADC, filter, and calculate percentage (rate-limited to 2 Hz)
    void update();

    // Get thread-safe snapshot of battery status
    LipoBatteryStatus getStatus();

    // Format display string: "Lipo Bat: 3.9V (68%)" or "Lipo Bat: N/A"
    void getFormattedString(char* buf, size_t bufSize);

    // Li-ion / LiPo 1S discharge curve interpolation (3.30V to 4.20V)
    static uint8_t voltageToPercent(float v);

private:
    BoardBattery();
    ~BoardBattery();

    float _filtered_voltage;
    LipoBatteryStatus _status;
    uint32_t _last_read_ms;
    portMUX_TYPE _mux;
};
