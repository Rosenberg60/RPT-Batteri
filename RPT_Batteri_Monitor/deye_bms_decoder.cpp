#include "deye_bms_decoder.h"

DeyeBmsDecoder& DeyeBmsDecoder::getInstance() {
    static DeyeBmsDecoder instance;
    return instance;
}

DeyeBmsDecoder::DeyeBmsDecoder() {
    _spinlock = portMUX_INITIALIZER_UNLOCKED;
    memset(&_data, 0, sizeof(_data));
}

DeyeBmsDecoder::~DeyeBmsDecoder() {
}

void DeyeBmsDecoder::begin() {
    portENTER_CRITICAL(&_spinlock);
    memset(&_data, 0, sizeof(_data));
    _data.communicationOK = false;
    _data.lastUpdate_ms = 0;
    portEXIT_CRITICAL(&_spinlock);
}

bool DeyeBmsDecoder::decodeFrame(const CanFrameRaw& frame) {
    bool recognized = false;

    portENTER_CRITICAL(&_spinlock);

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
            }
            break;

        case 0x355:
            // State of Charge (SOC) and State of Health (SOH)
            if (frame.dlc >= 4) {
                _data.soc_percent = ((uint16_t)frame.data[1] << 8 | frame.data[0]);
                _data.soh_percent = ((uint16_t)frame.data[3] << 8 | frame.data[2]);
                recognized = true;
                _data.lastUpdate_ms = frame.timestamp_ms;
                _data.communicationOK = true;
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
            }
            break;

        default:
            break;
    }

    portEXIT_CRITICAL(&_spinlock);
    return recognized;
}

bool DeyeBmsDecoder::getBatteryData(BatteryData& out_data) {
    portENTER_CRITICAL(&_spinlock);
    memcpy(&out_data, &_data, sizeof(BatteryData));
    portEXIT_CRITICAL(&_spinlock);
    return out_data.communicationOK;
}

void DeyeBmsDecoder::checkWatchdog(uint32_t timeout_ms) {
    uint32_t now = millis();
    portENTER_CRITICAL(&_spinlock);
    if (_data.communicationOK && (now - _data.lastUpdate_ms > timeout_ms)) {
        _data.communicationOK = false;
    }
    portEXIT_CRITICAL(&_spinlock);
}
