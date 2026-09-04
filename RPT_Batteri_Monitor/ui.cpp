#include "ui.h"
#include "can_receiver.h"
#include "sd_logger.h"
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

// C-hook to allow other modules (like SD logger) to toggle SD_CS on CH422G

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager()
    : _panel_handle(nullptr),
      _framebuffer(nullptr),
      _ch422g_out_mask(0),
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
        Serial.printf("[UI WARNING] CH422G write to 0x%02X failed (code: %d)\n", reg_addr, err);
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

bool UIManager::begin() {
    Serial.println("[UI] Initializing Wire on SDA=GPIO8, SCL=GPIO9 (100 kHz)...");

    // 1. Initialize Wire bus at 100 kHz with 50ms timeout
    Wire.begin(BOARD_I2C_SDA_PIN, BOARD_I2C_SCL_PIN, BOARD_I2C_FREQ_HZ);
    Wire.setTimeOut(50);
    delay(50);

    // 2. Configure CH422G:
    // Write WR_SET (0x24): 0x01 enables general output mode (IO_OE = 1)
    Serial.println("[UI] Configuring CH422G WR_SET (0x24)...");
    writeCh422gReg(CH422G_I2C_ADDR_WR_SET, 0x01);
    delay(10);

    // Default pin outputs on 7" Waveshare:
    // EXIO1 (Touch RST) = 1 (active)
    // EXIO2 (LCD BL)    = 1 (Backlight ON)
    // EXIO3 (LCD RST)   = 1 (Not in reset)
    // EXIO4 (SD CS)     = 1 (CS idle, active low)
    // EXIO5 (CAN_SEL)   = 1 (CRITICAL: HIGH = CAN mode for onboard TJA1051)
    Serial.println("[UI] Configuring CH422G WR_IO (0x38)...");
    _ch422g_out_mask = CH422G_EXIO1_TP_RST |
                       CH422G_EXIO2_LCD_BL  |
                       CH422G_EXIO3_LCD_RST |
                       CH422G_EXIO4_SD_CS   |
                       CH422G_EXIO5_CAN_SEL;
    updateCh422gOutput();
    Serial.println("[UI] CH422G configured: CAN_SEL=HIGH, Backlight=ON.");

    // 3. Allocate RGB565 Framebuffer in PSRAM (800 * 480 * 2 = 768,000 bytes)
    Serial.println("[UI] Allocating 800x480 RGB framebuffer in PSRAM...");
    _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                              MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_framebuffer) {
        Serial.println("[UI] PSRAM allocation failed, attempting default memory allocation...");
        _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                                  MALLOC_CAP_DEFAULT);
    }

    if (!_framebuffer) {
        Serial.println("[UI ERROR] Failed to allocate framebuffer memory!");
        return false;
    }
    Serial.printf("[UI] Framebuffer ready at %p (%d KB)\n", _framebuffer, (LCD_WIDTH * LCD_HEIGHT * 2) / 1024);

    // 4. Initialize 7.0-inch 800x480 RGB LCD Panel via ESP-IDF esp_lcd
    Serial.println("[UI] Initializing 7.0-inch 800x480 RGB LCD Driver...");
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
    panel_conf.data_width = 16;
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
        Serial.printf("[UI ERROR] esp_lcd_new_rgb_panel failed: 0x%X\n", err);
        return false;
    }

    esp_lcd_panel_reset(_panel_handle);
    esp_lcd_panel_init(_panel_handle);

    // Initial clear & test render
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

    return true;
}

void UIManager::uiTaskTrampoline(void* arg) {
    reinterpret_cast<UIManager*>(arg)->uiTask();
}

