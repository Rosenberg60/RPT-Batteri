#pragma once

#include <Arduino.h>
#include "board_config.h"
#include "battery_data.h"
#include "esp_lcd_panel_ops.h"


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

    // Get frame buffer pointer (if needed)
    uint16_t* getFrameBuffer() const { return _framebuffer; }

private:
    UIManager();
    ~UIManager();

    // Low-level CH422G I2C write
    void writeCh422gReg(uint8_t reg_addr, uint8_t value);
    void updateCh422gOutput();

    // Drawing primitives for RGB565 framebuffer
    void fillScreen(uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void drawFastHLine(int x, int y, int w, uint16_t color);
    void drawFastVLine(int x, int y, int h, uint16_t color);
    void drawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawString(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t size = 1);

    // FreeRTOS UI Task
    static void uiTaskTrampoline(void* arg);
    void uiTask();

    esp_lcd_panel_handle_t _panel_handle;
    uint16_t* _framebuffer;
    uint8_t _ch422g_out_mask;
    bool _initialized;
};
