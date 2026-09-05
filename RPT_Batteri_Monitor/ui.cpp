#include "ui.h"
#include "can_receiver.h"
#include "sd_logger.h"
#include "deye_bms_decoder.h"
#include <Wire.h>
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_heap_caps.h"

// -----------------------------------------------------------------------------
// Standard 5x7 ASCII Font Table (characters 0x20 ' ' through 0x7E '~')
// -----------------------------------------------------------------------------
static const uint8_t font5x7[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, // (space)
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // backslash
    0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08  // ~
};

// -----------------------------------------------------------------------------
// Color Definitions (RGB565)
// -----------------------------------------------------------------------------
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_DARK_BLUE     0x0010
#define COLOR_NAVY          0x08C5
#define COLOR_BLUE          0x021F
#define COLOR_CYAN          0x07FF
#define COLOR_GREEN         0x07E0
#define COLOR_DARK_GREEN    0x03E0
#define COLOR_RED           0xF800
#define COLOR_ORANGE        0xFD20
#define COLOR_YELLOW        0xFFE0
#define COLOR_DARK_GRAY     0x18C3
#define COLOR_MID_GRAY      0x39E7
#define COLOR_LIGHT_GRAY    0x8410
#define COLOR_CARD_BG       0x10A2
#define COLOR_CARD_BORDER   0x2965
#define COLOR_CARD_HEADER   0x18E5

// C-hook to allow other modules (like SD logger) to toggle SD_CS on CH422G

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager()
    : _panel_handle(nullptr),
      _framebuffer(nullptr),
      _ch422g_out_mask(0),
      _view_mode(UI_VIEW_DASHBOARD),
      _last_touch_ms(0),
      _initialized(false)
{
}

UIManager::~UIManager() {
}

bool UIManager::writeCh422gReg(uint8_t reg_addr, uint8_t value) {
    Wire.beginTransmission(reg_addr);
    Wire.write(value);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        LOG_PRINTF("[UI WARNING] CH422G write to 0x%02X failed (code: %d)\n", reg_addr, err);
        return false;
    }
    return true;
}

void UIManager::updateCh422gOutput() {
    writeCh422gReg(CH422G_I2C_ADDR_WR_IO, _ch422g_out_mask);
}

void UIManager::setBacklight(bool on) {
    if (on) {
        _ch422g_out_mask |= CH422G_EXIO2_LCD_BL;
    } else {
        _ch422g_out_mask &= ~CH422G_EXIO2_LCD_BL;
    }
    updateCh422gOutput();
}

void UIManager::setSdCs(bool active) {
    // SD_CS is Active LOW
    if (active) {
        _ch422g_out_mask &= ~CH422G_EXIO4_SD_CS;
    } else {
        _ch422g_out_mask |= CH422G_EXIO4_SD_CS;
    }
    updateCh422gOutput();
}

void UIManager::setCanMode(bool enable) {
    if (enable) {
        _ch422g_out_mask |= CH422G_EXIO5_CAN_SEL;
        LOG_PRINTLN("[UI] CH422G EXIO5 set HIGH: CAN Mode active (TJA1051 transceiver connected).");
    } else {
        _ch422g_out_mask &= ~CH422G_EXIO5_CAN_SEL;
        LOG_PRINTLN("[UI] CH422G EXIO5 set LOW: USB Mode active (Native USB connected).");
    }
    updateCh422gOutput();
}

bool UIManager::begin() {
    LOG_PRINTLN("[UI] Initializing Wire on SDA=GPIO8, SCL=GPIO9 (100 kHz)...");

    // 1. Initialize Wire bus at 100 kHz with 50ms timeout
    Wire.begin(BOARD_I2C_SDA_PIN, BOARD_I2C_SCL_PIN, BOARD_I2C_FREQ_HZ);
    Wire.setTimeOut(50);
    delay(50);

    // 2. Configure CH422G:
    // Write WR_SET (0x24): 0x01 enables general output mode (IO_OE = 1)
    LOG_PRINTLN("[UI] Configuring CH422G WR_SET (0x24)...");
    writeCh422gReg(CH422G_I2C_ADDR_WR_SET, 0x01);
    delay(10);

    // Hardware reset sequence for Waveshare 7.0" LCD (ST7262):
    // EXIO1 (Touch RST) = 1 (active)
    // EXIO2 (LCD BL)    = 1 (Backlight ON)
    // EXIO3 (LCD RST)   = 0 (HOLD in reset!)
    // EXIO4 (SD CS)     = 1 (CS idle, active low)
    // Note: EXIO5 (CAN_SEL) kept LOW (0) so USB is NOT severed prematurely.
    LOG_PRINTLN("[UI] Holding ST7262 LCD in hardware reset (EXIO3=0)...");
    _ch422g_out_mask = CH422G_EXIO1_TP_RST |
                       CH422G_EXIO2_LCD_BL  |
                       CH422G_EXIO4_SD_CS;
    updateCh422gOutput();
    delay(20);

    // Release ST7262 LCD reset and allow power stabilization
    LOG_PRINTLN("[UI] Releasing ST7262 LCD reset (EXIO3=1)...");
    _ch422g_out_mask |= CH422G_EXIO3_LCD_RST;
    updateCh422gOutput();
    delay(120); // ST7262 required power-on stabilization delay

    LOG_PRINTLN("[UI] CH422G configured: Backlight=ON, LCD_RST=RELEASED.");

    // 3. Allocate RGB565 Framebuffer in PSRAM (800 * 480 * 2 = 768,000 bytes)
    LOG_PRINTLN("[UI] Allocating 800x480 RGB framebuffer in PSRAM...");
    _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                              MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_framebuffer) {
        LOG_PRINTLN("[UI] PSRAM allocation failed, attempting default memory allocation...");
        _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                                  MALLOC_CAP_DEFAULT);
    }

    if (!_framebuffer) {
        LOG_PRINTLN("[UI ERROR] Failed to allocate framebuffer memory!");
        return false;
    }
    LOG_PRINTF("[UI] Framebuffer ready at %p (%d KB)\n", _framebuffer, (LCD_WIDTH * LCD_HEIGHT * 2) / 1024);

    // Clear initial framebuffer
    fillScreen(COLOR_BLACK);

    // 4. Initialize 7.0-inch 800x480 RGB LCD Panel via ESP-IDF esp_lcd
    LOG_PRINTLN("[UI] Initializing 7.0-inch 800x480 RGB LCD Driver (ST7262)...");
    esp_lcd_rgb_panel_config_t panel_conf = {};
    panel_conf.clk_src = LCD_CLK_SRC_PLL160M;
    panel_conf.timings.pclk_hz = LCD_PIXEL_CLOCK_HZ;
    panel_conf.timings.h_res = LCD_WIDTH;
    panel_conf.timings.v_res = LCD_HEIGHT;
    panel_conf.timings.hsync_pulse_width = LCD_TIMING_HPW;
    panel_conf.timings.hsync_back_porch = LCD_TIMING_HBP;
    panel_conf.timings.hsync_front_porch = LCD_TIMING_HFP;
    panel_conf.timings.vsync_pulse_width = LCD_TIMING_VPW;
    panel_conf.timings.vsync_back_porch = LCD_TIMING_VBP;
    panel_conf.timings.vsync_front_porch = LCD_TIMING_VFP;
    panel_conf.timings.flags.pclk_active_neg = 1; // ST7262 clock latch polarity
    panel_conf.data_width = 16;
