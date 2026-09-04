#pragma once

#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "battery_data.h"

class SdLogger {
public:
    static SdLogger& getInstance();

    // Initialize SPI and attempt mounting SD card via CH422G EXIO4
    bool begin();

    // Log a single CAN frame to the open CSV file
    void logFrame(const CanFrameRaw& frame);

    // Periodic flush to commit buffered writes to flash
    void flush();

    // Query status
    bool isMounted() const { return _mounted; }
    const char* getFileName() const { return _current_filename; }
    uint32_t getLoggedCount() const { return _logged_count; }

private:
    SdLogger();
    ~SdLogger();

    // Find next available filename (/can_log_001.csv ...)
    bool openNextLogFile();

    bool _mounted;
    File _file;
    char _current_filename[32];
    uint32_t _logged_count;
    uint32_t _last_flush_ms;
};
