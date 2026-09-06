// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : board_battery.cpp
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include "board_battery.h"

BoardBattery& BoardBattery::getInstance() {
    static BoardBattery instance;
    return instance;
}

BoardBattery::BoardBattery()
    : _filtered_voltage(0.0f),
      _last_read_ms(0),
      _mux(portMUX_INITIALIZER_UNLOCKED)
{
    _status.voltage_V = 0.0f;
    _status.soc_percent = 0;
    _status.connected = false;
}

BoardBattery::~BoardBattery() {
}

void BoardBattery::begin() {
    pinMode(BOARD_LIPO_ADC_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    // Initial reading to populate status immediately on boot
    update();

    LOG_PRINTF("[LIPO] ADC initialized on GPIO %d (Divider ratio: %.2fx)\n",
               (int)BOARD_LIPO_ADC_PIN, BOARD_LIPO_DIVIDER_RATIO);
    if (_status.connected) {
        LOG_PRINTF("[LIPO] Detected battery: %.2fV (%u%% SOC)\n",
                   _status.voltage_V, _status.soc_percent);
    } else {
        LOG_PRINTLN("[LIPO] No battery detected (V < 2.0V or jumper wire TP1->AD not installed).");
    }
}

void BoardBattery::update() {
    uint32_t now = millis();
    // Rate-limit reading to twice per second (500ms) to conserve CPU and avoid jitter
    if (_last_read_ms != 0 && (now - _last_read_ms < 500)) {
        return;
    }
    _last_read_ms = now;

    // Sample ADC 16 times to average out high-frequency electrical and RF noise
    uint32_t sumMv = 0;
    for (int i = 0; i < 16; i++) {
        sumMv += analogReadMilliVolts(BOARD_LIPO_ADC_PIN);
    }
    float pinVolts = (sumMv / 16.0f) / 1000.0f;
    float rawVoltage = pinVolts * BOARD_LIPO_DIVIDER_RATIO;

    // Check if connected (battery present and jumper wire TP1 -> AD installed)
    bool isConnected = (rawVoltage >= BOARD_LIPO_MIN_CONNECTED_V);

    if (isConnected) {
        // Exponential moving average filter (alpha = 0.20) for rock-solid zero-jitter display
        if (_filtered_voltage < 1.0f) {
            _filtered_voltage = rawVoltage;
        } else {
            _filtered_voltage = (_filtered_voltage * 0.80f) + (rawVoltage * 0.20f);
        }
    } else {
        _filtered_voltage = 0.0f;
    }

    uint8_t pct = isConnected ? voltageToPercent(_filtered_voltage) : 0;

    portENTER_CRITICAL(&_mux);
    _status.voltage_V = isConnected ? _filtered_voltage : 0.0f;
    _status.soc_percent = pct;
    _status.connected = isConnected;
    portEXIT_CRITICAL(&_mux);
}

LipoBatteryStatus BoardBattery::getStatus() {
    portENTER_CRITICAL(&_mux);
    LipoBatteryStatus st = _status;
    portEXIT_CRITICAL(&_mux);
    return st;
}

void BoardBattery::getFormattedString(char* buf, size_t bufSize) {
    if (!buf || bufSize == 0) return;
    LipoBatteryStatus st = getStatus();
    if (st.connected) {
        snprintf(buf, bufSize, "Lipo Bat: %.1fV (%u%%)", st.voltage_V, st.soc_percent);
    } else {
        snprintf(buf, bufSize, "Lipo Bat: N/A");
    }
}

uint8_t BoardBattery::voltageToPercent(float v) {
    if (v >= 4.20f) return 100;
    if (v <= 3.30f) return 0;

    // Piecewise linear interpolation for standard 1S 3.7V Li-ion / LiPo discharge profile
    static const struct {
        float v;
        uint8_t pct;
    } curve[] = {
        { 4.20f, 100 },
        { 4.10f,  90 },
        { 4.00f,  80 },
        { 3.90f,  70 },
        { 3.82f,  55 },
        { 3.75f,  40 },
        { 3.70f,  30 },
        { 3.65f,  20 },
        { 3.55f,  10 },
        { 3.40f,   3 },
        { 3.30f,   0 }
    };
    constexpr size_t numPoints = sizeof(curve) / sizeof(curve[0]);

    for (size_t i = 0; i < numPoints - 1; i++) {
        if (v >= curve[i + 1].v) {
            float rangeV = curve[i].v - curve[i + 1].v;
            float frac = (v - curve[i + 1].v) / rangeV;
            uint8_t rangePct = curve[i].pct - curve[i + 1].pct;
            return curve[i + 1].pct + (uint8_t)(frac * rangePct + 0.5f);
        }
    }
    return 0;
}
