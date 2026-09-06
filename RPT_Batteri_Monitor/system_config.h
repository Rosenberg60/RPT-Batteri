// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : system_config.h
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#pragma once

#include <Arduino.h>
#include <Preferences.h>

enum BatteryBrand {
    BATTERY_BRAND_ROSEN = 0,
    BATTERY_BRAND_RPT   = 1,
    BATTERY_BRAND_OTHER = 2
};

class SystemConfig {
public:
    static SystemConfig& getInstance() {
        static SystemConfig instance;
        return instance;
    }

    void begin() {
        Preferences prefs;
        prefs.begin("rpt_sys", false);
        uint8_t m = prefs.getUChar("master", (uint8_t)BATTERY_BRAND_RPT); // Default RPT as Master
        uint8_t s = prefs.getUChar("slave",  (uint8_t)BATTERY_BRAND_ROSEN); // Default Rosen as Slave
        prefs.end();

        _master = (m <= 2) ? (BatteryBrand)m : BATTERY_BRAND_RPT;
        _slave  = (s <= 2) ? (BatteryBrand)s : BATTERY_BRAND_ROSEN;
    }

    BatteryBrand getMaster() const { return _master; }
    BatteryBrand getSlave() const { return _slave; }

    void setMaster(BatteryBrand b) {
        if (_master != b) {
            _master = b;
            save();
        }
    }

    void setSlave(BatteryBrand b) {
        if (_slave != b) {
            _slave = b;
            save();
        }
    }

    const char* getBrandName(BatteryBrand b) const {
        switch (b) {
            case BATTERY_BRAND_ROSEN: return "ROSEN";
            case BATTERY_BRAND_RPT:   return "RPT";
            case BATTERY_BRAND_OTHER: return "Andet";
            default:                  return "Ukendt";
        }
    }

    const char* getMasterBrandName() const { return getBrandName(_master); }
    const char* getSlaveBrandName() const { return getBrandName(_slave); }

    float getBrandNominalCap(BatteryBrand b) const {
        switch (b) {
            case BATTERY_BRAND_ROSEN: return 200.0f;
            case BATTERY_BRAND_RPT:   return 300.0f;
            case BATTERY_BRAND_OTHER: return 250.0f;
            default:                  return 200.0f;
        }
    }

private:
    SystemConfig()
        : _master(BATTERY_BRAND_RPT),
          _slave(BATTERY_BRAND_ROSEN)
    {
    }

    void save() {
        Preferences prefs;
        prefs.begin("rpt_sys", false);
        prefs.putUChar("master", (uint8_t)_master);
        prefs.putUChar("slave",  (uint8_t)_slave);
        prefs.end();
    }

    BatteryBrand _master;
    BatteryBrand _slave;
};
