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
    // In Phase 1 (Scanner), we inspect IDs and prepare hooks for Phase 2.
    // The exact scaling will be determined after receiving the installation CAN log.
    bool recognized = false;

    portENTER_CRITICAL(&_spinlock);

    switch (frame.id) {
        case 0x351:
            // Typical Pylontech / Deye: Voltage & Current Limits
            // e.g. bytes 0-1: Charge Voltage Limit (0.1V)
            //      bytes 2-3: Charge Current Limit (0.1A)
            //      bytes 4-5: Discharge Current Limit (0.1A)
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
            break;

        case 0x355:
            // Typical Pylontech / Deye: SOC & SOH
            // e.g. bytes 0-1: SOC (1%)
            //      bytes 2-3: SOH (1%)
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
            break;

        case 0x356:
            // Typical Pylontech / Deye: Voltage, Current, Temp
            // e.g. bytes 0-1: Voltage (0.01V)
            //      bytes 2-3: Current (0.1A, signed)
            //      bytes 4-5: Temperature (0.1°C, signed)
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
            break;

        case 0x359:
            // Protection & Warning flags
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
            break;

        case 0x35C:
            // Charge / Discharge enable requests
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
            break;

        case 0x35E:
            // Manufacturer identifier string
            recognized = true;
            _data.lastUpdate_ms = frame.timestamp_ms;
            _data.communicationOK = true;
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
