// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : sd_logger.cpp
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include "sd_logger.h"
#include "board_config.h"
#include "ui.h"
#include <Wire.h>

static const char* TAG = "SD_LOGGER";

SdLogger& SdLogger::getInstance() {
    static SdLogger instance;
    return instance;
}

SdLogger::SdLogger()
    : _mounted(false),
      _logged_count(0),
      _last_flush_ms(0)
{
    memset(_current_filename, 0, sizeof(_current_filename));
}

SdLogger::~SdLogger() {
    if (_file) {
        _file.flush();
        _file.close();
    }
}

bool SdLogger::begin() {
    LOG_PRINTLN("[SD] Initializing SPI for MicroSD card...");

    // Initialize SPI bus pins for ESP32-S3-Touch-LCD-7
    SPI.begin(BOARD_SD_SCK_PIN, BOARD_SD_MISO_PIN, BOARD_SD_MOSI_PIN, -1);

    // Pull SD_CS LOW via CH422G
    UIManager::getInstance().setSdCs(true);
    delay(10);

    // Attempt mount
    if (!SD.begin(-1, SPI, 20000000)) { // 20 MHz SPI clock
        LOG_PRINTLN("[SD] No MicroSD card detected or mount failed. Logging disabled.");
        _mounted = false;
        UIManager::getInstance().setSdCs(false);
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        LOG_PRINTLN("[SD] No SD card attached. Logging disabled.");
        _mounted = false;
        UIManager::getInstance().setSdCs(false);
        return false;
    }

    LOG_PRINT("[SD] Card detected: ");
    if (cardType == CARD_MMC) LOG_PRINTLN("MMC");
    else if (cardType == CARD_SD) LOG_PRINTLN("SDSC");
    else if (cardType == CARD_SDHC) LOG_PRINTLN("SDHC");
    else LOG_PRINTLN("UNKNOWN");

    uint64_t cardSizeMB = SD.cardSize() / (1024 * 1024);
    LOG_PRINTF("[SD] Card size: %llu MB\n", cardSizeMB);

    _mounted = openNextLogFile();
    return _mounted;
}

bool SdLogger::openNextLogFile() {
    // Look for next free log file: /can_log_001.csv, /can_log_002.csv, etc.
    for (int i = 1; i <= 999; i++) {
        snprintf(_current_filename, sizeof(_current_filename), "/can_log_%03d.csv", i);
        if (!SD.exists(_current_filename)) {
            break;
        }
    }

    LOG_PRINTF("[SD] Creating new log file: %s\n", _current_filename);
    _file = SD.open(_current_filename, FILE_WRITE);
    if (!_file) {
        LOG_PRINTF("[SD ERROR] Failed to open %s for writing!\n", _current_filename);
        return false;
    }

    // Write CSV header exactly as required by specification:
    // timestamp_ms,can_id,extended,dlc,data0,data1,data2,data3,data4,data5,data6,data7
    _file.println("timestamp_ms,can_id,extended,dlc,data0,data1,data2,data3,data4,data5,data6,data7");
    _file.flush();
    _last_flush_ms = millis();
    _logged_count = 0;
    LOG_PRINTLN("[SD] Header written successfully. CSV Logging active.");
    return true;
}

void SdLogger::logFrame(const CanFrameRaw& frame) {
    if (!_mounted || !_file) return;

    // Buffer line in local string: timestamp_ms,can_id,extended,dlc,data0..data7
    char line[128];
    int len = snprintf(
        line, sizeof(line),
        "%lu,0x%03X,%d,%u,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
        (unsigned long)frame.timestamp_ms,
        (unsigned int)frame.id,
        frame.extended ? 1 : 0,
        frame.dlc,
        (frame.dlc > 0) ? frame.data[0] : 0,
        (frame.dlc > 1) ? frame.data[1] : 0,
        (frame.dlc > 2) ? frame.data[2] : 0,
        (frame.dlc > 3) ? frame.data[3] : 0,
        (frame.dlc > 4) ? frame.data[4] : 0,
        (frame.dlc > 5) ? frame.data[5] : 0,
        (frame.dlc > 6) ? frame.data[6] : 0,
        (frame.dlc > 7) ? frame.data[7] : 0
    );

    if (len > 0) {
        _file.write((const uint8_t*)line, len);
        _logged_count++;
    }

    // Flush to disk every 1000ms or every 50 frames
    uint32_t now = millis();
    if (_logged_count % 50 == 0 || (now - _last_flush_ms) >= 1000) {
        flush();
    }
}

void SdLogger::flush() {
    if (_mounted && _file) {
        _file.flush();
        _last_flush_ms = millis();
    }
}
