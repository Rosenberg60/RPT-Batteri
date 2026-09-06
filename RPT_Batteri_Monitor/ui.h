#pragma once

// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : ui.h (UI & Graphics Engine Header)
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include <Arduino.h>
#include "board_config.h"
#include "battery_data.h"
#include "esp_lcd_panel_ops.h"


enum UIViewMode {
    UI_VIEW_DASHBOARD = 0,        // Page 1: Main Battery Storage Dashboard
    UI_VIEW_CELL_DIAGNOSTICS = 1, // Page 2: Cell Balance & Voltage Diagnostics
    UI_VIEW_CAN_SCANNER = 2,      // Page 3: Raw CAN Bus Scanner
    UI_VIEW_CONFIG = 3            // Page 4: System Configuration (Master/Slave Selection)
};

class UIManager {
public:
    static UIManager& getInstance();

    // Initialize I2C, CH422G (CAN_SEL=1, Backlight=1) and 7" RGB Display
    bool begin();

    // Update screen contents with fresh statistics and live packet feed
    void updateDisplay();

    // Set LCD backlight state
    void setBacklight(bool on);

    // Set SD Card Chip Select state
    void setSdCs(bool active);

    // Set CAN mode (CH422G EXIO5: HIGH = CAN, LOW = USB)
    void setCanMode(bool enable);

    // View Mode Management (Dashboard vs Cell Diagnostics vs Raw CAN Scanner vs Config)
    void setViewMode(UIViewMode mode) { _view_mode = mode; }
    UIViewMode getViewMode() const { return _view_mode; }
    void setPage(uint8_t page) {
        if (page <= 3) _view_mode = (UIViewMode)page;
    }
    void toggleViewMode() {
        _view_mode = (UIViewMode)((_view_mode + 1) % 4);
    }
    uint8_t getCellViewPack() const { return _cell_view_pack; }
    void setCellViewPack(uint8_t pack) { _cell_view_pack = pack; }

    // Get frame buffer pointer (if needed)
    uint16_t* getFrameBuffer() const { return _framebuffer; }

private:
    UIManager();
    ~UIManager();

    // Low-level CH422G I2C write
    bool writeCh422gReg(uint8_t reg_addr, uint8_t value);
    void updateCh422gOutput();

    // Touch detection (with coordinate mapping)
    void checkTouch();

    // Render Routines (Static layout drawn once, dynamic values updated in-place)
    void drawStaticDashboard();
    void updateDynamicDashboard(const BatteryData& bData, const ScannerOverview& overview);

    void drawStaticCellDiagnostics();
    void updateDynamicCellDiagnostics(const BatteryData& bData, const ScannerOverview& overview);

    void drawStaticScanner();
    void updateDynamicScanner(const ScannerOverview& overview, const BatteryData& bData);

    void drawStaticConfig();
    void updateDynamicConfig(const BatteryData& bData);

    void drawBottomNav(uint8_t activePage);

    // Drawing primitives for RGB565 framebuffer
    void fillScreen(uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void drawFastHLine(int x, int y, int w, uint16_t color);
    void drawFastVLine(int x, int y, int h, uint16_t color);
    void drawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawGlyph(int x, int y, const uint8_t* glyph, uint16_t color, uint16_t bg, uint8_t size);
    void drawString(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t size = 1);
    void drawTextRow(int x, int y, int maxW, const char* text, uint16_t color, uint16_t bg, uint8_t size = 1);

    // FreeRTOS UI Task
    static void uiTaskTrampoline(void* arg);
    void uiTask();

    esp_lcd_panel_handle_t _panel_handle;
    uint16_t* _framebuffer;
    uint8_t _ch422g_out_mask;
    UIViewMode _view_mode;
    UIViewMode _last_drawn_mode;
    uint8_t _cell_view_pack;
    uint32_t _last_touch_ms;
    bool _initialized;
    bool _is_direct_fb;
};