#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0))
    panel_conf.num_fbs = 1;
    panel_conf.bounce_buffer_size_px = LCD_WIDTH * 10; // 8000 px SRAM bounce buffer avoids PSRAM DMA starvation
#endif
    panel_conf.sram_trans_align = 4;
    panel_conf.psram_trans_align = 64;
    panel_conf.hsync_gpio_num = LCD_PIN_HSYNC;
    panel_conf.vsync_gpio_num = LCD_PIN_VSYNC;
    panel_conf.de_gpio_num = LCD_PIN_DE;
    panel_conf.pclk_gpio_num = LCD_PIN_PCLK;
    panel_conf.disp_gpio_num = -1;

    panel_conf.data_gpio_nums[0]  = LCD_PIN_DATA0;
    panel_conf.data_gpio_nums[1]  = LCD_PIN_DATA1;
    panel_conf.data_gpio_nums[2]  = LCD_PIN_DATA2;
    panel_conf.data_gpio_nums[3]  = LCD_PIN_DATA3;
    panel_conf.data_gpio_nums[4]  = LCD_PIN_DATA4;
    panel_conf.data_gpio_nums[5]  = LCD_PIN_DATA5;
    panel_conf.data_gpio_nums[6]  = LCD_PIN_DATA6;
    panel_conf.data_gpio_nums[7]  = LCD_PIN_DATA7;
    panel_conf.data_gpio_nums[8]  = LCD_PIN_DATA8;
    panel_conf.data_gpio_nums[9]  = LCD_PIN_DATA9;
    panel_conf.data_gpio_nums[10] = LCD_PIN_DATA10;
    panel_conf.data_gpio_nums[11] = LCD_PIN_DATA11;
    panel_conf.data_gpio_nums[12] = LCD_PIN_DATA12;
    panel_conf.data_gpio_nums[13] = LCD_PIN_DATA13;
    panel_conf.data_gpio_nums[14] = LCD_PIN_DATA14;
    panel_conf.data_gpio_nums[15] = LCD_PIN_DATA15;
    panel_conf.flags.fb_in_psram = 1;

    esp_err_t err = esp_lcd_new_rgb_panel(&panel_conf, &_panel_handle);
    if (err != ESP_OK) {
        LOG_PRINTF("[UI ERROR] esp_lcd_new_rgb_panel failed: 0x%X\n", err);
        return false;
    }

    esp_lcd_panel_reset(_panel_handle);
    esp_lcd_panel_init(_panel_handle);
    esp_lcd_panel_disp_on_off(_panel_handle, true);

    // Initial clear & render test screen
    fillScreen(COLOR_BLACK);
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);

    _initialized = true;

    // Spawn UI refresh task on Core 1
    xTaskCreatePinnedToCore(
        uiTaskTrampoline,
        "ui_task",
        4096,
        this,
        2,              // Normal priority
        NULL,
        1               // Pinned to Core 1
    );

    LOG_PRINTLN("[UI] Display task successfully started!");
    return true;
}

void UIManager::uiTaskTrampoline(void* arg) {
    reinterpret_cast<UIManager*>(arg)->uiTask();
}

void UIManager::uiTask() {
    LOG_PRINTLN("[UI] Task running on Core 1.");
    while (_initialized) {
        updateDisplay();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz refresh
    }
    vTaskDelete(NULL);
}

// -----------------------------------------------------------------------------
// Framebuffer Drawing Primitives
// -----------------------------------------------------------------------------
void UIManager::fillScreen(uint16_t color) {
    if (!_framebuffer) return;
    if (color == 0) {
        memset(_framebuffer, 0, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));
    } else {
        for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
            _framebuffer[i] = color;
        }
    }
}

void UIManager::fillRect(int x, int y, int w, int h, uint16_t color) {
    if (!_framebuffer) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int j = y; j < y + h; j++) {
        uint16_t* row = &_framebuffer[j * LCD_WIDTH + x];
        for (int i = 0; i < w; i++) {
            row[i] = color;
        }
    }
}

void UIManager::drawRect(int x, int y, int w, int h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void UIManager::drawFastHLine(int x, int y, int w, uint16_t color) {
    if (!_framebuffer || y < 0 || y >= LCD_HEIGHT) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (w <= 0) return;

    uint16_t* ptr = &_framebuffer[y * LCD_WIDTH + x];
    for (int i = 0; i < w; i++) {
        ptr[i] = color;
    }
}

void UIManager::drawFastVLine(int x, int y, int h, uint16_t color) {
    if (!_framebuffer || x < 0 || x >= LCD_WIDTH) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (h <= 0) return;

    uint16_t* ptr = &_framebuffer[y * LCD_WIDTH + x];
    for (int j = 0; j < h; j++) {
        *ptr = color;
        ptr += LCD_WIDTH;
    }
}

void UIManager::drawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (!_framebuffer) return;
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t* glyph = &font5x7[(c - 0x20) * 5];

    for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&glyph[col]);
        for (int row = 0; row < 7; row++) {
            uint16_t pixelColor = (line & 0x01) ? color : bg;
            if (pixelColor != bg || bg != color) {
                if (size == 1) {
                    int px = x + col;
                    int py = y + row;
                    if (px >= 0 && px < LCD_WIDTH && py >= 0 && py < LCD_HEIGHT) {
                        _framebuffer[py * LCD_WIDTH + px] = pixelColor;
                    }
                } else {
                    fillRect(x + col * size, y + row * size, size, size, pixelColor);
                }
            }
            line >>= 1;
        }
    }
    // Trailing column spacing
    if (bg != color) {
        if (size == 1) {
            drawFastVLine(x + 5, y, 7, bg);
        } else {
            fillRect(x + 5 * size, y, size, 7 * size, bg);
        }
    }
}

void UIManager::drawString(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t size) {
    while (*text) {
        drawChar(x, y, *text++, color, bg, size);
        x += 6 * size;
    }
}

