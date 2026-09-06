// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : deye_can_gateway.cpp (BMS CAN Gateway to Deye Inverter)
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include "deye_can_gateway.h"
#include "can_receiver.h"
#include "deye_bms_decoder.h"
#include "system_config.h"

DeyeCanGateway& DeyeCanGateway::getInstance() {
    static DeyeCanGateway instance;
    return instance;
}

DeyeCanGateway::DeyeCanGateway()
    : _enabled(true),
      _last_heartbeat_ms(0),
      _tx_count(0) {
}

DeyeCanGateway::~DeyeCanGateway() {
}

void DeyeCanGateway::begin() {
    _enabled = true;
    _last_heartbeat_ms = millis();
    LOG_PRINTLN("[GATEWAY] Deye CAN BMS Gateway initialized (Pylontech 500k standard).");
}

void DeyeCanGateway::update() {
    if (!_enabled) return;

    uint32_t now = millis();
    // Transmit Pylontech heartbeat every 1000ms
    if (now - _last_heartbeat_ms >= 1000) {
        _last_heartbeat_ms = now;
        sendTelemetryHeartbeat();
    }
}

void DeyeCanGateway::sendTelemetryHeartbeat() {
    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    // Only broadcast to Deye if we have active telemetry from at least one battery
    if (!bData.rs485_online && !bData.communicationOK) {
        return;
    }

    uint8_t d[8] = { 0 };

    // --- Frame 0x351: Voltage & Current Limits ---
    // Target charge voltage: 56.8V (0.1V resolution = 568)
    uint16_t vChgLim = 568;

    // Dynamic charge current throttling based on highest cell voltage
    // Max sum: RPT 150A + Rosen 100A = 250A
    uint16_t iChgLim = 2500; // 250.0A in 0.1A units
    if (bData.maxCellVoltage_V >= 3.65f) {
        iChgLim = 0; // Cut off charge immediately if any cell hits 3.65V
    } else if (bData.maxCellVoltage_V >= 3.50f) {
        iChgLim = 200; // Throttle to 20.0A in absorption top
    } else if (bData.maxCellVoltage_V >= 3.42f) {
        iChgLim = 800; // Throttle to 80.0A
    }

    uint16_t iDchgLim = 2500; // 250.0A discharge limit
    if (bData.minCellVoltage_V <= 2.80f) {
        iDchgLim = 0; // Cut off discharge if any cell drops below 2.80V
    }

    uint16_t vCutoff = 448; // 44.8V cutoff (2.80V * 16)

    d[0] = vChgLim & 0xFF;
    d[1] = (vChgLim >> 8) & 0xFF;
    d[2] = iChgLim & 0xFF;
    d[3] = (iChgLim >> 8) & 0xFF;
    d[4] = iDchgLim & 0xFF;
    d[5] = (iDchgLim >> 8) & 0xFF;
    d[6] = vCutoff & 0xFF;
    d[7] = (vCutoff >> 8) & 0xFF;
    CanReceiver::getInstance().transmitFrame(0x351, d, 8);
    _tx_count++;

    // --- Frame 0x355: SOC & SOH ---
    uint16_t soc = bData.soc_percent;
    uint16_t soh = bData.soh_percent > 0 ? bData.soh_percent : 100;
    d[0] = soc & 0xFF;
    d[1] = (soc >> 8) & 0xFF;
    d[2] = soh & 0xFF;
    d[3] = (soh >> 8) & 0xFF;
    d[4] = 0;
    d[5] = 0;
    d[6] = 0;
    d[7] = 0;
    CanReceiver::getInstance().transmitFrame(0x355, d, 8);
    _tx_count++;

    // --- Frame 0x356: Voltage, Current & Temperature ---
    // Voltage in 0.01V units
    float vActual = (bData.voltage_V > 40.0f) ? bData.voltage_V : 53.40f;
    uint16_t vUnits = (uint16_t)(vActual * 100.0f + 0.5f);

    // Current in 0.1A signed units
    int16_t iUnits = (int16_t)(bData.current_A * 10.0f + 0.5f);

    // Temperature in 0.1°C signed units
    float tMax = (bData.maxCellTemp_C > 0.0f) ? bData.maxCellTemp_C : 24.5f;
    int16_t tUnits = (int16_t)(tMax * 10.0f + 0.5f);

    d[0] = vUnits & 0xFF;
    d[1] = (vUnits >> 8) & 0xFF;
    d[2] = iUnits & 0xFF;
    d[3] = (iUnits >> 8) & 0xFF;
    d[4] = tUnits & 0xFF;
    d[5] = (tUnits >> 8) & 0xFF;
    d[6] = 0;
    d[7] = 0;
    CanReceiver::getInstance().transmitFrame(0x356, d, 8);
    _tx_count++;

    // --- Frame 0x359: Protection & Alarm Flags ---
    memset(d, 0, 8);
    if (bData.maxCellVoltage_V >= 3.65f) d[0] |= 0x04; // High cell voltage protection
    if (bData.minCellVoltage_V <= 2.80f) d[0] |= 0x10; // Under voltage protection
    if (bData.maxCellTemp_C >= 55.0f)    d[0] |= 0x01; // High temp protection
    d[4] = (bData.pack1_online && bData.pack2_online) ? 2 : 1; // 2 battery packs
    CanReceiver::getInstance().transmitFrame(0x359, d, 8);
    _tx_count++;

    // --- Frame 0x35C: Charge & Discharge Enable Requests ---
    memset(d, 0, 8);
    uint8_t req = 0xC0; // Bit 7: Charge allowed, Bit 6: Discharge allowed
    if (iChgLim == 0)  req &= ~0x80; // Disable charge
    if (iDchgLim == 0) req &= ~0x40; // Disable discharge
    d[0] = req;
    CanReceiver::getInstance().transmitFrame(0x35C, d, 8);
    _tx_count++;

    // --- Frame 0x35E: Manufacturer String ("PYLON   ") ---
    memcpy(d, "PYLON   ", 8);
    CanReceiver::getInstance().transmitFrame(0x35E, d, 8);
    _tx_count++;

    bData.deye_gateway_active = true;
    bData.deye_tx_count = _tx_count;
    DeyeBmsDecoder::getInstance().setBatteryData(bData);
}
