// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : deye_bms_decoder.cpp
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

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
        _data.pack1_online = false;
        _data.pack2_online = false;
        _data.pack1_cells_valid = false;
        _data.pack2_cells_valid = false;
        _data.individualCellsKnown = false;
        _data.lastUpdate_ms = 0;
        _data.pack1_capacity_Ah = 200.0f;
        _data.pack2_capacity_Ah = 0.0f;
        xSemaphoreGive(_mutex);
    }
}

#include "system_config.h"

void DeyeBmsDecoder::updatePackTelemetry() {
    // Zero-tolerance for estimated/fake data:
    // CAN communication is directly with Pack 1 (Configured Master).
    // All measured values from CAN bus belong to Pack 1 / Master bank.
    BatteryBrand mBrand = SystemConfig::getInstance().getMaster();
    BatteryBrand sBrand = SystemConfig::getInstance().getSlave();
    float mCap = SystemConfig::getInstance().getBrandNominalCap(mBrand);
    float sCap = SystemConfig::getInstance().getBrandNominalCap(sBrand);

    _data.pack1_online = _data.communicationOK;
    _data.pack1_capacity_Ah = (_data.totalCapacity_Ah > 0) ? (float)_data.totalCapacity_Ah : mCap;
    _data.pack1_soc_percent = _data.soc_percent;
    _data.pack1_current_A = _data.current_A;
    _data.pack1_power_W = _data.power_W;
    _data.pack1_chargeLimit_A = _data.chargeCurrentLimit_A;
    _data.pack1_dischargeLimit_A = _data.dischargeCurrentLimit_A;
    _data.pack1_minV = _data.minCellVoltage_V;
    _data.pack1_maxV = _data.maxCellVoltage_V;

    float socFrac1 = (_data.pack1_soc_percent > 0) ? (_data.pack1_soc_percent / 100.0f) : 0.0f;
    _data.pack1_energy_kwh = (_data.pack1_capacity_Ah * 51.2f * socFrac1) / 1000.0f;

    // Pack 2 (Configured Slave):
    // No physical data stream received directly on CAN bus yet.
    // Explicitly set to OFFLINE / UNCONNECTED until dedicated telemetry is received.
    _data.pack2_online = false;
    _data.pack2_cells_valid = false;
    _data.pack2_soc_percent = 0;
    _data.pack2_capacity_Ah = sCap;
    _data.pack2_current_A = 0.0f;
    _data.pack2_power_W = 0.0f;
    _data.pack2_energy_kwh = 0.0f;
    _data.pack2_chargeLimit_A = 0.0f;
    _data.pack2_dischargeLimit_A = 0.0f;
    _data.pack2_minV = 0.0f;
    _data.pack2_maxV = 0.0f;
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
                _data.pack1_minV       = _data.minCellVoltage_V;
                _data.pack1_maxV       = _data.maxCellVoltage_V;
                if (frame.dlc >= 8) {
                    uint16_t tMinK = ((uint16_t)frame.data[5] << 8 | frame.data[4]);
                    uint16_t tMaxK = ((uint16_t)frame.data[7] << 8 | frame.data[6]);
                    if (tMinK > 200 && tMinK < 400) _data.minCellTemp_C = tMinK - 273.15f;
                    if (tMaxK > 200 && tMaxK < 400) _data.maxCellTemp_C = tMaxK - 273.15f;
                }

                // Zero fake data: No synthetic offsets generated!
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                _data.pack1_online = true;
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
                        float vCell = rawMv * 0.001f;
                        _data.cellVoltages[baseCell + i] = vCell;
                        _data.pack1_cellVoltages[baseCell + i] = vCell;
                        _data.individualCellsKnown = true;
                        _data.pack1_cells_valid = true;
                    }
                }
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
                _data.pack1_online = true;
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

void DeyeBmsDecoder::setBatteryData(const BatteryData& in_data) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        memcpy(&_data, &in_data, sizeof(BatteryData));
        _data.lastUpdate_ms = millis();
        xSemaphoreGive(_mutex);
    }
}

void DeyeBmsDecoder::checkWatchdog(uint32_t timeout_ms) {
    uint32_t now = millis();
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (_data.communicationOK && (now - _data.lastUpdate_ms > timeout_ms)) {
            _data.communicationOK = false;
            _data.pack1_online = false;
        }
        xSemaphoreGive(_mutex);
    }
}