void UIManager::uiTask() {
    Serial.println("[UI] Task running on Core 1.");
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
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        _framebuffer[i] = color;
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
// High-Level Dashboard Render Routine
// -----------------------------------------------------------------------------
void UIManager::updateDisplay() {
    if (!_framebuffer || !_panel_handle) return;

    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    CanIdStats idStats[14];
    size_t activeCount = CanReceiver::getInstance().getIdStatistics(idStats, 14);

    CanFrameRaw recentFrames[10];
    size_t recentCount = CanReceiver::getInstance().getRecentFrames(recentFrames, 10);

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "RPT BATTERY CAN MONITOR", COLOR_WHITE, COLOR_NAVY, 2);
    drawString(360, 10, "Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)", COLOR_CYAN, COLOR_NAVY, 1);

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

    // 2. Metrics Bar (Y: 48 to 78)
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

    // 3. Left Panel: Discovered CAN-IDs Table (X: 10, Y: 84, W: 470, H: 360)
    fillRect(10, 84, 470, 360, COLOR_BLACK);
    drawRect(10, 84, 470, 360, COLOR_MID_GRAY);
    fillRect(11, 85, 468, 22, COLOR_DARK_BLUE);
    drawString(18, 91, "DISCOVERED CAN-IDs & TIMING", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    // Column Headers
    drawString(18, 112, "CAN-ID    TYPE  DLC   COUNT    INTERVAL   LAST PAYLOAD (HEX)",
               COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    drawFastHLine(15, 124, 460, COLOR_MID_GRAY);

    int rowY = 130;
    if (activeCount == 0) {
        drawString(40, 200, "Waiting for CAN traffic from RPT/Deye bus...", COLOR_ORANGE, COLOR_BLACK, 1);
        drawString(40, 220, "Ensure CAN-H / CAN-L connected and 500 kbit/s active.", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    } else {
        for (size_t i = 0; i < activeCount && i < 12; i++) {
            const CanIdStats& s = idStats[i];
            char rowBuf[96];

            // Highlight known/suspected BMS CAN-IDs
            uint16_t idColor = COLOR_WHITE;
            if (s.id == 0x351 || s.id == 0x355 || s.id == 0x356 ||
                s.id == 0x359 || s.id == 0x35C || s.id == 0x35E) {
                idColor = COLOR_GREEN; // Known BMS ID
            }

            snprintf(rowBuf, sizeof(rowBuf), "0x%03X%s   %s   %u   %6lu   %5lums",
                     (unsigned int)s.id,
                     (s.id < 0x100 ? " " : ""),
                     s.extended ? "EXT" : "STD",
                     s.dlc,
                     (unsigned long)s.count,
                     (unsigned long)s.interval_ms);
            drawString(18, rowY, rowBuf, idColor, COLOR_BLACK, 1);

            // Format payload bytes
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

    // 4. Right Panel: Live Frame Stream (X: 490, Y: 84, W: 300, H: 360)
    fillRect(490, 84, 300, 360, COLOR_BLACK);
    drawRect(490, 84, 300, 360, COLOR_MID_GRAY);
    fillRect(491, 85, 298, 22, COLOR_DARK_BLUE);
    drawString(498, 91, "LIVE CAN STREAM (NEWEST FIRST)", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    int streamY = 114;
    if (recentCount == 0) {
        drawString(510, 160, "No frames received yet.", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    } else {
        for (size_t i = 0; i < recentCount && i < 11; i++) {
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

    // 5. Bottom Status Bar (Y: 450 to 480)
    fillRect(0, 450, LCD_WIDTH, 30, COLOR_NAVY);
    drawFastHLine(0, 450, LCD_WIDTH, COLOR_CYAN);
#if BOARD_CAN_POINT_TO_POINT
    drawString(15, 458, "STATUS: Dedicated RPT Battery CAN Link | Jumper 13: ON (120 Ohm Enabled)",
               COLOR_WHITE, COLOR_NAVY, 1);
#else
    drawString(15, 458, "STATUS: Passive Bus Sniffer Active | Jumper 13: OFF (120 Ohm Disabled)",
               COLOR_WHITE, COLOR_NAVY, 1);
#endif
    drawString(640, 458, "RPT Tower Monitor", COLOR_CYAN, COLOR_NAVY, 1);

    // Push framebuffer to the RGB LCD display panel
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);
}