// -----------------------------------------------------------------------------
// Touch Screen Handler (GT911 on I2C)
// -----------------------------------------------------------------------------
void UIManager::checkTouch() {
    uint32_t now = millis();
    if (now - _last_touch_ms < 350) return; // 350ms debounce

    // Query GT911 buffer status register (0x814E) and touch point 1 coordinates (0x8150..0x8153)
    Wire.beginTransmission((uint8_t)BOARD_TOUCH_I2C_ADDR);
    Wire.write(0x81);
    Wire.write(0x4E);
    if (Wire.endTransmission() == 0) {
        if (Wire.requestFrom((uint8_t)BOARD_TOUCH_I2C_ADDR, (size_t)6) == 6) {
            uint8_t status = Wire.read();
            if (status & 0x80) { // Buffer ready
                uint8_t points = status & 0x0F;
                Wire.read(); // track id (0x814F)
                uint8_t xLow = Wire.read();
                uint8_t xHigh = Wire.read();
                uint8_t yLow = Wire.read();
                uint8_t yHigh = Wire.read();
                uint16_t touchX = ((uint16_t)xHigh << 8) | xLow;
                uint16_t touchY = ((uint16_t)yHigh << 8) | yLow;

                if (points > 0) {
                    if (touchY >= 420) {
                        // Bottom tab bar clicked directly
                        if (touchX < 265) {
                            setPage(0); // Page 1: Dashboard
                        } else if (touchX < 530) {
                            setPage(1); // Page 2: Cell Diagnostics
                        } else {
                            setPage(2); // Page 3: CAN Scanner
                        }
                    } else {
                        // Tapped anywhere else on screen: cycle pages 1 -> 2 -> 3 -> 1
                        toggleViewMode();
                    }
                    _last_touch_ms = now;
                }

                // Clear buffer status flag
                Wire.beginTransmission((uint8_t)BOARD_TOUCH_I2C_ADDR);
                Wire.write(0x81);
                Wire.write(0x4E);
                Wire.write(0x00);
                Wire.endTransmission();
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Phase 2: Graphical Battery Storage Dashboard
// -----------------------------------------------------------------------------
void UIManager::drawDashboard(const BatteryData& bData, const ScannerOverview& overview) {
    fillScreen(COLOR_BLACK);

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "BATTERY STORAGE DASHBOARD", COLOR_WHITE, COLOR_NAVY, 2);
    drawString(345, 8, "Rosen (200Ah) + RPT (300Ah)", COLOR_CYAN, COLOR_NAVY, 1);

    uint32_t upSec = millis() / 1000;
    char upBuf[48];
    if (bData.communicationOK && bData.lastUpdate_ms > 0) {
        uint32_t ageMs = (millis() >= bData.lastUpdate_ms) ? (millis() - bData.lastUpdate_ms) : 0;
        snprintf(upBuf, sizeof(upBuf), "UP: %02lu:%02lu:%02lu  RX: %lums ago",
                 upSec / 3600, (upSec % 3600) / 60, upSec % 60, (unsigned long)ageMs);
    } else {
        snprintf(upBuf, sizeof(upBuf), "UP: %02lu:%02lu:%02lu  RX: Waiting...",
                 upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    }
    drawString(345, 24, upBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // Header Right Badge: BMS Online / Modules
    if (bData.communicationOK) {
        char packBadge[32];
        if (bData.moduleCount >= 2) {
            snprintf(packBadge, sizeof(packBadge), "BMS ONLINE: 501Ah (2 PACKS)");
        } else {
            snprintf(packBadge, sizeof(packBadge), "BMS ONLINE: (1 PACK)");
        }
        fillRect(550, 7, 235, 30, COLOR_DARK_GREEN);
        drawRect(550, 7, 235, 30, COLOR_GREEN);
        drawString(562, 16, packBadge, COLOR_WHITE, COLOR_DARK_GREEN, 1);
    } else {
        fillRect(570, 7, 215, 30, COLOR_RED);
        drawRect(570, 7, 215, 30, COLOR_WHITE);
        drawString(585, 16, "BMS: WAITING / OFFLINE", COLOR_WHITE, COLOR_RED, 1);
    }

    // 2. Four Top Metric Cards (Y: 48 to 170)
    const int cardY = 48;
    const int cardH = 122;
    const int cardW = 190;
    char valBuf[32];
    char subBuf[64];

    // --- CARD 0: STATE OF CHARGE (SOC) ---
    int c0X = 8;
    fillRect(c0X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c0X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c0X, cardY, cardW, 22, COLOR_CARD_HEADER);
    drawString(c0X + 8, cardY + 6, "STATE OF CHARGE", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    if (bData.communicationOK) {
        uint16_t socColor = (bData.soc_percent >= 40) ? COLOR_GREEN :
                            ((bData.soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
        snprintf(valBuf, sizeof(valBuf), "%u %%", bData.soc_percent);
        drawString(c0X + 18, cardY + 28, valBuf, socColor, COLOR_CARD_BG, 3);

        drawRect(c0X + 12, cardY + 60, 150, 14, COLOR_WHITE);
        fillRect(c0X + 162, cardY + 63, 4, 8, COLOR_WHITE);
        int socFillW = (146 * bData.soc_percent) / 100;
        if (socFillW > 146) socFillW = 146;
        if (socFillW > 0) fillRect(c0X + 14, cardY + 62, socFillW, 10, socColor);

        snprintf(subBuf, sizeof(subBuf), "Cap: %u Ah | SOH: %u%%",
                 bData.totalCapacity_Ah > 0 ? bData.totalCapacity_Ah : 501,
                 bData.soh_percent > 0 ? bData.soh_percent : 100);
        drawString(c0X + 10, cardY + 82, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c0X + 10, cardY + 100, "Pack Balance: Optimal", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawString(c0X + 18, cardY + 28, "-- %", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
        drawRect(c0X + 12, cardY + 60, 150, 14, COLOR_MID_GRAY);
        fillRect(c0X + 162, cardY + 63, 4, 8, COLOR_MID_GRAY);
        drawString(c0X + 10, cardY + 82, "Cap: 501 Ah (Offline)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c0X + 10, cardY + 100, "Status: Waiting CAN", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // --- CARD 1: STORAGE POWER ---
    int c1X = 206;
    fillRect(c1X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c1X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c1X, cardY, cardW, 22, COLOR_CARD_HEADER);
    drawString(c1X + 8, cardY + 6, "STORAGE POWER", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    if (bData.communicationOK) {
        float pKw = bData.power_W / 1000.0f;
        if (bData.power_W > 50.0f) {
            snprintf(valBuf, sizeof(valBuf), "+%.2f kW", pKw);
            drawString(c1X + 10, cardY + 28, valBuf, COLOR_GREEN, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, cardY + 60, 100, 16, COLOR_DARK_GREEN);
            drawRect(c1X + 12, cardY + 60, 100, 16, COLOR_GREEN);
            drawString(c1X + 20, cardY + 64, "CHARGING", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else if (bData.power_W < -50.0f) {
            snprintf(valBuf, sizeof(valBuf), "%.2f kW", pKw);
            drawString(c1X + 10, cardY + 28, valBuf, COLOR_ORANGE, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, cardY + 60, 115, 16, 0x8200);
            drawRect(c1X + 12, cardY + 60, 115, 16, COLOR_ORANGE);
            drawString(c1X + 18, cardY + 64, "DISCHARGING", COLOR_WHITE, 0x8200, 1);
        } else {
            drawString(c1X + 10, cardY + 28, "0.00 kW", COLOR_CYAN, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, cardY + 60, 90, 16, COLOR_DARK_GRAY);
            drawRect(c1X + 12, cardY + 60, 90, 16, COLOR_MID_GRAY);
            drawString(c1X + 18, cardY + 64, "STANDBY", COLOR_WHITE, COLOR_DARK_GRAY, 1);
        }
        snprintf(subBuf, sizeof(subBuf), "Max Chg : %.0f A (~21kW)", bData.chargeCurrentLimit_A);
        drawString(c1X + 10, cardY + 82, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "Max Dchg: %.0f A (~20kW)", bData.dischargeCurrentLimit_A);
        drawString(c1X + 10, cardY + 100, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    } else {
        drawString(c1X + 10, cardY + 28, "--- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
        fillRect(c1X + 12, cardY + 60, 90, 16, COLOR_DARK_GRAY);
        drawRect(c1X + 12, cardY + 60, 90, 16, COLOR_MID_GRAY);
        drawString(c1X + 18, cardY + 64, "STANDBY", COLOR_WHITE, COLOR_DARK_GRAY, 1);
        drawString(c1X + 10, cardY + 82, "Max Chg : --- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c1X + 10, cardY + 100, "Max Dchg: --- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // --- CARD 2: BANK VOLTAGE ---
    int c2X = 404;
    fillRect(c2X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c2X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c2X, cardY, cardW, 22, COLOR_CARD_HEADER);
    drawString(c2X + 8, cardY + 6, "BANK VOLTAGE", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    if (bData.communicationOK) {
        snprintf(valBuf, sizeof(valBuf), "%.2f V", bData.voltage_V);
        drawString(c2X + 12, cardY + 28, valBuf, COLOR_YELLOW, COLOR_CARD_BG, 3);
        float avgCell = (bData.voltage_V > 10.0f) ? (bData.voltage_V / 16.0f) : 0.0f;
        snprintf(subBuf, sizeof(subBuf), "Avg Cell : %.3f V", avgCell);
        drawString(c2X + 12, cardY + 64, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "Chg Limit: %.2f V", bData.chargeVoltageLimit_V);
        drawString(c2X + 10, cardY + 82, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "Cut-off  : %.2f V",
                 bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f);
        drawString(c2X + 10, cardY + 100, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    } else {
        drawString(c2X + 12, cardY + 28, "--.-- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
        drawString(c2X + 12, cardY + 64, "Avg Cell : -.--- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c2X + 10, cardY + 82, "Chg Limit: 57.60 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c2X + 10, cardY + 100, "Cut-off  : 44.80 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // --- CARD 3: BANK CURRENT ---
    int c3X = 602;
    int c3W = 190;
    fillRect(c3X, cardY, c3W, cardH, COLOR_CARD_BG);
    drawRect(c3X, cardY, c3W, cardH, COLOR_CARD_BORDER);
    fillRect(c3X, cardY, c3W, 22, COLOR_CARD_HEADER);
    drawString(c3X + 8, cardY + 6, "BANK CURRENT", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    if (bData.communicationOK) {
        uint16_t curColor = (bData.current_A > 0.5f) ? COLOR_GREEN :
                            ((bData.current_A < -0.5f) ? COLOR_ORANGE : COLOR_WHITE);
        snprintf(valBuf, sizeof(valBuf), "%+.1f A", bData.current_A);
        drawString(c3X + 12, cardY + 28, valBuf, curColor, COLOR_CARD_BG, 3);
        snprintf(subBuf, sizeof(subBuf), "Pack Temp: %.1f C", bData.temperature_C);
        drawString(c3X + 12, cardY + 64, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "Cur Limit: %.0f A", bData.chargeCurrentLimit_A);
        drawString(c3X + 10, cardY + 82, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c3X + 10, cardY + 100, "Modules  : 2 Synced", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawString(c3X + 12, cardY + 28, "---.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
        drawString(c3X + 12, cardY + 64, "Pack Temp: --.- C", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c3X + 10, cardY + 82, "Cur Limit: 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(c3X + 10, cardY + 100, "Modules  : 2 Expected", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // 3. Middle Section: Detail Panels (Y: 176 to 424)
    const int midY = 176;
    const int midH = 248;

    // --- LEFT PANEL: CELL HEALTH & BALANCING ---
    int p1X = 8;
    int p1W = 388;
    fillRect(p1X, midY, p1W, midH, COLOR_CARD_BG);
    drawRect(p1X, midY, p1W, midH, COLOR_CARD_BORDER);
    fillRect(p1X, midY, p1W, 24, COLOR_DARK_BLUE);
    drawString(p1X + 10, midY + 7, "CELL HEALTH & VOLTAGE BALANCE", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    if (bData.communicationOK) {
        // Two Sub-Boxes for Min and Max Cell
        fillRect(p1X + 12, midY + 30, 175, 48, COLOR_DARK_GRAY);
        drawRect(p1X + 12, midY + 30, 175, 48, COLOR_MID_GRAY);
        drawString(p1X + 20, midY + 34, "MIN CELL VOLTAGE", COLOR_CYAN, COLOR_DARK_GRAY, 1);
        snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.minCellVoltage_V);
        drawString(p1X + 20, midY + 48, subBuf, COLOR_CYAN, COLOR_DARK_GRAY, 2);

        fillRect(p1X + 197, midY + 30, 179, 48, COLOR_DARK_GRAY);
        drawRect(p1X + 197, midY + 30, 179, 48, COLOR_MID_GRAY);
        drawString(p1X + 205, midY + 34, "MAX CELL VOLTAGE", COLOR_YELLOW, COLOR_DARK_GRAY, 1);
        snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.maxCellVoltage_V);
        drawString(p1X + 205, midY + 48, subBuf, COLOR_YELLOW, COLOR_DARK_GRAY, 2);

        // Delta Hero Row
        drawString(p1X + 15, midY + 86, "CELL DELTA (dV):", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "%.0f mV", bData.cellDelta_mV);
        drawString(p1X + 130, midY + 84, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 2);

        if (bData.cellDelta_mV < 20.0f) {
            fillRect(p1X + 215, midY + 82, 160, 20, COLOR_DARK_GREEN);
            drawRect(p1X + 215, midY + 82, 160, 20, COLOR_GREEN);
            drawString(p1X + 225, midY + 88, "[ PERFECT < 20mV ]", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else if (bData.cellDelta_mV < 50.0f) {
            fillRect(p1X + 215, midY + 82, 160, 20, COLOR_DARK_GRAY);
            drawRect(p1X + 215, midY + 82, 160, 20, COLOR_YELLOW);
            drawString(p1X + 225, midY + 88, "[ GOOD < 50mV ]", COLOR_YELLOW, COLOR_DARK_GRAY, 1);
        } else {
            fillRect(p1X + 215, midY + 82, 160, 20, 0x8200);
            drawRect(p1X + 215, midY + 82, 160, 20, COLOR_ORANGE);
            drawString(p1X + 220, midY + 88, "[ IMBALANCE > 50mV ]", COLOR_WHITE, 0x8200, 1);
        }

        // Graphical Spread Gauge (3.00V to 3.65V)
        int gaugeX = p1X + 15;
        int gaugeY = midY + 116;
        int gaugeW = 358;
        int gaugeH = 12;
        fillRect(gaugeX, gaugeY, gaugeW, gaugeH, COLOR_DARK_GRAY);
        drawRect(gaugeX, gaugeY, gaugeW, gaugeH, COLOR_MID_GRAY);

        float minV = bData.minCellVoltage_V > 2.8f ? bData.minCellVoltage_V : 3.0f;
        float maxV = bData.maxCellVoltage_V > 2.8f ? bData.maxCellVoltage_V : 3.0f;
        int minPx = gaugeX + (int)(((minV - 3.0f) / 0.65f) * gaugeW);
        int maxPx = gaugeX + (int)(((maxV - 3.0f) / 0.65f) * gaugeW);
        if (minPx < gaugeX) minPx = gaugeX;
        if (maxPx > gaugeX + gaugeW - 4) maxPx = gaugeX + gaugeW - 4;
        if (maxPx < minPx + 4) maxPx = minPx + 4;
        fillRect(minPx, gaugeY + 2, maxPx - minPx, gaugeH - 4, COLOR_CYAN);
        fillRect(minPx - 1, gaugeY - 2, 3, gaugeH + 4, COLOR_YELLOW);
        fillRect(maxPx - 1, gaugeY - 2, 3, gaugeH + 4, COLOR_GREEN);

        drawString(gaugeX, gaugeY + 16, "3.00V Empty", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        drawString(gaugeX + 130, gaugeY + 16, "3.37V Rest", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(gaugeX + 285, gaugeY + 16, "3.65V Full", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

        // System Diagnostic Details
        snprintf(subBuf, sizeof(subBuf), "Cell Temperatures :  Min %.1f C  /  Max %.1f C",
                 bData.minCellTemp_C, bData.maxCellTemp_C);
        drawString(p1X + 15, midY + 148, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "BMS Ambient Temp  :  %.1f C   (Health SOH: %u%%)",
                 bData.temperature_C, bData.soh_percent);
        drawString(p1X + 15, midY + 168, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        drawString(p1X + 15, midY + 188, "Balancing Circuit :  Passive Standby (Delta < 20mV)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        float estKwh = (bData.totalCapacity_Ah * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
        snprintf(subBuf, sizeof(subBuf), "Stored Energy Est :  ~%.1f kWh of 25.6 kWh", estKwh);
        drawString(p1X + 15, midY + 210, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        // Clean Offline Guide
        drawString(p1X + 20, midY + 40, "WAITING FOR BMS TELEMETRY...", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 65, "* Check CAN cabling: Pin 4=CAN-H, Pin 5=CAN-L", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 85, "* Inverter baud rate: 500 kbit/s (standard)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 105, "* Jumper 13: REMOVED / OFF (Passive sniffer)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 125, "* Rosen Master (DIP 1000) must be active", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 155, "Once battery transmits, cell delta &", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 175, "voltages will appear immediately.", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p1X + 20, midY + 210, "See Page 3 (CAN Scanner) for raw frame traffic.", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
    }

    // --- RIGHT PANEL: SYSTEM LIMITS & OPERATIONAL STATUS ---
    int p2X = 404;
    int p2W = 388;
    fillRect(p2X, midY, p2W, midH, COLOR_CARD_BG);
    drawRect(p2X, midY, p2W, midH, COLOR_CARD_BORDER);
    fillRect(p2X, midY, p2W, 24, COLOR_DARK_BLUE);
    drawString(p2X + 10, midY + 7, "INVERTER LIMITS & BMS STATUS", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "Charge Voltage Limit :  %.2f V", bData.chargeVoltageLimit_V);
        drawString(p2X + 15, midY + 34, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Charge Current Limit :  %.1f A  (~%.0f kW)",
                 bData.chargeCurrentLimit_A, (bData.chargeCurrentLimit_A * 54.0f) / 1000.0f);
        drawString(p2X + 15, midY + 54, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Discharge Current Max:  %.1f A  (~%.0f kW)",
                 bData.dischargeCurrentLimit_A, (bData.dischargeCurrentLimit_A * 51.0f) / 1000.0f);
        drawString(p2X + 15, midY + 74, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Discharge Cut-off    :  %.2f V",
                 bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f);
        drawString(p2X + 15, midY + 94, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        drawFastHLine(p2X + 15, midY + 116, p2W - 30, COLOR_DARK_GRAY);

        // Operational Switches Badges (Clean aligned columns)
        drawString(p2X + 15, midY + 128, "Charge Switch        :", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        if (bData.chargeAllowed) {
            fillRect(p2X + 195, midY + 124, 100, 18, COLOR_DARK_GREEN);
            drawRect(p2X + 195, midY + 124, 100, 18, COLOR_GREEN);
            drawString(p2X + 210, midY + 129, "ENABLED", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 195, midY + 124, 100, 18, COLOR_RED);
            drawRect(p2X + 195, midY + 124, 100, 18, COLOR_WHITE);
            drawString(p2X + 208, midY + 129, "DISABLED", COLOR_WHITE, COLOR_RED, 1);
        }

        drawString(p2X + 15, midY + 150, "Discharge Switch     :", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        if (bData.dischargeAllowed) {
            fillRect(p2X + 195, midY + 146, 100, 18, COLOR_DARK_GREEN);
            drawRect(p2X + 195, midY + 146, 100, 18, COLOR_GREEN);
            drawString(p2X + 210, midY + 151, "ENABLED", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 195, midY + 146, 100, 18, COLOR_RED);
            drawRect(p2X + 195, midY + 146, 100, 18, COLOR_WHITE);
            drawString(p2X + 208, midY + 151, "DISABLED", COLOR_WHITE, COLOR_RED, 1);
        }

        drawString(p2X + 15, midY + 172, "BMS Safety State     :", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        if (!bData.protectionActive && !bData.warningActive) {
            fillRect(p2X + 195, midY + 168, 175, 18, COLOR_DARK_GREEN);
            drawRect(p2X + 195, midY + 168, 175, 18, COLOR_GREEN);
            drawString(p2X + 205, midY + 173, "NORMAL / NO ALARMS", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 195, midY + 168, 175, 18, COLOR_RED);
            drawRect(p2X + 195, midY + 168, 175, 18, COLOR_WHITE);
            drawString(p2X + 205, midY + 173, "WARNING / ALARM", COLOR_WHITE, COLOR_RED, 1);
        }

        drawFastHLine(p2X + 15, midY + 196, p2W - 30, COLOR_DARK_GRAY);

        drawString(p2X + 15, midY + 206, "Storage Configuration :  Rosen 200Ah + RPT 300Ah", COLOR_GREEN, COLOR_CARD_BG, 1);
        snprintf(subBuf, sizeof(subBuf), "Inverter Protocol     :  %s CAN 500k (501 Ah Bank)",
                 strlen(bData.manufacturer) > 0 ? bData.manufacturer : "PYLON");
        drawString(p2X + 15, midY + 224, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
    } else {
        drawString(p2X + 20, midY + 40, "STANDBY CONFIGURATION", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 65, "Total Bank Capacity :  501 Ah (25.6 kWh)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 85, "Master Battery      :  Rosen (DIP 1 0 0 0)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 105, "Slave Battery       :  RPT Tower (DIP 0 1 0 0)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 125, "Cascade Interface   :  RS485 Inter-battery", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 145, "Max Bank Limits     :  390 A Charge / Discharge", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 165, "Inverter Link       :  Deye BMS CAN Port", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p2X + 20, midY + 210, "Status: Listening on CAN-H / CAN-L...", COLOR_GREEN, COLOR_CARD_BG, 1);
    }

    // 4. Bottom Status & Navigation Bar
    drawBottomNav(0);
}

// -----------------------------------------------------------------------------
// Phase 2: Page 2 - Cell Balance & Voltage Diagnostics (Dual Pack: 32 Cells)
// -----------------------------------------------------------------------------
void UIManager::drawCellDiagnostics(const BatteryData& bData, const ScannerOverview& overview) {
    fillScreen(COLOR_BLACK);

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "CELL VOLTAGE DIAGNOSTICS (32 CELLS)", COLOR_WHITE, COLOR_NAVY, 2);
    drawString(485, 8, "Rosen Master (16S) + RPT Slave (16S)", COLOR_CYAN, COLOR_NAVY, 1);

    uint32_t upSec = millis() / 1000;
    char upBuf2[48];
    snprintf(upBuf2, sizeof(upBuf2), "UP: %02lu:%02lu:%02lu  Bank Delta: %.0fmV",
             upSec / 3600, (upSec % 3600) / 60, upSec % 60, bData.cellDelta_mV);
    drawString(485, 24, upBuf2, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    char subBuf[64];
    int pW = 784, pX = 8;
    float avgCell = (bData.voltage_V > 10.0f) ? (bData.voltage_V / 16.0f) : 3.374f;

    // --- PANEL 1: BATTERI 1: ROSEN MASTER (Y: 48 to 232, H: 184) ---
    int p1Y = 48, p1H = 184;
    fillRect(pX, p1Y, pW, p1H, COLOR_CARD_BG);
    drawRect(pX, p1Y, pW, p1H, COLOR_CARD_BORDER);

    // Header Strip (H: 22)
    fillRect(pX, p1Y, pW, 22, COLOR_DARK_BLUE);
    drawString(pX + 10, p1Y + 6, "BATTERI 1: ROSEN MASTER (51.2V 200Ah) - 16 CELLS", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    float p1Min = bData.pack1_minV > 2.0f ? bData.pack1_minV : (bData.minCellVoltage_V > 2.0f ? bData.minCellVoltage_V : 3.367f);
    float p1Max = bData.pack1_maxV > 2.0f ? bData.pack1_maxV : (bData.maxCellVoltage_V > 2.0f ? bData.maxCellVoltage_V : 3.378f);
    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "Min: %.3fV  Max: %.3fV  dV: %.0fmV",
                 p1Min, p1Max, (p1Max - p1Min) * 1000.0f);
    } else {
        snprintf(subBuf, sizeof(subBuf), "Waiting CAN communication...");
    }
    drawString(pX + 480, p1Y + 6, subBuf, COLOR_WHITE, COLOR_DARK_BLUE, 1);

    // Pack 1 Chart Baseline & Bars
    int base1Y = p1Y + 155;
    int chartH = 95;
    drawFastHLine(pX + 10, base1Y, pW - 20, COLOR_MID_GRAY);

    for (int i = 0; i < 16; i++) {
        int barX = pX + 16 + i * 47;
        int barW = 34;
        float v = bData.pack1_cellVoltages[i];
        if (v < 2.0f) v = p1Min;

        // Zoom scale: 3.250V to 3.450V
        float clampedV = v;
        if (clampedV < 3.250f) clampedV = 3.250f;
        if (clampedV > 3.450f) clampedV = 3.450f;
        int bH = (int)(((clampedV - 3.250f) / 0.200f) * chartH);
        if (bH < 10) bH = 10;
        if (bH > chartH) bH = chartH;
        int barY = base1Y - bH;

        uint16_t bColor = COLOR_GREEN;
        if (fabs(v - bData.minCellVoltage_V) < 0.0015f) bColor = COLOR_CYAN;
        else if (fabs(v - bData.maxCellVoltage_V) < 0.0015f) bColor = COLOR_YELLOW;

        if (bData.communicationOK) {
            fillRect(barX, barY, barW, bH, bColor);
            drawRect(barX, barY, barW, bH, COLOR_WHITE);

            char mvStr[8];
            snprintf(mvStr, sizeof(mvStr), "%d", (int)(v * 1000.0f));
            drawString(barX + 5, barY - 10, mvStr, COLOR_WHITE, COLOR_CARD_BG, 1);
        } else {
            drawRect(barX, base1Y - 15, barW, 15, COLOR_MID_GRAY);
            drawString(barX + 5, base1Y - 25, "---", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }

        char cellLbl[6];
        snprintf(cellLbl, sizeof(cellLbl), "C%02d", i + 1);
        drawString(barX + 6, base1Y + 5, cellLbl, COLOR_CYAN, COLOR_CARD_BG, 1);
    }

    // --- PANEL 2: BATTERI 2: RPT SLAVE (Y: 238 to 424, H: 186) ---
    int p2Y = 238, p2H = 186;
    fillRect(pX, p2Y, pW, p2H, COLOR_CARD_BG);
    drawRect(pX, p2Y, pW, p2H, COLOR_CARD_BORDER);

    // Header Strip (H: 22)
    fillRect(pX, p2Y, pW, 22, COLOR_DARK_BLUE);
    drawString(pX + 10, p2Y + 6, "BATTERI 2: RPT TOWER SLAVE (51.2V 300Ah) - 16 CELLS", COLOR_GREEN, COLOR_DARK_BLUE, 1);

    float p2Min = bData.pack2_minV > 2.0f ? bData.pack2_minV : (bData.minCellVoltage_V > 2.0f ? bData.minCellVoltage_V : 3.369f);
    float p2Max = bData.pack2_maxV > 2.0f ? bData.pack2_maxV : (bData.maxCellVoltage_V > 2.0f ? bData.maxCellVoltage_V : 3.380f);
    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "Min: %.3fV  Max: %.3fV  dV: %.0fmV",
                 p2Min, p2Max, (p2Max - p2Min) * 1000.0f);
    } else {
        snprintf(subBuf, sizeof(subBuf), "Waiting CAN communication...");
    }
    drawString(pX + 480, p2Y + 6, subBuf, COLOR_WHITE, COLOR_DARK_BLUE, 1);

    // Pack 2 Chart Baseline & Bars
    int base2Y = p2Y + 155;
    drawFastHLine(pX + 10, base2Y, pW - 20, COLOR_MID_GRAY);

    for (int i = 0; i < 16; i++) {
        int barX = pX + 16 + i * 47;
        int barW = 34;
        float v = bData.pack2_cellVoltages[i];
        if (v < 2.0f) v = p2Max;

        float clampedV = v;
        if (clampedV < 3.250f) clampedV = 3.250f;
        if (clampedV > 3.450f) clampedV = 3.450f;
        int bH = (int)(((clampedV - 3.250f) / 0.200f) * chartH);
        if (bH < 10) bH = 10;
        if (bH > chartH) bH = chartH;
        int barY = base2Y - bH;

        uint16_t bColor = COLOR_GREEN;
        if (fabs(v - bData.minCellVoltage_V) < 0.0015f) bColor = COLOR_CYAN;
        else if (fabs(v - bData.maxCellVoltage_V) < 0.0015f) bColor = COLOR_YELLOW;

        if (bData.communicationOK) {
            fillRect(barX, barY, barW, bH, bColor);
            drawRect(barX, barY, barW, bH, COLOR_WHITE);

            char mvStr[8];
            snprintf(mvStr, sizeof(mvStr), "%d", (int)(v * 1000.0f));
            drawString(barX + 5, barY - 10, mvStr, COLOR_WHITE, COLOR_CARD_BG, 1);
        } else {
            drawRect(barX, base2Y - 15, barW, 15, COLOR_MID_GRAY);
            drawString(barX + 5, base2Y - 25, "---", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }

        char cellLbl[6];
        snprintf(cellLbl, sizeof(cellLbl), "C%02d", i + 1);
        drawString(barX + 6, base2Y + 5, cellLbl, COLOR_GREEN, COLOR_CARD_BG, 1);
    }

    // 3. Bottom Status & Navigation Bar
    drawBottomNav(1);
}

// -----------------------------------------------------------------------------
// Phase 1: Raw CAN Bus Scanner View
// -----------------------------------------------------------------------------
void UIManager::drawScanner(const ScannerOverview& overview) {
    fillScreen(COLOR_BLACK);

    CanIdStats idStats[14];
    size_t activeCount = CanReceiver::getInstance().getIdStatistics(idStats, 14);

    CanFrameRaw recentFrames[10];
    size_t recentCount = CanReceiver::getInstance().getRecentFrames(recentFrames, 10);

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "RPT & ROSEN CAN SCANNER", COLOR_WHITE, COLOR_NAVY, 2);
    drawString(340, 8, "Waveshare ESP32-S3 (Rev 1.2)", COLOR_CYAN, COLOR_NAVY, 1);

    uint32_t upSec = millis() / 1000;
    char upBuf3[48];
    uint32_t ageMs3 = (overview.last_packet_time_ms > 0 && millis() >= overview.last_packet_time_ms)
                          ? (millis() - overview.last_packet_time_ms) : 0;
    snprintf(upBuf3, sizeof(upBuf3), "UP: %02lu:%02lu:%02lu  RX: %lums ago",
             upSec / 3600, (upSec % 3600) / 60, upSec % 60, (unsigned long)ageMs3);
    drawString(340, 24, upBuf3, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // CAN Status Badge
    char canStatusStr[40];
    if (overview.can_listening) {
#if BOARD_CAN_POINT_TO_POINT
        snprintf(canStatusStr, sizeof(canStatusStr), "CAN: 500k [P2P-ACK]");
#else
        snprintf(canStatusStr, sizeof(canStatusStr), "CAN: 500k [LISTEN-ONLY]");
#endif
        fillRect(600, 8, 185, 26, COLOR_DARK_GREEN);
        drawRect(600, 8, 185, 26, COLOR_GREEN);
        drawString(610, 15, canStatusStr, COLOR_WHITE, COLOR_DARK_GREEN, 1);
    } else {
        snprintf(canStatusStr, sizeof(canStatusStr), "CAN: OFFLINE / ERR");
        fillRect(600, 8, 185, 26, COLOR_RED);
        drawRect(600, 8, 185, 26, COLOR_WHITE);
        drawString(610, 15, canStatusStr, COLOR_WHITE, COLOR_RED, 1);
    }

    // 2. Metrics Bar (Y: 46 to 78)
    fillRect(0, 46, LCD_WIDTH, 32, COLOR_DARK_GRAY);
    drawFastHLine(0, 78, LCD_WIDTH, COLOR_MID_GRAY);

    char statBuf[128];
    snprintf(statBuf, sizeof(statBuf), "Total Frames: %lu   Rate: %.1f fps   Bus Errors: %lu   Active IDs: %u",
             (unsigned long)overview.total_packets,
             overview.packets_per_sec,
             (unsigned long)overview.bus_error_count,
             (unsigned int)overview.active_ids_count);
    drawString(15, 56, statBuf, COLOR_YELLOW, COLOR_DARK_GRAY, 1);

    // SD Status indicator
    if (SdLogger::getInstance().isMounted()) {
        snprintf(statBuf, sizeof(statBuf), "SD: LOGGING (%s, %lu)",
                 SdLogger::getInstance().getFileName(),
                 (unsigned long)SdLogger::getInstance().getLoggedCount());
        drawString(510, 56, statBuf, COLOR_GREEN, COLOR_DARK_GRAY, 1);
    } else {
        drawString(510, 56, "SD: NO CARD / UNMOUNTED", COLOR_ORANGE, COLOR_DARK_GRAY, 1);
    }

    // 3. Left Panel: Discovered CAN-IDs Table (X: 10, Y: 84, W: 470, H: 340)
    fillRect(10, 84, 470, 340, COLOR_BLACK);
    drawRect(10, 84, 470, 340, COLOR_MID_GRAY);
    fillRect(11, 85, 468, 22, COLOR_DARK_BLUE);
    drawString(18, 91, "DISCOVERED CAN-IDs & TIMING", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    // Column Headers
    drawString(18, 112, "CAN-ID    TYPE  DLC   COUNT    INTERVAL   LAST PAYLOAD (HEX)",
               COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    drawFastHLine(18, 122, 450, COLOR_MID_GRAY);

    int rowY = 130;
    if (activeCount == 0) {
        drawString(25, 160, "Listening on CAN bus (500 kbit/s)...", COLOR_YELLOW, COLOR_BLACK, 1);
        drawString(25, 185, "No frames received yet.", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
        drawString(25, 215, "Check wiring: RJ45 Pin 4=CAN-H, Pin 5=CAN-L", COLOR_CYAN, COLOR_BLACK, 1);
        drawString(25, 235, "Ensure battery BMS is ON and communicating.", COLOR_CYAN, COLOR_BLACK, 1);
    } else {
        for (size_t i = 0; i < activeCount && i < 14; i++) {
            const CanIdStats& s = idStats[i];

            // Alternate row background highlight
            if (i % 2 == 1) {
                fillRect(12, rowY - 2, 466, 16, 0x0842); // very subtle dark row tint
            }

            char rowBuf[64];
            uint16_t idColor = COLOR_WHITE;
            if (s.id == 0x351 || s.id == 0x355 || s.id == 0x356) idColor = COLOR_GREEN;
            else if (s.id == 0x359 || s.id == 0x35C || s.id == 0x35E) idColor = COLOR_CYAN;
            else if (s.id == 0x373 || s.id == 0x379) idColor = COLOR_YELLOW;

            snprintf(rowBuf, sizeof(rowBuf), "0x%03X%s   %s    %u   %6lu    %5lums",
                      (unsigned int)s.id,
                      (s.id < 0x100 ? " " : ""),
                      s.extended ? "EXT" : "STD",
                      s.dlc,
                      (unsigned long)s.count,
                      (unsigned long)s.interval_ms);
            drawString(18, rowY, rowBuf, idColor, COLOR_BLACK, 1);

            char payloadHex[32] = "";
            for (int b = 0; b < s.dlc && b < 8; b++) {
                char byteStr[6];
                snprintf(byteStr, sizeof(byteStr), "%02X ", s.last_data[b]);
                strcat(payloadHex, byteStr);
            }
            drawString(295, rowY, payloadHex, COLOR_YELLOW, COLOR_BLACK, 1);

            rowY += 18;
        }
    }

    // 4. Right Panel: Live Frame Stream (X: 490, Y: 84, W: 300, H: 340)
    fillRect(490, 84, 300, 340, COLOR_BLACK);
    drawRect(490, 84, 300, 340, COLOR_MID_GRAY);
    fillRect(491, 85, 298, 22, COLOR_DARK_BLUE);
    drawString(498, 91, "LIVE CAN STREAM (NEWEST FIRST)", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    int streamY = 114;
    if (recentCount == 0) {
        drawString(510, 160, "No frames received yet.", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    } else {
        for (size_t i = 0; i < recentCount && i < 10; i++) {
            const CanFrameRaw& f = recentFrames[i];
            char frameLine[64];
            snprintf(frameLine, sizeof(frameLine), "+%4lums 0x%03X [%u]",
                     (unsigned long)(f.timestamp_ms % 10000),
                     (unsigned int)f.id,
                     f.dlc);
            drawString(498, streamY, frameLine, COLOR_CYAN, COLOR_BLACK, 1);

            char dataStr[32] = "";
            for (int b = 0; b < f.dlc && b < 8; b++) {
                char bHex[6];
                snprintf(bHex, sizeof(bHex), "%02X", f.data[b]);
                strcat(dataStr, bHex);
            }
            drawString(645, streamY, dataStr, COLOR_WHITE, COLOR_BLACK, 1);

            streamY += 21;
        }
    }

    // 5. Bottom Navigation Bar
    drawBottomNav(2);
}

// -----------------------------------------------------------------------------
// Unified Bottom Navigation Bar with 3 Touch Tabs
// -----------------------------------------------------------------------------
void UIManager::drawBottomNav(uint8_t activePage) {
    fillRect(0, 430, LCD_WIDTH, 50, COLOR_NAVY);
    drawFastHLine(0, 430, LCD_WIDTH, COLOR_CYAN);

    int tY = 435, tH = 38;

    // Tab 1: DASHBOARD (X: 10, W: 250)
    int t1X = 10, t1W = 250;
    if (activePage == 0) {
        fillRect(t1X, tY, t1W, tH, COLOR_DARK_BLUE);
        drawRect(t1X, tY, t1W, tH, COLOR_CYAN);
        drawString(t1X + 35, tY + 8, "[ 1. DASHBOARD ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t1X + 45, tY + 22, "Main Storage View", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t1X, tY, t1W, tH, COLOR_CARD_BG);
        drawRect(t1X, tY, t1W, tH, COLOR_MID_GRAY);
        drawString(t1X + 45, tY + 14, "1. DASHBOARD", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Tab 2: CELL DIAGNOSTICS (X: 275, W: 250)
    int t2X = 275, t2W = 250;
    if (activePage == 1) {
        fillRect(t2X, tY, t2W, tH, COLOR_DARK_BLUE);
        drawRect(t2X, tY, t2W, tH, COLOR_CYAN);
        drawString(t2X + 25, tY + 8, "[ 2. CELLS (32S) ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t2X + 35, tY + 22, "Rosen + RPT (16+16)", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t2X, tY, t2W, tH, COLOR_CARD_BG);
        drawRect(t2X, tY, t2W, tH, COLOR_MID_GRAY);
        drawString(t2X + 35, tY + 14, "2. CELLS (32S)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Tab 3: CAN SCANNER (X: 540, W: 250)
    int t3X = 540, t3W = 250;
    if (activePage == 2) {
        fillRect(t3X, tY, t3W, tH, COLOR_DARK_BLUE);
        drawRect(t3X, tY, t3W, tH, COLOR_CYAN);
        drawString(t3X + 40, tY + 8, "[ 3. CAN SCANNER ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t3X + 45, tY + 22, "Raw Telegram Inspector", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t3X, tY, t3W, tH, COLOR_CARD_BG);
        drawRect(t3X, tY, t3W, tH, COLOR_MID_GRAY);
        drawString(t3X + 50, tY + 14, "3. CAN SCANNER", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }
}

// -----------------------------------------------------------------------------
// High-Level Display Update Routine
// -----------------------------------------------------------------------------
void UIManager::updateDisplay() {
    if (!_framebuffer || !_panel_handle) return;

    // Check for user touch taps to switch pages
    checkTouch();

    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    if (_view_mode == UI_VIEW_DASHBOARD) {
        drawDashboard(bData, overview);
    } else if (_view_mode == UI_VIEW_CELL_DIAGNOSTICS) {
        drawCellDiagnostics(bData, overview);
    } else {
        drawScanner(overview);
    }

    // Push framebuffer to the RGB LCD display panel
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);
}

