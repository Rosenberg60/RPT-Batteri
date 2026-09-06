// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : ui.cpp (UI & Graphics Engine - Dual Pack 32-Cell Tabs & Dashboard)
// DATO/TID: 2026-09-06 20:05:00
// =============================================================================

#include "ui.h"
#include "can_receiver.h"
#include "sd_logger.h"
#include "deye_bms_decoder.h"
#include "web_server.h"
#include "system_config.h"
#include "rs485_battery_manager.h"
#include <Wire.h>
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"

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

// VSYNC tracking & IRAM-safe ISR callback to prevent crashes during Flash/WiFi ops
static volatile bool s_vsync_occurred = false;
static bool s_diag_needs_full_redraw = true;

#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
IRAM_ATTR static bool rgb_lcd_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx) {
    s_vsync_occurred = true;
    return false;
}
#else
IRAM_ATTR static bool rgb_lcd_on_vsync_event(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx) {
    s_vsync_occurred = true;
    return false;
}
#endif

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager()
    : _panel_handle(nullptr),
      _framebuffer(nullptr),
      _ch422g_out_mask(0),
      _view_mode(UI_VIEW_DASHBOARD),
      _last_drawn_mode((UIViewMode)255),
      _cell_view_pack(0),
      _last_touch_ms(0),
      _initialized(false),
      _is_direct_fb(false)
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

    // 3. Initialize 7.0-inch 800x480 RGB LCD Panel via ESP-IDF esp_lcd
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
#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
    // 20 lines (16,000 px = 32KB per ping-pong buffer) in internal SRAM completely insulates
    // GDMA from Octal PSRAM bus contention during WiFi/CPU bursts, preventing screen drift
    panel_conf.bounce_buffer_size_px = LCD_WIDTH * 20;
#endif
    panel_conf.sram_trans_align = 4;
    panel_conf.psram_trans_align = 64;
    panel_conf.hsync_gpio_num = LCD_PIN_HSYNC;
    panel_conf.vsync_gpio_num = LCD_PIN_VSYNC;
    panel_conf.de_gpio_num = LCD_PIN_DE;
    panel_conf.pclk_gpio_num = LCD_PIN_PCLK;
    panel_conf.disp_gpio_num = -1;
#if !defined(ESP_IDF_VERSION) || (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
    panel_conf.on_frame_trans_done = rgb_lcd_on_vsync_event;
#endif

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

#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
    esp_lcd_rgb_panel_event_callbacks_t cbs = {};
    cbs.on_vsync = rgb_lcd_on_vsync_event;
    esp_lcd_rgb_panel_register_event_callbacks(_panel_handle, &cbs, NULL);
#endif

    esp_lcd_panel_reset(_panel_handle);
    esp_lcd_panel_init(_panel_handle);
    esp_lcd_panel_disp_on_off(_panel_handle, true);

    // 4. Extract direct scanout framebuffer from RGB panel driver to eliminate duplicate 768KB buffer
    uint16_t* direct_fb = nullptr;
#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
    esp_lcd_rgb_panel_get_frame_buffer(_panel_handle, 1, (void**)&direct_fb);
#else
    // In ESP-IDF 4.4 esp_lcd RGB panel driver: offset 72 (0x48) is uint8_t *fb (allocated in PSRAM)
    direct_fb = *(uint16_t**)((uint8_t*)_panel_handle + 72);
#endif
    if (direct_fb && ((uint32_t)direct_fb >= 0x3C000000 && (uint32_t)direct_fb < 0x3E000000)) {
        _framebuffer = direct_fb;
        _is_direct_fb = true;
        LOG_PRINTF("[UI] Direct zero-copy PSRAM scanout framebuffer active at %p\n", _framebuffer);
    } else {
        LOG_PRINTLN("[UI WARNING] Direct FB pointer not in PSRAM range, allocating fallback PSRAM buffer...");
        _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                                  MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!_framebuffer) {
            _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
                                                      MALLOC_CAP_DEFAULT);
        }
        _is_direct_fb = false;
    }

    if (!_framebuffer) {
        LOG_PRINTLN("[UI ERROR] Failed to allocate framebuffer memory!");
        return false;
    }

    // Initial clear & render test screen (triggers GDMA transmission start)
    fillScreen(COLOR_BLACK);
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);

    _initialized = true;

    // Spawn UI refresh task on Core 1 (12 KB stack prevents overflow from large local structs & snprintf)
    xTaskCreatePinnedToCore(
        uiTaskTrampoline,
        "ui_task",
        12288,
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
    uint32_t last_render_ms = 0;
    UIViewMode last_mode = _view_mode;
    while (_initialized) {
        checkTouch();
        uint32_t now = millis();
        // Redraw at 2.5 Hz (every 400ms) or immediately if page changed
        if (now - last_render_ms >= 400 || _view_mode != last_mode) {
            // Wait for VSYNC frame boundary before rendering to prevent tearing
            s_vsync_occurred = false;
            uint32_t vsync_wait_start = millis();
            while (!s_vsync_occurred && (millis() - vsync_wait_start < 40)) {
                vTaskDelay(pdMS_TO_TICKS(2));
            }
            updateDisplay();
            last_render_ms = millis();
            last_mode = _view_mode;
        }
        vTaskDelay(pdMS_TO_TICKS(40)); // 25 Hz touch polling (40ms response)
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

// -----------------------------------------------------------------------------
// Danish 5x7 Font Glyphs (Column-major, 5 cols x 7 rows, LSB at top row 0)
// Supports Æ, Ø, Å, æ, ø, å across all font sizes (size 1, 2, 3...)
// -----------------------------------------------------------------------------
static const uint8_t glyph_AE[5] PROGMEM  = { 0x7E, 0x09, 0x7F, 0x49, 0x49 }; // Æ
static const uint8_t glyph_OE[5] PROGMEM  = { 0x7E, 0x61, 0x5D, 0x43, 0x3F }; // Ø
static const uint8_t glyph_AA[5] PROGMEM  = { 0x70, 0x1A, 0x15, 0x1A, 0x70 }; // Å
static const uint8_t glyph_ae[5] PROGMEM  = { 0x20, 0x54, 0x7C, 0x54, 0x38 }; // æ
static const uint8_t glyph_oe[5] PROGMEM  = { 0x38, 0x64, 0x54, 0x4C, 0x3C }; // ø
static const uint8_t glyph_aa[5] PROGMEM  = { 0x20, 0x54, 0x55, 0x54, 0x78 }; // å
static const uint8_t glyph_deg[5] PROGMEM = { 0x00, 0x06, 0x09, 0x09, 0x06 }; // °

void UIManager::drawGlyph(int x, int y, const uint8_t* glyph, uint16_t color, uint16_t bg, uint8_t size) {
    if (!_framebuffer || !glyph) return;

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
    // Trailing column spacing (col 5) and bottom row spacing (row 7)
    if (bg != color) {
        if (size == 1) {
            drawFastVLine(x + 5, y, 8, bg);
            drawFastHLine(x, y + 7, 6, bg);
        } else {
            fillRect(x + 5 * size, y, size, 8 * size, bg);
            fillRect(x, y + 7 * size, 6 * size, size, bg);
        }
    }
}

void UIManager::drawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (!_framebuffer) return;
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t* glyph = &font5x7[(c - 0x20) * 5];
    drawGlyph(x, y, glyph, color, bg, size);
}

void UIManager::drawString(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t size) {
    if (!text) return;
    const uint8_t* p = (const uint8_t*)text;
    while (*p) {
        // UTF-8 2-byte sequence for Danish characters (0xC3 0xXX)
        if (*p == 0xC3 && *(p + 1)) {
            uint8_t c2 = *(p + 1);
            const uint8_t* g = nullptr;
            switch (c2) {
                case 0x86: g = glyph_AE; break; // Æ
                case 0x98: g = glyph_OE; break; // Ø
                case 0x85: g = glyph_AA; break; // Å
                case 0xA6: g = glyph_ae; break; // æ
                case 0xB8: g = glyph_oe; break; // ø
                case 0xA5: g = glyph_aa; break; // å
                default:   g = nullptr;  break;
            }
            if (g) {
                drawGlyph(x, y, g, color, bg, size);
                x += 6 * size;
                p += 2;
                continue;
            }
        }

        // UTF-8 degree symbol ° (0xC2 0xB0)
        if (*p == 0xC2 && *(p + 1) == 0xB0) {
            drawGlyph(x, y, glyph_deg, color, bg, size);
            x += 6 * size;
            p += 2;
            continue;
        }

        // Single-byte ISO-8859-1 / Windows-1252 Danish characters fallback
        if (*p >= 0x80) {
            const uint8_t* g = nullptr;
            switch (*p) {
                case 0xC6: g = glyph_AE; break; // Æ
                case 0xD8: g = glyph_OE; break; // Ø
                case 0xC5: g = glyph_AA; break; // Å
                case 0xE6: g = glyph_ae; break; // æ
                case 0xF8: g = glyph_oe; break; // ø
                case 0xE5: g = glyph_aa; break; // å
                case 0xB0: g = glyph_deg; break; // °
                default:   g = nullptr;  break;
            }
            if (g) {
                drawGlyph(x, y, g, color, bg, size);
                x += 6 * size;
                p++;
                continue;
            }
        }

        // Standard 7-bit ASCII
        drawChar(x, y, (char)*p, color, bg, size);
        x += 6 * size;
        p++;
    }
}

// -----------------------------------------------------------------------------
// Opaque Single-Pass Text Row Renderer (Eliminates Flicker)
// Renders text with background color directly, then clears only trailing remainder
// -----------------------------------------------------------------------------
void UIManager::drawTextRow(int x, int y, int maxW, const char* text, uint16_t color, uint16_t bg, uint8_t size) {
    if (!_framebuffer || !text) return;
    drawString(x, y, text, color, bg, size);

    // Measure rendered width in pixels (handling UTF-8 multi-byte characters)
    const uint8_t* p = (const uint8_t*)text;
    int glyphCount = 0;
    while (*p) {
        if (*p == 0xC3 && *(p + 1)) { p += 2; }
        else if (*p == 0xC2 && *(p + 1) == 0xB0) { p += 2; }
        else { p++; }
        glyphCount++;
    }
    int renderedW = glyphCount * 6 * size;
    if (renderedW < maxW) {
        fillRect(x + renderedW, y, maxW - renderedW, 8 * size, bg);
    }
}

// -----------------------------------------------------------------------------
// Touch Screen Handler (GT911 on I2C)
// Touch is EXCLUSIVELY active on the 3 bottom navigation buttons.
// The main screen area does NOT trigger touch/page toggle.
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
                    // 1. Only active within bottom navigation bar tabs (Y: 420 to 480)
                    if (touchY >= 420 && touchY <= 480) {
                        if (touchX >= 5 && touchX < 200) {
                            setPage(0); // Button 1: Dashboard
                            _last_touch_ms = now;
                        } else if (touchX >= 200 && touchX < 400) {
                            setPage(1); // Button 2: Cell Diagnostics (32S)
                            _last_touch_ms = now;
                        } else if (touchX >= 400 && touchX < 600) {
                            setPage(2); // Button 3: CAN Scanner
                            _last_touch_ms = now;
                        } else if (touchX >= 600 && touchX <= 795) {
                            setPage(3); // Button 4: Konfiguration
                            _last_touch_ms = now;
                        }
                    }
                    // 2. Interactive Touch for Page 4: RS485 Monitor & Configuration
                    else if (_view_mode == UI_VIEW_CONFIG) {
                        // Action buttons inside Box 4 (Y: 250 to 295)
                        if (touchY >= 250 && touchY <= 295) {
                            // Button 1: [ SKIFT BAUD ] (X: 414 to 590)
                            if (touchX >= 414 && touchX <= 590) {
                                Rs485BatteryManager::getInstance().cycleBaudrate();
                                _last_drawn_mode = (UIViewMode)255; // Redraw static page with new baud
                                _last_touch_ms = now;
                            }
                            // Button 2: [ TEST POLL NU ] (X: 606 to 782)
                            else if (touchX >= 606 && touchX <= 782) {
                                Rs485BatteryManager::getInstance().triggerImmediatePoll();
                                _last_touch_ms = now;
                            }
                        }
                    }
                    // 3. Interactive Touch for Page 2: Cell Diagnostics Pack Selector
                    else if (_view_mode == UI_VIEW_CELL_DIAGNOSTICS) {
                        if (touchY >= 100 && touchY <= 140) {
                            if (touchX >= 8 && touchX <= 215) {
                                if (_cell_view_pack != 0) {
                                    _cell_view_pack = 0;
                                    s_diag_needs_full_redraw = true;
                                    _last_touch_ms = now;
                                }
                            } else if (touchX >= 220 && touchX <= 430) {
                                if (_cell_view_pack != 1) {
                                    _cell_view_pack = 1;
                                    s_diag_needs_full_redraw = true;
                                    _last_touch_ms = now;
                                }
                            }
                        }
                    }
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

static bool s_dash_needs_full_redraw = true;

// -----------------------------------------------------------------------------
// Phase 2: Page 1 - Graphical Battery Storage Dashboard (Static Layout)
// Drawn ONCE when entering Dashboard view to eliminate PSRAM bus saturation
// -----------------------------------------------------------------------------
void UIManager::drawStaticDashboard() {
    s_dash_needs_full_redraw = true;

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "BATTERY STORAGE DASHBOARD", COLOR_WHITE, COLOR_NAVY, 2);

    // 2. Four Large Focus Cards Frames (Y: 48 to 268, H: 220) - STØRRE FOKUS
    const int cardY = 48;
    const int cardH = 220;
    const int cardW = 190;

    // --- CARD 0: STATE OF CHARGE (SOC) ---
    int c0X = 8;
    fillRect(c0X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c0X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c0X, cardY, cardW, 24, COLOR_CARD_HEADER);
    drawString(c0X + 8, cardY + 7, "STATE OF CHARGE (SOC)", COLOR_CYAN, COLOR_CARD_HEADER, 1);
    // Large Battery Graphic Outline (W: 160, H: 16)
    drawRect(c0X + 12, 110, 160, 16, COLOR_WHITE);
    fillRect(c0X + 172, 113, 4, 10, COLOR_WHITE);

    // --- CARD 1: STORAGE POWER ---
    int c1X = 206;
    fillRect(c1X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c1X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c1X, cardY, cardW, 24, COLOR_CARD_HEADER);
    drawString(c1X + 8, cardY + 7, "STORAGE POWER", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    // --- CARD 2: BANK VOLTAGE ---
    int c2X = 404;
    fillRect(c2X, cardY, cardW, cardH, COLOR_CARD_BG);
    drawRect(c2X, cardY, cardW, cardH, COLOR_CARD_BORDER);
    fillRect(c2X, cardY, cardW, 24, COLOR_CARD_HEADER);
    drawString(c2X + 8, cardY + 7, "BANK VOLTAGE", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    // --- CARD 3: TOTAL & PEAK CURRENT ---
    int c3X = 602;
    int c3W = 190;
    fillRect(c3X, cardY, c3W, cardH, COLOR_CARD_BG);
    drawRect(c3X, cardY, c3W, cardH, COLOR_CARD_BORDER);
    fillRect(c3X, cardY, c3W, 24, COLOR_CARD_HEADER);
    drawString(c3X + 8, cardY + 7, "TOTAL & PEAK CURRENT", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    // 3. Lower Section: Compacted / Suppressed Detail Panels (Y: 274 to 424, H: 150)
    const int midY = 274;
    const int midH = 150;

    // --- LEFT PANEL: CELL HEALTH & BALANCE ---
    int p1X = 8;
    int p1W = 388;
    fillRect(p1X, midY, p1W, midH, COLOR_CARD_BG);
    drawRect(p1X, midY, p1W, midH, COLOR_CARD_BORDER);
    fillRect(p1X, midY, p1W, 22, COLOR_DARK_BLUE);
    drawString(p1X + 10, midY + 6, "CELLEBALANCE & VOLT EKSTREMER (32S)", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    // --- RIGHT PANEL: INVERTER LIMITS & STATUS ---
    int p2X = 404;
    int p2W = 388;
    fillRect(p2X, midY, p2W, midH, COLOR_CARD_BG);
    drawRect(p2X, midY, p2W, midH, COLOR_CARD_BORDER);
    fillRect(p2X, midY, p2W, 22, COLOR_DARK_BLUE);
    drawString(p2X + 10, midY + 6, "INVERTER SIKKERHED & STATUS", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    if (bData.communicationOK) {
        // Left Panel Static Elements (when online)
        // 3 boxes at Y: midY + 26 = 300, H = 34
        fillRect(p1X + 8, midY + 26, 116, 34, COLOR_DARK_GRAY);
        drawRect(p1X + 8, midY + 26, 116, 34, COLOR_MID_GRAY);
        drawString(p1X + 14, midY + 29, "MIN CELLE", COLOR_CYAN, COLOR_DARK_GRAY, 1);

        fillRect(p1X + 130, midY + 26, 116, 34, COLOR_DARK_GRAY);
        drawRect(p1X + 130, midY + 26, 116, 34, COLOR_MID_GRAY);
        drawString(p1X + 136, midY + 29, "MAX CELLE", COLOR_YELLOW, COLOR_DARK_GRAY, 1);

        fillRect(p1X + 252, midY + 26, 126, 34, COLOR_DARK_GRAY);
        drawRect(p1X + 252, midY + 26, 126, 34, COLOR_MID_GRAY);
        drawString(p1X + 258, midY + 29, "DELTA (dV)", COLOR_WHITE, COLOR_DARK_GRAY, 1);

        // Graphical Spread Gauge (3.00V to 3.65V)
        int gaugeX = p1X + 10;
        int gaugeY = midY + 66; // 340
        int gaugeW = 368;
        int gaugeH = 8;
        fillRect(gaugeX, gaugeY, gaugeW, gaugeH, COLOR_DARK_GRAY);
        drawRect(gaugeX, gaugeY, gaugeW, gaugeH, COLOR_MID_GRAY);
        drawString(gaugeX, gaugeY + 11, "3.00V Empty", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        drawString(gaugeX + 145, gaugeY + 11, "3.37V Rest", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(gaugeX + 300, gaugeY + 11, "3.65V Full", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

        drawString(p1X + 10, midY + 134, "-> Skift til fane [2. CELLS] for alle 32 celler grafisk", COLOR_CYAN, COLOR_CARD_BG, 1);

        // Right Panel Static Elements (when online)
        drawString(p2X + 10, midY + 134, "Protokol: Deye / Pylon 500k (P2P-ACK Aktiv)", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
    } else {
        // Left Panel Static Elements (when offline)
        drawString(p1X + 15, midY + 28, "WAITING FOR BATTERY TELEMETRY (RS485)...", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 46, "* RS485 Kabling : Stik J7 (A=Pin 1, B=Pin 2)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 62, "* Multidrop Bus : RPT ID=1, Rosen ID=2", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 78, "* RS485 Baud    : 9600 bps (eller 115200)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 94, "* CAN Gateway   : Sender 500k data til Deye Inverter", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 114, "Når batteri sender, vises celledata straks.", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 132, "Se fane [4. RS485 / GW] for Live Bus Monitor.", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

        // Right Panel Static Elements (when offline)
        const char* mN = SystemConfig::getInstance().getMasterBrandName();
        const char* sN = SystemConfig::getInstance().getSlaveBrandName();
        float mC = SystemConfig::getInstance().getBrandNominalCap(SystemConfig::getInstance().getMaster());
        float sC = SystemConfig::getInstance().getBrandNominalCap(SystemConfig::getInstance().getSlave());
        char bufLine[64];

        drawString(p2X + 15, midY + 28, "STANDBY KONFIGURATION", COLOR_YELLOW, COLOR_CARD_BG, 1);
        snprintf(bufLine, sizeof(bufLine), "Total Bank Kapacitet : %.0f Ah (%.1f kWh)", mC + sC, (mC + sC) * 51.2f / 1000.0f);
        drawString(p2X + 15, midY + 46, bufLine, COLOR_WHITE, COLOR_CARD_BG, 1);
        snprintf(bufLine, sizeof(bufLine), "Master Batteri       : %s %.0fAh", mN, mC);
        drawString(p2X + 15, midY + 62, bufLine, COLOR_WHITE, COLOR_CARD_BG, 1);
        snprintf(bufLine, sizeof(bufLine), "Slave Batteri        : %s %.0fAh", sN, sC);
        drawString(p2X + 15, midY + 78, bufLine, COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 94, "Maks Ladestrøm       : 390 A (Deye Inverter)", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 114, "Konfiguration        : Se fane [4. RS485 / GW]", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 132, "Status               : Poller RS485 telemetri...", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // 4. Bottom Navigation Bar (Tab 0 active)
    drawBottomNav(0);
}

// -----------------------------------------------------------------------------
// Phase 2: Page 1 - Graphical Battery Storage Dashboard (Dynamic In-Place Update)
// Updates ONLY changed numbers inside minimal bounding boxes (eliminates flicker)
// -----------------------------------------------------------------------------
void UIManager::updateDynamicDashboard(const BatteryData& bData, const ScannerOverview& overview) {
    // Redraw static layout if BMS communication state changed between offline and online
    static bool last_comm_ok = false;
    if (bData.communicationOK != last_comm_ok) {
        last_comm_ok = bData.communicationOK;
        drawStaticDashboard();
    }

    // State tracking variables to completely eliminate flicker on Page 1
    static int8_t s_last_wifi_connected = -1;
    static String s_last_ip = "___UNSET___";
    static uint32_t s_last_upSec = 0xFFFFFFFF;
    static uint8_t s_last_lipo_soc = 0xFF;
    static float s_last_lipo_v = -1.0f;
    static int8_t s_last_lipo_conn = -1;
    static uint32_t s_last_rx_update = 0xFFFFFFFF;
    static int s_last_badge_state = -1;

    // Card 0 cached state
    static uint8_t s_last_c0_soc = 0xFF;
    static uint8_t s_last_c0_p1_soc = 0xFF;
    static uint8_t s_last_c0_p2_soc = 0xFF;
    static uint16_t s_last_c0_cap = 0xFFFF;
    static uint8_t s_last_c0_soh = 0xFF;
    static float s_last_c0_kwh = -1.0f;

    // Card 1 cached state
    static float s_last_c1_power = -99999.0f;
    static int s_last_c1_mode = -1; // 0=standby, 1=chg, 2=dchg
    static float s_last_c1_p1_p = -99999.0f;
    static float s_last_c1_p2_p = -99999.0f;
    static float s_last_c1_chg_lim = -1.0f;
    static float s_last_c1_dchg_lim = -1.0f;

    // Card 2 cached state
    static float s_last_c2_v = -1.0f;
    static float s_last_c2_avg = -1.0f;
    static float s_last_c2_chg_v = -1.0f;
    static float s_last_c2_dchg_v = -1.0f;
    static float s_last_c2_delta = -1.0f;
    static float s_last_c2_min = -1.0f;

    // Card 3 cached state
    static float s_last_c3_cur = -99999.0f;
    static float s_last_c3_peak_chg = -1.0f;
    static float s_last_c3_peak_dchg = 1.0f;
    static float s_last_c3_p1_i = -99999.0f;
    static float s_last_c3_p2_i = -99999.0f;
    static float s_last_c3_temp = -999.0f;
    static float s_last_c3_chg_lim = -1.0f;
    static float s_last_c3_dchg_lim = -1.0f;

    // Lower panels cached state
    static float s_last_low_min_v = -1.0f;
    static float s_last_low_max_v = -1.0f;
    static float s_last_low_delta = -1.0f;
    static int s_last_gauge_min_px = -1;
    static int s_last_gauge_max_px = -1;
    static float s_last_min_temp = -999.0f;
    static float s_last_max_temp = -999.0f;
    static float s_last_bms_temp = -999.0f;
    static uint8_t s_last_bms_soh = 0xFF;
    static float s_last_low_kwh = -1.0f;

    static int8_t s_last_chg_allowed = -1;
    static int8_t s_last_dchg_allowed = -1;
    static int8_t s_last_alarm_state = -1;
    static float s_last_chg_v_lim = -1.0f;
    static float s_last_chg_a_lim = -1.0f;
    static float s_last_dchg_a_lim = -1.0f;
    static uint8_t s_last_r_p1_soc = 0xFF;
    static float s_last_r_p1_i = -999.0f;
    static float s_last_r_p1_w = -99999.0f;
    static uint8_t s_last_r_p2_soc = 0xFF;
    static float s_last_r_p2_i = -999.0f;
    static float s_last_r_p2_w = -99999.0f;

    // If a full redraw was requested, invalidate all caches
    if (s_dash_needs_full_redraw) {
        s_dash_needs_full_redraw = false;
        s_last_wifi_connected = -1;
        s_last_ip = "___UNSET___";
        s_last_upSec = 0xFFFFFFFF;
        s_last_lipo_soc = 0xFF;
        s_last_lipo_v = -1.0f;
        s_last_lipo_conn = -1;
        s_last_rx_update = 0xFFFFFFFF;
        s_last_badge_state = -1;
        s_last_c0_soc = 0xFF;
        s_last_c0_p1_soc = 0xFF;
        s_last_c0_p2_soc = 0xFF;
        s_last_c0_cap = 0xFFFF;
        s_last_c0_soh = 0xFF;
        s_last_c0_kwh = -1.0f;
        s_last_c1_power = -99999.0f;
        s_last_c1_mode = -1;
        s_last_c1_p1_p = -99999.0f;
        s_last_c1_p2_p = -99999.0f;
        s_last_c1_chg_lim = -1.0f;
        s_last_c1_dchg_lim = -1.0f;
        s_last_c2_v = -1.0f;
        s_last_c2_avg = -1.0f;
        s_last_c2_chg_v = -1.0f;
        s_last_c2_dchg_v = -1.0f;
        s_last_c2_delta = -1.0f;
        s_last_c2_min = -1.0f;
        s_last_c3_cur = -99999.0f;
        s_last_c3_peak_chg = -1.0f;
        s_last_c3_peak_dchg = 1.0f;
        s_last_c3_p1_i = -99999.0f;
        s_last_c3_p2_i = -99999.0f;
        s_last_c3_temp = -999.0f;
        s_last_c3_chg_lim = -1.0f;
        s_last_c3_dchg_lim = -1.0f;
        s_last_low_min_v = -1.0f;
        s_last_low_max_v = -1.0f;
        s_last_low_delta = -1.0f;
        s_last_gauge_min_px = -1;
        s_last_gauge_max_px = -1;
        s_last_min_temp = -999.0f;
        s_last_max_temp = -999.0f;
        s_last_bms_temp = -999.0f;
        s_last_bms_soh = 0xFF;
        s_last_low_kwh = -1.0f;
        s_last_chg_allowed = -1;
        s_last_dchg_allowed = -1;
        s_last_alarm_state = -1;
        s_last_chg_v_lim = -1.0f;
        s_last_chg_a_lim = -1.0f;
        s_last_dchg_a_lim = -1.0f;
        s_last_r_p1_soc = 0xFF;
        s_last_r_p1_i = -999.0f;
        s_last_r_p1_w = -99999.0f;
        s_last_r_p2_soc = 0xFF;
        s_last_r_p2_i = -999.0f;
        s_last_r_p2_w = -99999.0f;
    }

    // 1. Header Bar Updates (in-place text updates without wiping whole navy header)
    bool cur_wifi = BatteryWebServer::getInstance().isConnected();
    String cur_ip = BatteryWebServer::getInstance().getIpAddress();
    int8_t cur_wifi_state = cur_wifi ? 1 : 0;
    if (cur_wifi_state != s_last_wifi_connected || cur_ip != s_last_ip) {
        s_last_wifi_connected = cur_wifi_state;
        s_last_ip = cur_ip;
        char wifiBuf[48];
        if (cur_wifi) {
            snprintf(wifiBuf, sizeof(wifiBuf), "IP: %s", cur_ip.c_str());
            drawTextRow(325, 8, 140, wifiBuf, COLOR_GREEN, COLOR_NAVY, 1);
        } else {
            snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: %s", cur_ip.c_str());
            drawTextRow(325, 8, 140, wifiBuf, COLOR_YELLOW, COLOR_NAVY, 1);
        }
    }

    uint32_t upSec = millis() / 1000;
    if (upSec != s_last_upSec) {
        s_last_upSec = upSec;
        char upBuf[32];
        snprintf(upBuf, sizeof(upBuf), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
        drawTextRow(470, 8, 75, upBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);
    }

    // LiPo Battery Status (Option A via TP1 & GPIO 6)
    int8_t cur_lipo_conn = bData.lipo_connected ? 1 : 0;
    if (cur_lipo_conn != s_last_lipo_conn ||
        bData.lipo_soc_percent != s_last_lipo_soc ||
        fabsf(bData.lipo_voltage_V - s_last_lipo_v) >= 0.08f) {
        s_last_lipo_conn = cur_lipo_conn;
        s_last_lipo_soc = bData.lipo_soc_percent;
        s_last_lipo_v = bData.lipo_voltage_V;
        char lipoBuf[32];
        if (bData.lipo_connected) {
            snprintf(lipoBuf, sizeof(lipoBuf), "Lipo Bat: %.1fV (%u%%)", bData.lipo_voltage_V, bData.lipo_soc_percent);
            uint16_t lipoColor = (bData.lipo_soc_percent >= 40) ? COLOR_GREEN :
                                 ((bData.lipo_soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
            drawTextRow(325, 24, 140, lipoBuf, lipoColor, COLOR_NAVY, 1);
        } else {
            drawTextRow(325, 24, 140, "Lipo Bat: N/A", COLOR_MID_GRAY, COLOR_NAVY, 1);
        }
    }

    // RX Status
    if (bData.lastUpdate_ms != s_last_rx_update) {
        s_last_rx_update = bData.lastUpdate_ms;
        char rxBuf[32];
        if (bData.communicationOK && bData.lastUpdate_ms > 0) {
            uint32_t ageMs = (millis() >= bData.lastUpdate_ms) ? (millis() - bData.lastUpdate_ms) : 0;
            snprintf(rxBuf, sizeof(rxBuf), "RX: %lums", (unsigned long)ageMs);
        } else {
            snprintf(rxBuf, sizeof(rxBuf), "RX: Wait");
        }
        drawTextRow(470, 24, 75, rxBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);
    }

    // Dynamic names from persistent configuration
    const char* mName = SystemConfig::getInstance().getBrandName(SystemConfig::getInstance().getMaster());
    const char* sName = SystemConfig::getInstance().getBrandName(SystemConfig::getInstance().getSlave());

    // Header Right Badge: BMS Online / Modules
    int cur_badge_state = bData.communicationOK ? (bData.moduleCount >= 2 ? 2 : 1) : 0;
    if (cur_badge_state != s_last_badge_state) {
        s_last_badge_state = cur_badge_state;
        fillRect(548, 6, 242, 32, COLOR_NAVY);
        if (bData.communicationOK) {
            char packBadge[32];
            if (bData.pack2_online) {
                snprintf(packBadge, sizeof(packBadge), "BMS: 501Ah (2 PACKS)");
            } else {
                snprintf(packBadge, sizeof(packBadge), "BMS: %s MASTER", mName);
            }
            fillRect(550, 7, 235, 30, COLOR_DARK_GREEN);
            drawRect(550, 7, 235, 30, COLOR_GREEN);
            drawString(562, 16, packBadge, COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(570, 7, 215, 30, COLOR_RED);
            drawRect(570, 7, 215, 30, COLOR_WHITE);
            drawString(585, 16, "BMS: WAITING / OFFLINE", COLOR_WHITE, COLOR_RED, 1);
        }
    }

    const int cardW = 190;
    char valBuf[32];
    char subBuf[64];

    // --- CARD 0: STATE OF CHARGE (SOC) - ENLARGED HERO ---
    int c0X = 8;
    if (bData.communicationOK) {
        if (bData.soc_percent != s_last_c0_soc) {
            s_last_c0_soc = bData.soc_percent;
            uint16_t socColor = (bData.soc_percent >= 40) ? COLOR_GREEN :
                                ((bData.soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
            snprintf(valBuf, sizeof(valBuf), "%u %%", bData.soc_percent);
            drawTextRow(c0X + 14, 76, 120, valBuf, socColor, COLOR_CARD_BG, 4);

            int socFillW = (156 * bData.soc_percent) / 100;
            if (socFillW > 156) socFillW = 156;
            if (socFillW > 0) fillRect(c0X + 14, 112, socFillW, 12, socColor);
            if (socFillW < 156) fillRect(c0X + 14 + socFillW, 112, 156 - socFillW, 12, COLOR_CARD_BG);
        }

        if (bData.pack1_soc_percent != s_last_c0_p1_soc) {
            s_last_c0_p1_soc = bData.pack1_soc_percent;
            snprintf(subBuf, sizeof(subBuf), "%s Master: %u %%", mName, bData.pack1_soc_percent);
            drawTextRow(c0X + 10, 136, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        }

        if (bData.pack2_online) {
            if (bData.pack2_soc_percent != s_last_c0_p2_soc) {
                s_last_c0_p2_soc = bData.pack2_soc_percent;
                snprintf(subBuf, sizeof(subBuf), "%s Slave : %u %%", sName, bData.pack2_soc_percent);
                drawTextRow(c0X + 10, 154, cardW - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
            }
        } else {
            snprintf(subBuf, sizeof(subBuf), "%s Slave : OFFLINE", sName);
            drawTextRow(c0X + 10, 154, cardW - 14, subBuf, COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        }

        if (bData.totalCapacity_Ah != s_last_c0_cap) {
            s_last_c0_cap = bData.totalCapacity_Ah;
            snprintf(subBuf, sizeof(subBuf), "Total Kapacitet: %u Ah",
                     bData.totalCapacity_Ah > 0 ? bData.totalCapacity_Ah : 501);
            drawTextRow(c0X + 10, 172, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.soh_percent != s_last_c0_soh) {
            s_last_c0_soh = bData.soh_percent;
            snprintf(subBuf, sizeof(subBuf), "Sundhed SOH    : %u%%",
                     bData.soh_percent > 0 ? bData.soh_percent : 100);
            drawTextRow(c0X + 10, 190, cardW - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }

        float estKwh = (bData.totalCapacity_Ah * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
        if (fabsf(estKwh - s_last_c0_kwh) >= 0.1f) {
            s_last_c0_kwh = estKwh;
            snprintf(subBuf, sizeof(subBuf), "Est. Energi    : ~%.1f kWh", estKwh);
            drawTextRow(c0X + 10, 208, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);
        }
    } else {
        if (s_last_c0_soc != 0xFE) {
            s_last_c0_soc = 0xFE;
            drawTextRow(c0X + 14, 76, 120, "-- %", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
            fillRect(c0X + 14, 112, 156, 12, COLOR_CARD_BG);
            drawTextRow(c0X + 10, 136, cardW - 14, "Master: -- %", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c0X + 10, 154, cardW - 14, "Slave : -- %", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c0X + 10, 172, cardW - 14, "Total Kapacitet: 501 Ah", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c0X + 10, 190, cardW - 14, "Sundhed SOH    : 100%", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c0X + 10, 208, cardW - 14, "Est. Energi    : --.- kWh", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c0X + 10, 228, cardW - 14, "Status: Poller RS485...", COLOR_ORANGE, COLOR_CARD_BG, 1);
        }
    }

    // --- CARD 1: STORAGE POWER - ENLARGED HERO (FONT SIZE 4) ---
    int c1X = 206;
    if (bData.communicationOK) {
        float pKw = bData.power_W / 1000.0f;
        int curMode = (bData.power_W > 50.0f) ? 1 : ((bData.power_W < -50.0f) ? 2 : 0);

        if (curMode != s_last_c1_mode || fabsf(bData.power_W - s_last_c1_power) >= 15.0f) {
            s_last_c1_power = bData.power_W;
            bool modeChanged = (curMode != s_last_c1_mode);
            s_last_c1_mode = curMode;

            uint16_t pColor = COLOR_CYAN;
            if (curMode == 1) { // Charging
                pColor = COLOR_GREEN;
                if (pKw >= 10.0f) {
                    snprintf(valBuf, sizeof(valBuf), "+%.1fkW", pKw);
                } else {
                    snprintf(valBuf, sizeof(valBuf), "+%.2fkW", pKw);
                }
                if (modeChanged) {
                    fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GREEN);
                    drawRect(c1X + 12, 106, 166, 22, COLOR_GREEN);
                    drawString(c1X + 22, 112, "CHARGING / OPLADNING", COLOR_WHITE, COLOR_DARK_GREEN, 1);
                }
            } else if (curMode == 2) { // Discharging
                pColor = COLOR_ORANGE;
                if (pKw <= -10.0f) {
                    snprintf(valBuf, sizeof(valBuf), "%.1fkW", pKw);
                } else {
                    snprintf(valBuf, sizeof(valBuf), "%.2fkW", pKw);
                }
                if (modeChanged) {
                    fillRect(c1X + 12, 106, 166, 22, 0x8200);
                    drawRect(c1X + 12, 106, 166, 22, COLOR_ORANGE);
                    drawString(c1X + 22, 112, "DISCHARGING / AFLAD", COLOR_WHITE, 0x8200, 1);
                }
            } else { // Standby
                pColor = COLOR_CYAN;
                snprintf(valBuf, sizeof(valBuf), "0.00kW");
                if (modeChanged) {
                    fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GRAY);
                    drawRect(c1X + 12, 106, 166, 22, COLOR_MID_GRAY);
                    drawString(c1X + 38, 112, "STANDBY DRIFT", COLOR_WHITE, COLOR_DARK_GRAY, 1);
                }
            }

            // Large Hero Font Size 4: Clear entire number slot cleanly to eliminate artifacts
            fillRect(c1X + 6, 74, 178, 32, COLOR_CARD_BG);
            drawString(c1X + 10, 76, valBuf, pColor, COLOR_CARD_BG, 4);

            snprintf(subBuf, sizeof(subBuf), "Effekt Total: %+.2f kW", pKw);
            drawTextRow(c1X + 10, 208, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);
        }

        if (fabsf(bData.pack1_power_W - s_last_c1_p1_p) >= 15.0f) {
            s_last_c1_p1_p = bData.pack1_power_W;
            snprintf(subBuf, sizeof(subBuf), "%s Effekt: %+.2f kW", mName, bData.pack1_power_W / 1000.0f);
            drawTextRow(c1X + 10, 136, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        }

        if (bData.pack2_online) {
            if (fabsf(bData.pack2_power_W - s_last_c1_p2_p) >= 15.0f) {
                s_last_c1_p2_p = bData.pack2_power_W;
                snprintf(subBuf, sizeof(subBuf), "%s Effekt: %+.2f kW", sName, bData.pack2_power_W / 1000.0f);
                drawTextRow(c1X + 10, 154, cardW - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
            }
        } else {
            snprintf(subBuf, sizeof(subBuf), "%s Effekt: OFFLINE", sName);
            drawTextRow(c1X + 10, 154, cardW - 14, subBuf, COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        }

        if (bData.chargeCurrentLimit_A != s_last_c1_chg_lim) {
            s_last_c1_chg_lim = bData.chargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Ladning: %.0f A (~21kW)", bData.chargeCurrentLimit_A);
            drawTextRow(c1X + 10, 172, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.dischargeCurrentLimit_A != s_last_c1_dchg_lim) {
            s_last_c1_dchg_lim = bData.dischargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Aflad  : %.0f A (~20kW)", bData.dischargeCurrentLimit_A);
            drawTextRow(c1X + 10, 190, cardW - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }
    } else {
        if (s_last_c1_mode != -2) {
            s_last_c1_mode = -2;
            drawTextRow(c1X + 10, 76, 172, "--- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GRAY);
            drawRect(c1X + 12, 106, 166, 22, COLOR_MID_GRAY);
            drawString(c1X + 38, 112, "STANDBY DRIFT", COLOR_WHITE, COLOR_DARK_GRAY, 1);
            drawTextRow(c1X + 10, 136, cardW - 14, "Rosen Effekt: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c1X + 10, 154, cardW - 14, "RPT   Effekt: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c1X + 10, 172, cardW - 14, "Maks Ladning: 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c1X + 10, 190, cardW - 14, "Maks Aflad  : 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c1X + 10, 208, cardW - 14, "Effekt Total: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c1X + 10, 228, cardW - 14, "Status: Poller RS485...", COLOR_ORANGE, COLOR_CARD_BG, 1);
        }
    }

    // --- CARD 2: BANK VOLTAGE - ENLARGED HERO ---
    int c2X = 404;
    if (bData.communicationOK) {
        if (fabsf(bData.voltage_V - s_last_c2_v) >= 0.02f) {
            s_last_c2_v = bData.voltage_V;
            snprintf(valBuf, sizeof(valBuf), "%.2f V", bData.voltage_V);
            drawTextRow(c2X + 12, 76, 168, valBuf, COLOR_YELLOW, COLOR_CARD_BG, 4);

            float avgCell = (bData.voltage_V > 10.0f) ? (bData.voltage_V / 16.0f) : 0.0f;
            if (fabsf(avgCell - s_last_c2_avg) >= 0.002f) {
                s_last_c2_avg = avgCell;
                snprintf(subBuf, sizeof(subBuf), "Avg Celle: %.3f V", avgCell);
                drawTextRow(c2X + 18, 114, 154, subBuf, COLOR_CYAN, COLOR_DARK_GRAY, 1);
            }
        }

        if (fabsf(bData.chargeVoltageLimit_V - s_last_c2_chg_v) >= 0.05f) {
            s_last_c2_chg_v = bData.chargeVoltageLimit_V;
            snprintf(subBuf, sizeof(subBuf), "Maks Ladesp. : %.2f V", bData.chargeVoltageLimit_V);
            drawTextRow(c2X + 10, 136, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        float cutoffV = bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f;
        if (fabsf(cutoffV - s_last_c2_dchg_v) >= 0.05f) {
            s_last_c2_dchg_v = cutoffV;
            snprintf(subBuf, sizeof(subBuf), "Aflad Cut-off: %.2f V", cutoffV);
            drawTextRow(c2X + 10, 154, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (fabsf(bData.cellDelta_mV - s_last_c2_delta) >= 1.0f) {
            s_last_c2_delta = bData.cellDelta_mV;
            snprintf(subBuf, sizeof(subBuf), "Celledelta dV: %.0f mV", bData.cellDelta_mV);
            drawTextRow(c2X + 10, 190, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);
        }

        if (fabsf(bData.minCellVoltage_V - s_last_c2_min) >= 0.002f) {
            s_last_c2_min = bData.minCellVoltage_V;
            snprintf(subBuf, sizeof(subBuf), "Spænding Min : %.3f V", bData.minCellVoltage_V);
            drawTextRow(c2X + 10, 208, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        }
    } else {
        if (s_last_c2_v != -2.0f) {
            s_last_c2_v = -2.0f;
            drawTextRow(c2X + 12, 76, 168, "--.-- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
            fillRect(c2X + 12, 108, 166, 20, COLOR_DARK_GRAY);
            drawRect(c2X + 12, 108, 166, 20, COLOR_MID_GRAY);
            drawString(c2X + 18, 114, "Avg Celle: -.--- V", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);
            drawTextRow(c2X + 10, 136, cardW - 14, "Maks Ladesp. : 57.60 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c2X + 10, 154, cardW - 14, "Aflad Cut-off: 44.80 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c2X + 10, 172, cardW - 14, "Fælles Busbar: 51.2V 16S", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c2X + 10, 190, cardW - 14, "Celledelta dV: -- mV", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c2X + 10, 208, cardW - 14, "Spænding Min : -.--- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c2X + 10, 228, cardW - 14, "Status: Poller RS485...", COLOR_ORANGE, COLOR_CARD_BG, 1);
        }
    }

    // --- CARD 3: TOTAL & PEAK CURRENT - ENLARGED HERO ---
    int c3X = 602;
    int c3W = 190;
    static float s_peak_chg_A = 0.0f;
    static float s_peak_dchg_A = 0.0f;
    if (bData.communicationOK) {
        if (bData.current_A > s_peak_chg_A) s_peak_chg_A = bData.current_A;
        if (bData.current_A < s_peak_dchg_A) s_peak_dchg_A = bData.current_A;

        if (fabsf(bData.current_A - s_last_c3_cur) >= 0.15f) {
            s_last_c3_cur = bData.current_A;
            uint16_t curColor = (bData.current_A > 0.5f) ? COLOR_GREEN :
                                ((bData.current_A < -0.5f) ? COLOR_ORANGE : COLOR_WHITE);

            // Always use Large Hero Font (Size 4): fits perfectly with 6-7 chars
            if (fabsf(bData.current_A) >= 100.0f) {
                snprintf(valBuf, sizeof(valBuf), "%+.0f A", bData.current_A);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%+.1f A", bData.current_A);
            }

            // Wipe entire number hero slot to cleanly eliminate any leftover old digits
            fillRect(c3X + 6, 74, 180, 34, COLOR_CARD_BG);
            drawString(c3X + 10, 76, valBuf, curColor, COLOR_CARD_BG, 4);
        }

        if (s_peak_chg_A != s_last_c3_peak_chg || s_peak_dchg_A != s_last_c3_peak_dchg) {
            s_last_c3_peak_chg = s_peak_chg_A;
            s_last_c3_peak_dchg = s_peak_dchg_A;
            float dchgPeak = (s_peak_dchg_A < -0.1f) ? s_peak_dchg_A : 0.0f;
            snprintf(subBuf, sizeof(subBuf), "Peak: +%.0fA / %.0fA", s_peak_chg_A, dchgPeak);
            drawTextRow(c3X + 16, 114, 158, subBuf, COLOR_YELLOW, 0x18C3, 1);
        }

        if (fabsf(bData.pack1_current_A - s_last_c3_p1_i) >= 0.15f) {
            s_last_c3_p1_i = bData.pack1_current_A;
            snprintf(subBuf, sizeof(subBuf), "%s Strøm : %+.1f A", mName, bData.pack1_current_A);
            drawTextRow(c3X + 10, 136, c3W - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        }

        if (bData.pack2_online) {
            if (fabsf(bData.pack2_current_A - s_last_c3_p2_i) >= 0.15f) {
                s_last_c3_p2_i = bData.pack2_current_A;
                snprintf(subBuf, sizeof(subBuf), "%s Strøm : %+.1f A", sName, bData.pack2_current_A);
                drawTextRow(c3X + 10, 154, c3W - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
            }
        } else {
            snprintf(subBuf, sizeof(subBuf), "%s Strøm : OFFLINE", sName);
            drawTextRow(c3X + 10, 154, c3W - 14, subBuf, COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        }

        if (fabsf(bData.temperature_C - s_last_c3_temp) >= 0.2f) {
            s_last_c3_temp = bData.temperature_C;
            snprintf(subBuf, sizeof(subBuf), "Batteri Temp: %.1f C", bData.temperature_C);
            drawTextRow(c3X + 10, 172, c3W - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.chargeCurrentLimit_A != s_last_c3_chg_lim) {
            s_last_c3_chg_lim = bData.chargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Ladestr: %.0f A", bData.chargeCurrentLimit_A);
            drawTextRow(c3X + 10, 190, c3W - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }

        if (bData.dischargeCurrentLimit_A != s_last_c3_dchg_lim) {
            s_last_c3_dchg_lim = bData.dischargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Aflad  : %.0f A", bData.dischargeCurrentLimit_A);
            drawTextRow(c3X + 10, 208, c3W - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }
    } else {
        if (s_last_c3_cur != -99998.0f) {
            s_last_c3_cur = -99998.0f;
            drawTextRow(c3X + 10, 76, 170, "---.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
            fillRect(c3X + 12, 108, 166, 20, COLOR_DARK_GRAY);
            drawRect(c3X + 12, 108, 166, 20, COLOR_MID_GRAY);
            drawTextRow(c3X + 16, 114, 158, "Peak: +0A / 0A", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);
            drawTextRow(c3X + 10, 136, c3W - 14, "Rosen Strøm : --.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c3X + 10, 154, c3W - 14, "RPT   Strøm : --.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c3X + 10, 172, c3W - 14, "Batteri Temp: --.- C", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c3X + 10, 190, c3W - 14, "Maks Ladestr: 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c3X + 10, 208, c3W - 14, "Maks Aflad  : 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
            drawTextRow(c3X + 10, 228, c3W - 14, "Status: Poller RS485...", COLOR_ORANGE, COLOR_CARD_BG, 1);
        }
    }

    // 3. Lower Section: Compacted Detail Panels In-Place Updates (midY = 274, midH = 150)
    const int midY = 274;
    int p1X = 8;
    int p1W = 388;
    int p2X = 404;
    int p2W = 388;

    if (bData.communicationOK) {
        // --- LEFT PANEL DYNAMIC UPDATES ---
        if (fabsf(bData.minCellVoltage_V - s_last_low_min_v) >= 0.002f) {
            s_last_low_min_v = bData.minCellVoltage_V;
            snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.minCellVoltage_V);
            drawTextRow(p1X + 14, midY + 39, 104, subBuf, COLOR_CYAN, COLOR_DARK_GRAY, 2);
        }

        if (fabsf(bData.maxCellVoltage_V - s_last_low_max_v) >= 0.002f) {
            s_last_low_max_v = bData.maxCellVoltage_V;
            snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.maxCellVoltage_V);
            drawTextRow(p1X + 136, midY + 39, 104, subBuf, COLOR_YELLOW, COLOR_DARK_GRAY, 2);
        }

        if (fabsf(bData.cellDelta_mV - s_last_low_delta) >= 1.0f) {
            s_last_low_delta = bData.cellDelta_mV;
            snprintf(subBuf, sizeof(subBuf), "%.0f mV", bData.cellDelta_mV);
            uint16_t deltaColor = (bData.cellDelta_mV < 20.0f) ? COLOR_GREEN :
                                  ((bData.cellDelta_mV < 50.0f) ? COLOR_YELLOW : COLOR_ORANGE);
            drawTextRow(p1X + 258, midY + 39, 112, subBuf, deltaColor, COLOR_DARK_GRAY, 2);
        }

        // Graphical Spread Gauge Fill (3.00V to 3.65V) - only update if bar coordinates change
        int gaugeX = p1X + 10;
        int gaugeY = midY + 66;
        int gaugeW = 368;
        int gaugeH = 8;
        float minV = bData.minCellVoltage_V > 2.8f ? bData.minCellVoltage_V : 3.0f;
        float maxV = bData.maxCellVoltage_V > 2.8f ? bData.maxCellVoltage_V : 3.0f;
        int minPx = gaugeX + (int)(((minV - 3.0f) / 0.65f) * gaugeW);
        int maxPx = gaugeX + (int)(((maxV - 3.0f) / 0.65f) * gaugeW);
        if (minPx < gaugeX + 2) minPx = gaugeX + 2;
        if (maxPx > gaugeX + gaugeW - 4) maxPx = gaugeX + gaugeW - 4;
        if (maxPx < minPx + 4) maxPx = minPx + 4;

        if (minPx != s_last_gauge_min_px || maxPx != s_last_gauge_max_px) {
            s_last_gauge_min_px = minPx;
            s_last_gauge_max_px = maxPx;
            fillRect(gaugeX + 1, gaugeY + 1, gaugeW - 2, gaugeH - 2, COLOR_DARK_GRAY);
            fillRect(minPx, gaugeY + 1, maxPx - minPx, gaugeH - 2, COLOR_CYAN);
            fillRect(minPx - 1, gaugeY + 1, 3, gaugeH - 2, COLOR_YELLOW);
            fillRect(maxPx - 1, gaugeY + 1, 3, gaugeH - 2, COLOR_GREEN);
        }

        if (bData.minCellTemp_C != s_last_min_temp || bData.maxCellTemp_C != s_last_max_temp) {
            s_last_min_temp = bData.minCellTemp_C;
            s_last_max_temp = bData.maxCellTemp_C;
            snprintf(subBuf, sizeof(subBuf), "Celle Temp  :  Min %.1f C  /  Max %.1f C",
                     bData.minCellTemp_C, bData.maxCellTemp_C);
            drawTextRow(p1X + 10, midY + 88, p1W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.temperature_C != s_last_bms_temp || bData.soh_percent != s_last_bms_soh) {
            s_last_bms_temp = bData.temperature_C;
            s_last_bms_soh = bData.soh_percent;
            snprintf(subBuf, sizeof(subBuf), "BMS Temp/SOH:  %.1f C   (Sundhed: %u%% SOH)",
                     bData.temperature_C, bData.soh_percent);
            drawTextRow(p1X + 10, midY + 104, p1W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        float estKwh = (bData.totalCapacity_Ah * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
        if (fabsf(estKwh - s_last_low_kwh) >= 0.1f) {
            s_last_low_kwh = estKwh;
            snprintf(subBuf, sizeof(subBuf), "Est. Energi :  ~%.1f kWh af 25.6 kWh", estKwh);
            drawTextRow(p1X + 10, midY + 120, p1W - 20, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
        }

        // --- RIGHT PANEL DYNAMIC UPDATES ---
        int8_t cur_chg = bData.chargeAllowed ? 1 : 0;
        if (cur_chg != s_last_chg_allowed) {
            s_last_chg_allowed = cur_chg;
            if (cur_chg) {
                fillRect(p2X + 8, midY + 26, 116, 22, COLOR_DARK_GREEN);
                drawRect(p2X + 8, midY + 26, 116, 22, COLOR_GREEN);
                drawString(p2X + 16, midY + 32, "LAD: TILLADT", COLOR_WHITE, COLOR_DARK_GREEN, 1);
            } else {
                fillRect(p2X + 8, midY + 26, 116, 22, COLOR_RED);
                drawRect(p2X + 8, midY + 26, 116, 22, COLOR_WHITE);
                drawString(p2X + 16, midY + 32, "LAD: STOPPET", COLOR_WHITE, COLOR_RED, 1);
            }
        }

        int8_t cur_dchg = bData.dischargeAllowed ? 1 : 0;
        if (cur_dchg != s_last_dchg_allowed) {
            s_last_dchg_allowed = cur_dchg;
            if (cur_dchg) {
                fillRect(p2X + 130, midY + 26, 116, 22, COLOR_DARK_GREEN);
                drawRect(p2X + 130, midY + 26, 116, 22, COLOR_GREEN);
                drawString(p2X + 134, midY + 32, "AFLAD: TILLADT", COLOR_WHITE, COLOR_DARK_GREEN, 1);
            } else {
                fillRect(p2X + 130, midY + 26, 116, 22, COLOR_RED);
                drawRect(p2X + 130, midY + 26, 116, 22, COLOR_WHITE);
                drawString(p2X + 134, midY + 32, "AFLAD: STOPPET", COLOR_WHITE, COLOR_RED, 1);
            }
        }

        int8_t cur_alarm = (!bData.protectionActive && !bData.warningActive) ? 0 : 1;
        if (cur_alarm != s_last_alarm_state) {
            s_last_alarm_state = cur_alarm;
            if (cur_alarm == 0) {
                fillRect(p2X + 252, midY + 26, 126, 22, COLOR_DARK_GREEN);
                drawRect(p2X + 252, midY + 26, 126, 22, COLOR_GREEN);
                drawString(p2X + 260, midY + 32, "ALARM: NORMAL", COLOR_WHITE, COLOR_DARK_GREEN, 1);
            } else {
                fillRect(p2X + 252, midY + 26, 126, 22, COLOR_RED);
                drawRect(p2X + 252, midY + 26, 126, 22, COLOR_WHITE);
                drawString(p2X + 260, midY + 32, "ALARM: AKTIV", COLOR_WHITE, COLOR_RED, 1);
            }
        }

        if (bData.chargeVoltageLimit_V != s_last_chg_v_lim) {
            s_last_chg_v_lim = bData.chargeVoltageLimit_V;
            snprintf(subBuf, sizeof(subBuf), "Ladespænding  :  %.2f V   (Cut-off: %.2f V)",
                     bData.chargeVoltageLimit_V,
                     bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f);
            drawTextRow(p2X + 10, midY + 54, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.chargeCurrentLimit_A != s_last_chg_a_lim) {
            s_last_chg_a_lim = bData.chargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Ladestrøm:  %.1f A   (~%.0f kW)",
                     bData.chargeCurrentLimit_A, (bData.chargeCurrentLimit_A * 54.0f) / 1000.0f);
            drawTextRow(p2X + 10, midY + 70, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.dischargeCurrentLimit_A != s_last_dchg_a_lim) {
            s_last_dchg_a_lim = bData.dischargeCurrentLimit_A;
            snprintf(subBuf, sizeof(subBuf), "Maks Afladning:  %.1f A   (~%.0f kW)",
                     bData.dischargeCurrentLimit_A, (bData.dischargeCurrentLimit_A * 51.0f) / 1000.0f);
            drawTextRow(p2X + 10, midY + 86, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);
        }

        if (bData.pack1_soc_percent != s_last_r_p1_soc ||
            fabsf(bData.pack1_current_A - s_last_r_p1_i) >= 0.2f ||
            fabsf(bData.pack1_power_W - s_last_r_p1_w) >= 20.0f) {
            s_last_r_p1_soc = bData.pack1_soc_percent;
            s_last_r_p1_i = bData.pack1_current_A;
            s_last_r_p1_w = bData.pack1_power_W;
            snprintf(subBuf, sizeof(subBuf), "%s (Master):  %u%% SOC | %+.1f A | %+.2f kW",
                     mName, bData.pack1_soc_percent, bData.pack1_current_A, bData.pack1_power_W / 1000.0f);
            drawTextRow(p2X + 10, midY + 104, p2W - 20, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);
        }

        if (bData.pack2_online) {
            if (bData.pack2_soc_percent != s_last_r_p2_soc ||
                fabsf(bData.pack2_current_A - s_last_r_p2_i) >= 0.2f ||
                fabsf(bData.pack2_power_W - s_last_r_p2_w) >= 20.0f) {
                s_last_r_p2_soc = bData.pack2_soc_percent;
                s_last_r_p2_i = bData.pack2_current_A;
                s_last_r_p2_w = bData.pack2_power_W;
                snprintf(subBuf, sizeof(subBuf), "%s (Slave) :  %u%% SOC | %+.1f A | %+.2f kW",
                         sName, bData.pack2_soc_percent, bData.pack2_current_A, bData.pack2_power_W / 1000.0f);
                drawTextRow(p2X + 10, midY + 120, p2W - 20, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
            }
        } else {
            snprintf(subBuf, sizeof(subBuf), "%s (Slave) :  OFFLINE / INGEN DATA", sName);
            drawTextRow(p2X + 10, midY + 120, p2W - 20, subBuf, COLOR_MID_GRAY, COLOR_CARD_BG, 1);
        }
    }
}

// State tracking for Page 3 flicker elimination
static bool s_scanner_needs_clear = true;

// -----------------------------------------------------------------------------
// Phase 2: Page 2 - Cell Balance & Voltage Diagnostics (Static Layout)
// Restored proven 16-cell profile with large legible bars and thermal/capacity panels
// -----------------------------------------------------------------------------
void UIManager::drawStaticCellDiagnostics() {
    s_diag_needs_full_redraw = true;

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "CELL BALANCE & VOLTAGE DIAGNOSTICS", COLOR_WHITE, COLOR_NAVY, 2);
    drawString(15, 26, "16-SERIES LiFePO4 INDIVIDUEL CELLEBALANCERING & DIAGNOSTIK", COLOR_CYAN, COLOR_NAVY, 1);

    const int statY = 48;
    const int statH = 54;
    const int statW = 190;

    // Card 1: MIN CELL
    fillRect(8, statY, statW, statH, COLOR_CARD_BG);
    drawRect(8, statY, statW, statH, COLOR_CARD_BORDER);
    drawString(16, statY + 6, "MIN CELLESPÆNDING", COLOR_CYAN, COLOR_CARD_BG, 1);
    drawString(125, statY + 28, "(Laveste)", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

    // Card 2: MAX CELL
    fillRect(206, statY, statW, statH, COLOR_CARD_BG);
    drawRect(206, statY, statW, statH, COLOR_CARD_BORDER);
    drawString(214, statY + 6, "MAX CELLESPÆNDING", COLOR_YELLOW, COLOR_CARD_BG, 1);
    drawString(320, statY + 28, "(Højeste)", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

    // Card 3: CELL DELTA (dV)
    fillRect(404, statY, statW, statH, COLOR_CARD_BG);
    drawRect(404, statY, statW, statH, COLOR_CARD_BORDER);
    drawString(412, statY + 6, "CELLEDELTA (dV)", COLOR_CYAN, COLOR_CARD_BG, 1);

    // Card 4: AVERAGE CELL
    fillRect(602, statY, statW, statH, COLOR_CARD_BG);
    drawRect(602, statY, statW, statH, COLOR_CARD_BORDER);
    drawString(610, statY + 6, "GENNEMSNIT / CELLE", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(715, statY + 28, "(Total/16)", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

    // 3. Main 16-Cell Vertical Bar Chart (Y: 106 to 328, H: 222)
    int grpX = 8, grpY = 106, grpW = 784, grpH = 222;
    fillRect(grpX, grpY, grpW, grpH, COLOR_CARD_BG);
    drawRect(grpX, grpY, grpW, grpH, COLOR_CARD_BORDER);

    // Chart header strip (Y: 106 to 132, H: 26)
    fillRect(grpX, grpY, grpW, 26, COLOR_DARK_BLUE);

    // Chart baseline & cell labels
    int baselineY = 282;
    drawFastHLine(grpX + 15, baselineY, grpW - 30, COLOR_MID_GRAY);

    for (int i = 0; i < 16; i++) {
        int barX = grpX + 22 + i * 46;
        char cellLbl[6];
        snprintf(cellLbl, sizeof(cellLbl), "C%02d", i + 1);
        drawString(barX + 6, baselineY + 5, cellLbl, COLOR_CYAN, COLOR_CARD_BG, 1);
    }

    // 4. Bottom Diagnostic Panels Row (Y: 332 to 424, H: 92)
    int p1X = 8, pY = 332, pW = 388, pH = 92;
    fillRect(p1X, pY, pW, pH, COLOR_CARD_BG);
    drawRect(p1X, pY, pW, pH, COLOR_CARD_BORDER);
    fillRect(p1X, pY, pW, 20, COLOR_DARK_BLUE);
    drawString(p1X + 10, pY + 5, "BATTERIPAKKE TEMPERATUR & HEALTH", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    drawString(p1X + 12, pY + 68, "Termisk Status : Optimal (< 30 C) | Køling: Passiv Normal", COLOR_GREEN, COLOR_CARD_BG, 1);

    int p2X = 404;
    fillRect(p2X, pY, pW, pH, COLOR_CARD_BG);
    drawRect(p2X, pY, pW, pH, COLOR_CARD_BORDER);
    fillRect(p2X, pY, pW, 20, COLOR_DARK_BLUE);
    drawString(p2X + 10, pY + 5, "BANK KAPACITET & BALANCERINGSSTATUS", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    drawString(p2X + 12, pY + 68, "BMS Balancering: Standby (Delta < 20mV er Optimal)", COLOR_CYAN, COLOR_CARD_BG, 1);

    // 5. Bottom Navigation Bar (Tab 1 active)
    drawBottomNav(1);
}

// -----------------------------------------------------------------------------
// Phase 2: Page 2 - Cell Balance & Voltage Diagnostics (Dynamic In-Place Update)
// -----------------------------------------------------------------------------
void UIManager::updateDynamicCellDiagnostics(const BatteryData& bData, const ScannerOverview& overview) {
    // 1. Header Bar Updates
    char wifiBuf[48];
    if (BatteryWebServer::getInstance().isConnected()) {
        snprintf(wifiBuf, sizeof(wifiBuf), "IP: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(450, 8, 160, wifiBuf, COLOR_GREEN, COLOR_NAVY, 1);
    } else {
        snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(450, 8, 160, wifiBuf, COLOR_YELLOW, COLOR_NAVY, 1);
    }

    uint32_t upSec = millis() / 1000;
    char upBuf2[32];
    snprintf(upBuf2, sizeof(upBuf2), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    drawTextRow(620, 8, 175, upBuf2, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // LiPo Battery Status
    char lipoBuf[32];
    if (bData.lipo_connected) {
        snprintf(lipoBuf, sizeof(lipoBuf), "Lipo Bat: %.1fV (%u%%)", bData.lipo_voltage_V, bData.lipo_soc_percent);
        uint16_t lipoColor = (bData.lipo_soc_percent >= 40) ? COLOR_GREEN :
                             ((bData.lipo_soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
        drawTextRow(450, 24, 160, lipoBuf, lipoColor, COLOR_NAVY, 1);
    } else {
        snprintf(lipoBuf, sizeof(lipoBuf), "Lipo Bat: N/A");
        drawTextRow(450, 24, 160, lipoBuf, COLOR_MID_GRAY, COLOR_NAVY, 1);
    }

    char deltaBuf[48];
    snprintf(deltaBuf, sizeof(deltaBuf), "Bank Delta: %.0fmV", bData.cellDelta_mV);
    drawTextRow(620, 24, 175, deltaBuf, COLOR_YELLOW, COLOR_NAVY, 1);

    // Auto-select online pack if current selection is offline
    if (_cell_view_pack == 1 && !bData.pack2_online && bData.pack1_online) {
        _cell_view_pack = 0;
    } else if (_cell_view_pack == 0 && !bData.pack1_online && bData.pack2_online) {
        _cell_view_pack = 1;
    }

    // Determine active battery pack for Page 2
    const float* activeCells = bData.cellVoltages;
    const char* curPackName = "BATTERIBANK";
    float packMinV = bData.minCellVoltage_V;
    float packMaxV = bData.maxCellVoltage_V;
    float packDelta = bData.cellDelta_mV;
    float packAvg = (bData.voltage_V > 10.0f) ? (bData.voltage_V / 16.0f) : 3.374f;
    uint16_t packSoc = bData.soc_percent;

    if (_cell_view_pack == 1 && (bData.pack2_cells_valid || bData.pack2_online)) {
        activeCells = bData.pack2_cellVoltages;
        curPackName = bData.pack2_name[0] ? bData.pack2_name : "ROSEN (200Ah)";
        packMinV = bData.pack2_minV;
        packMaxV = bData.pack2_maxV;
        packDelta = (packMaxV - packMinV) * 1000.0f;
        packSoc = bData.pack2_soc_percent;
        float sum = 0;
        for (int i = 0; i < 16; i++) sum += activeCells[i];
        packAvg = sum / 16.0f;
    } else if (_cell_view_pack == 0 && (bData.pack1_cells_valid || bData.pack1_online)) {
        activeCells = bData.pack1_cellVoltages;
        curPackName = bData.pack1_name[0] ? bData.pack1_name : "RPT (300Ah)";
        packMinV = bData.pack1_minV;
        packMaxV = bData.pack1_maxV;
        packDelta = (packMaxV - packMinV) * 1000.0f;
        packSoc = bData.pack1_soc_percent;
        float sum = 0;
        for (int i = 0; i < 16; i++) sum += activeCells[i];
        packAvg = sum / 16.0f;
    }

    char subBuf[64];
    const int statY = 48;

    // Card 1: MIN CELL
    snprintf(subBuf, sizeof(subBuf), "%.3f V", packMinV);
    drawTextRow(20, statY + 24, 100, subBuf, COLOR_CYAN, COLOR_CARD_BG, 2);

    // Card 2: MAX CELL
    snprintf(subBuf, sizeof(subBuf), "%.3f V", packMaxV);
    drawTextRow(218, statY + 24, 100, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 2);

    // Card 3: CELL DELTA (dV)
    snprintf(subBuf, sizeof(subBuf), "%.0f mV", packDelta);
    uint16_t dColor = (packDelta < 20.0f) ? COLOR_GREEN :
                      ((packDelta < 50.0f) ? COLOR_YELLOW : COLOR_ORANGE);
    drawTextRow(416, statY + 24, 90, subBuf, dColor, COLOR_CARD_BG, 2);
    if (packDelta < 20.0f) {
        drawTextRow(510, statY + 28, 75, "[PERFEKT]", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else if (packDelta < 50.0f) {
        drawTextRow(510, statY + 28, 75, "[BALANCERET]", COLOR_YELLOW, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(510, statY + 28, 75, "[UBALANCE]", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // Card 4: AVERAGE CELL
    snprintf(subBuf, sizeof(subBuf), "%.3f V", packAvg);
    drawTextRow(614, statY + 24, 100, subBuf, COLOR_WHITE, COLOR_CARD_BG, 2);

    // 3. Main 16-Cell Vertical Bar Chart
    int grpX = 8, grpY = 106, baselineY = 282, chartH = 145;
    static int s_diag_last_mv[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    static uint16_t s_diag_last_col[16] = { 0 };

    // Interactive Pack Tabs in Chart Header (Y: 106 to 132)
    const char* p1Name = bData.pack1_name[0] ? bData.pack1_name : "RPT (300Ah)";
    const char* p2Name = bData.pack2_name[0] ? bData.pack2_name : "ROSEN (200Ah)";

    uint16_t t1Bg = (_cell_view_pack == 0) ? COLOR_CYAN : COLOR_DARK_GRAY;
    uint16_t t1Fg = (_cell_view_pack == 0) ? COLOR_BLACK : COLOR_WHITE;
    fillRect(grpX + 8, grpY + 3, 195, 20, t1Bg);
    drawRect(grpX + 8, grpY + 3, 195, 20, COLOR_WHITE);
    char p1Tab[32];
    snprintf(p1Tab, sizeof(p1Tab), "1: %s [%s]", p1Name, bData.pack1_online ? "OK" : "--");
    drawString(grpX + 16, grpY + 8, p1Tab, t1Fg, t1Bg, 1);

    uint16_t t2Bg = (_cell_view_pack == 1) ? COLOR_CYAN : COLOR_DARK_GRAY;
    uint16_t t2Fg = (_cell_view_pack == 1) ? COLOR_BLACK : COLOR_WHITE;
    fillRect(grpX + 210, grpY + 3, 195, 20, t2Bg);
    drawRect(grpX + 210, grpY + 3, 195, 20, COLOR_WHITE);
    char p2Tab[32];
    snprintf(p2Tab, sizeof(p2Tab), "2: %s [%s]", p2Name, bData.pack2_online ? "OK" : "--");
    drawString(grpX + 218, grpY + 8, p2Tab, t2Fg, t2Bg, 1);

    drawString(grpX + 418, grpY + 8, "Tryk fane for at skifte | Skala 3.25V->3.45V", COLOR_LIGHT_GRAY, COLOR_DARK_BLUE, 1);

    bool forceRedraw = s_diag_needs_full_redraw;
    s_diag_needs_full_redraw = false;

    for (int i = 0; i < 16; i++) {
        int barX = grpX + 22 + i * 46;
        int barW = 34;
        float v = activeCells[i];
        if (v < 2.0f && packAvg > 2.0f) v = packAvg;

        // Scale: 3.250V to 3.450V (span 0.200V = 145px)
        float clampedV = v;
        if (clampedV < 3.250f) clampedV = 3.250f;
        if (clampedV > 3.450f) clampedV = 3.450f;
        int bH = (int)(((clampedV - 3.250f) / 0.200f) * chartH);
        if (bH < 15) bH = 15;
        if (bH > chartH) bH = chartH;
        int barY = baselineY - bH;

        uint16_t bColor = COLOR_GREEN;
        if (fabs(v - packMinV) < 0.0015f) {
            bColor = COLOR_CYAN;
        } else if (fabs(v - packMaxV) < 0.0015f) {
            bColor = COLOR_YELLOW;
        }

        int curMv = (int)(v * 1000.0f + 0.5f);
        if (!forceRedraw && curMv == s_diag_last_mv[i] && bColor == s_diag_last_col[i]) {
            continue;
        }
        s_diag_last_mv[i] = curMv;
        s_diag_last_col[i] = bColor;

        // Clear only bar column
        fillRect(barX - 2, baselineY - chartH - 14, barW + 4, chartH + 14, COLOR_CARD_BG);
        fillRect(barX, barY, barW, bH, bColor);
        drawRect(barX, barY, barW, bH, COLOR_WHITE);

        // Millivolts printed above bar
        char mvStr[8];
        snprintf(mvStr, sizeof(mvStr), "%d", curMv);
        drawString(barX + 5, barY - 10, mvStr, COLOR_WHITE, COLOR_CARD_BG, 1);

        // Delta from average below cell label
        int dMv = (int)((v - packAvg) * 1000.0f);
        char dBuf[8];
        snprintf(dBuf, sizeof(dBuf), "%+d", dMv);
        drawTextRow(barX + 6, baselineY + 18, 28, dBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // 4. Bottom Diagnostic Panels Row
    int p1X = 8, pY = 332;
    float curTMin = (_cell_view_pack == 1) ? 24.0f : bData.minCellTemp_C;
    float curTMax = (_cell_view_pack == 1) ? 25.0f : bData.maxCellTemp_C;
    snprintf(subBuf, sizeof(subBuf), "Pakke: %-13s   Temp: Min %.1f C / Max %.1f C",
             curPackName, curTMin, curTMax);
    drawTextRow(p1X + 12, pY + 27, 360, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

    float curI = (_cell_view_pack == 1) ? bData.pack2_current_A : bData.pack1_current_A;
    float curP = (_cell_view_pack == 1) ? (bData.pack2_power_W / 1000.0f) : (bData.pack1_power_W / 1000.0f);
    snprintf(subBuf, sizeof(subBuf), "Strøm: %+.1f A (%+.2f kW) | SOC: %u%% | SOH: %u%%",
             curI, curP, (unsigned int)packSoc, bData.soh_percent);
    drawTextRow(p1X + 12, pY + 47, 360, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

    int p2X = 404;
    float totalCap = (bData.totalCapacity_Ah > 0) ? bData.totalCapacity_Ah : 500.0f;
    float estKwh = (totalCap * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
    float totalKwh = totalCap * 51.2f / 1000.0f;

    snprintf(subBuf, sizeof(subBuf), "Fælles Bank: %.0f Ah (%.1f kWh) | Lagret: ~%.1f kWh",
             totalCap, totalKwh, estKwh);
    drawTextRow(p2X + 12, pY + 27, 360, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

    snprintf(subBuf, sizeof(subBuf), "Bank Strøm: %+.1f A | Total Spænding: %.2f V",
             bData.current_A, bData.voltage_V);
    drawTextRow(p2X + 12, pY + 47, 360, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
}

// -----------------------------------------------------------------------------
// CAN Scanner Helper: Decode CAN-ID function name and payload summary
// -----------------------------------------------------------------------------
static const char* getCanIdName(uint32_t id) {
    switch (id) {
        case 0x351: return "Grænser (V/A)";
        case 0x355: return "SOC / SOH %";
        case 0x356: return "Volt/Amp/Temp";
        case 0x359: return "BMS Alarmer";
        case 0x35C: return "BMS Kontakter";
        case 0x35E: return "Fabrikat-ID";
        case 0x370: return "Celler 1-4";
        case 0x371: return "Celler 5-8";
        case 0x372: return "Celler 9-12";
        case 0x373: return "Celle Ekstremer";
        case 0x374: return "Celler 13-16";
        case 0x379: return "Kapacitet (Ah)";
        default:    return "CAN Telegram";
    }
}

static void decodePayloadSummary(const CanIdStats& s, char* out, size_t outSize) {
    if (!out || outSize == 0) return;
    out[0] = '\0';

    switch (s.id) {
        case 0x351:
            if (s.dlc >= 6) {
                float vLim = ((uint16_t)s.last_data[1] << 8 | s.last_data[0]) * 0.1f;
                float cLim = ((uint16_t)s.last_data[3] << 8 | s.last_data[2]) * 0.1f;
                float dLim = ((uint16_t)s.last_data[5] << 8 | s.last_data[4]) * 0.1f;
                snprintf(out, outSize, "%.1fV | %.0fA Chg | %.0fA Dchg", vLim, cLim, dLim);
                return;
            }
            break;

        case 0x355:
            if (s.dlc >= 4) {
                uint16_t soc = ((uint16_t)s.last_data[1] << 8 | s.last_data[0]);
                uint16_t soh = ((uint16_t)s.last_data[3] << 8 | s.last_data[2]);
                if (s.dlc >= 6 && s.last_data[4] > 0 && s.last_data[4] <= 100) {
                    uint16_t soc2 = ((uint16_t)s.last_data[5] << 8 | s.last_data[4]);
                    snprintf(out, outSize, "SOC: %u%% | SOH: %u%% | P2: %u%%", soc, soh, soc2);
                } else {
                    snprintf(out, outSize, "SOC: %u%% | SOH: %u%%", soc, soh);
                }
                return;
            }
            break;

        case 0x356:
            if (s.dlc >= 6) {
                float v = ((uint16_t)s.last_data[1] << 8 | s.last_data[0]) * 0.01f;
                int16_t rawI = (int16_t)((uint16_t)s.last_data[3] << 8 | s.last_data[2]);
                float iA = rawI * 0.1f;
                int16_t rawT = (int16_t)((uint16_t)s.last_data[5] << 8 | s.last_data[4]);
                float tC = rawT * 0.1f;
                snprintf(out, outSize, "%.2fV | %+.1fA | %.1f C", v, iA, tC);
                return;
            }
            break;

        case 0x359:
            if (s.dlc >= 4) {
                bool prot = (s.last_data[0] != 0 || s.last_data[1] != 0);
                bool warn = (s.last_data[2] != 0 || s.last_data[3] != 0);
                if (!prot && !warn) {
                    snprintf(out, outSize, "Normal (Ingen fejl/alarmer)");
                } else {
                    snprintf(out, outSize, "ALARM: %02X %02X %02X %02X",
                             s.last_data[0], s.last_data[1], s.last_data[2], s.last_data[3]);
                }
                return;
            }
            break;

        case 0x35C:
            if (s.dlc >= 2) {
                bool chgEn = (s.last_data[0] & 0x80) != 0;
                bool dchgEn = (s.last_data[0] & 0x40) != 0;
                snprintf(out, outSize, "Chg: %s | Dchg: %s", chgEn ? "TILLADT" : "STOP", dchgEn ? "TILLADT" : "STOP");
                return;
            }
            break;

        case 0x35E:
            if (s.dlc >= 1) {
                char mfg[9];
                int len = s.dlc < 8 ? s.dlc : 8;
                for (int b = 0; b < len; b++) {
                    char c = (char)s.last_data[b];
                    mfg[b] = (c >= 0x20 && c <= 0x7E) ? c : '.';
                }
                mfg[len] = '\0';
                snprintf(out, outSize, "Fabrikat: \"%s\"", mfg);
                return;
            }
            break;

        case 0x373:
            if (s.dlc >= 4) {
                uint16_t minMv = ((uint16_t)s.last_data[1] << 8 | s.last_data[0]);
                uint16_t maxMv = ((uint16_t)s.last_data[3] << 8 | s.last_data[2]);
                snprintf(out, outSize, "Min:%umV Max:%umV dV:%umV", minMv, maxMv, (maxMv > minMv ? maxMv - minMv : 0));
                return;
            }
            break;

        case 0x379:
            if (s.dlc >= 2) {
                uint16_t capAh = ((uint16_t)s.last_data[1] << 8 | s.last_data[0]);
                snprintf(out, outSize, "Kapacitet: %u Ah nominel", capAh);
                return;
            }
            break;

        default:
            break;
    }

    // Default: Raw HEX
    for (int b = 0; b < s.dlc && b < 8; b++) {
        char byteStr[6];
        snprintf(byteStr, sizeof(byteStr), "%02X ", s.last_data[b]);
        strncat(out, byteStr, outSize - strlen(out) - 1);
    }
}

// -----------------------------------------------------------------------------
// Phase 1: Raw CAN Bus Scanner View (Static Layout)
// Drawn ONCE when entering CAN Scanner view to eliminate PSRAM bus saturation
// -----------------------------------------------------------------------------
void UIManager::drawStaticScanner() {
    s_scanner_needs_clear = true;

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "RPT & ROSEN CAN SCANNER", COLOR_WHITE, COLOR_NAVY, 2);

    // 2. Metrics Bar Background (Y: 46 to 78)
    fillRect(0, 46, LCD_WIDTH, 32, COLOR_DARK_GRAY);
    drawFastHLine(0, 78, LCD_WIDTH, COLOR_MID_GRAY);

    // 3. Left Panel: Discovered CAN-IDs Table Frame (X: 8, Y: 84, W: 480, H: 340)
    int p1X = 8, p1W = 480, p1H = 340;
    fillRect(p1X, 84, p1W, p1H, COLOR_BLACK);
    drawRect(p1X, 84, p1W, p1H, COLOR_MID_GRAY);
    fillRect(p1X + 1, 85, p1W - 2, 22, COLOR_DARK_BLUE);
    drawString(p1X + 8, 91, "BMS CAN-TELEGRAMMER (DEYE / PYLON V1.3)", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    // Column Headers
    drawString(p1X + 8, 111, "ID    FUNKTION          INTVL  ANTAL   DEKODET INDHOLD / STATUS",
               COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    drawFastHLine(p1X + 6, 122, p1W - 12, COLOR_MID_GRAY);

    // 4. Right Panel: Top = Live Frame Stream, Bottom = Info & Explanation
    int p2X = 496, p2W = 296;

    // --- BOX 4A: LIVE CAN TRAFIK (H: 154) ---
    int b1Y = 84, b1H = 154;
    fillRect(p2X, b1Y, p2W, b1H, COLOR_BLACK);
    drawRect(p2X, b1Y, p2W, b1H, COLOR_MID_GRAY);
    fillRect(p2X + 1, b1Y + 1, p2W - 2, 22, COLOR_DARK_BLUE);
    drawString(p2X + 8, b1Y + 7, "LIVE CAN STRØM (NYESTE FØRST)", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    drawString(p2X + 8, b1Y + 28, "TID (ms)   ID     DLC   DATA (HEX)", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
    drawFastHLine(p2X + 6, b1Y + 38, p2W - 12, COLOR_DARK_GRAY);

    // --- BOX 4B: HVAD VISES HER? (INFO & GUIDE, H: 180) - 100% STATIC ---
    int b2Y = 244, b2H = 180;
    fillRect(p2X, b2Y, p2W, b2H, COLOR_CARD_BG);
    drawRect(p2X, b2Y, p2W, b2H, COLOR_CARD_BORDER);
    fillRect(p2X + 1, b2Y + 1, p2W - 2, 22, COLOR_DARK_BLUE);
    drawString(p2X + 8, b2Y + 7, "HVAD BETYDER DISSE RAMMER?", COLOR_WHITE, COLOR_DARK_BLUE, 1);

    int infoY = b2Y + 30;
    drawString(p2X + 8, infoY, "* RS485: Multidrop poller alle 32 celler (P1 & P2).", COLOR_YELLOW, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* CAN GATEWAY: Sender Pylontech 500k til Deye.", COLOR_GREEN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x351: Dynamisk ladestrøm (throttler ved 3.42V).", COLOR_WHITE, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x355: Fælles vægtet SOC % og SOH %.", COLOR_WHITE, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x356: Bank total Volt, Strøm og højeste Temp.", COLOR_WHITE, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x359: BMS beskyttelse (afbryder ved 3.65V/2.8V).", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x35C: Charge / Discharge enable flags.", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x35E: Fabrikant identifikation (PYLON).", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* SD KORT: Logger alle rammer til CSV-fil.", COLOR_GREEN, COLOR_CARD_BG, 1);

    // 5. Bottom Navigation Bar (Tab 2 active)
    drawBottomNav(2);
}

// -----------------------------------------------------------------------------
// Phase 1: Raw CAN Bus Scanner View (Dynamic In-Place Update)
// -----------------------------------------------------------------------------
void UIManager::updateDynamicScanner(const ScannerOverview& overview, const BatteryData& bData) {
    CanIdStats idStats[14];
    size_t activeCount = CanReceiver::getInstance().getIdStatistics(idStats, 14);

    CanFrameRaw recentFrames[8];
    size_t recentCount = CanReceiver::getInstance().getRecentFrames(recentFrames, 8);

    // 1. Header Bar Updates in-place
    char wifiBuf[48];
    if (BatteryWebServer::getInstance().isConnected()) {
        snprintf(wifiBuf, sizeof(wifiBuf), "IP: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(325, 8, 140, wifiBuf, COLOR_GREEN, COLOR_NAVY, 1);
    } else {
        snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(325, 8, 140, wifiBuf, COLOR_YELLOW, COLOR_NAVY, 1);
    }

    uint32_t upSec = millis() / 1000;
    char upBuf3[32];
    snprintf(upBuf3, sizeof(upBuf3), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    drawTextRow(470, 8, 120, upBuf3, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // LiPo Battery Status (Option A)
    char lipoBuf[32];
    if (bData.lipo_connected) {
        snprintf(lipoBuf, sizeof(lipoBuf), "Lipo Bat: %.1fV (%u%%)", bData.lipo_voltage_V, bData.lipo_soc_percent);
        uint16_t lipoColor = (bData.lipo_soc_percent >= 40) ? COLOR_GREEN :
                             ((bData.lipo_soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
        drawTextRow(325, 24, 140, lipoBuf, lipoColor, COLOR_NAVY, 1);
    } else {
        snprintf(lipoBuf, sizeof(lipoBuf), "Lipo Bat: N/A");
        drawTextRow(325, 24, 140, lipoBuf, COLOR_MID_GRAY, COLOR_NAVY, 1);
    }

    uint32_t ageMs3 = (overview.last_packet_time_ms > 0 && millis() >= overview.last_packet_time_ms)
                          ? (millis() - overview.last_packet_time_ms) : 0;
    char rxBuf3[32];
    if (overview.total_packets > 0) {
        snprintf(rxBuf3, sizeof(rxBuf3), "RX: %lums ago", (unsigned long)ageMs3);
    } else {
        snprintf(rxBuf3, sizeof(rxBuf3), "RX: Waiting...");
    }
    drawTextRow(470, 24, 120, rxBuf3, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // CAN Status Badge (only redraw when listening state changes)
    static int s_last_can_listen = -1;
    int cur_can_listen = overview.can_listening ? 1 : 0;
    if (cur_can_listen != s_last_can_listen) {
        s_last_can_listen = cur_can_listen;
        fillRect(598, 6, 192, 32, COLOR_NAVY);
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
    }

    // 2. Metrics Bar In-Place Update
    char statBuf[128];
    snprintf(statBuf, sizeof(statBuf), "Total Frames: %lu   Rate: %.1f fps   Bus Errors: %lu   Active IDs: %u",
             (unsigned long)overview.total_packets,
             overview.packets_per_sec,
             (unsigned long)overview.bus_error_count,
             (unsigned int)overview.active_ids_count);
    drawTextRow(15, 56, 490, statBuf, COLOR_YELLOW, COLOR_DARK_GRAY, 1);

    // SD Status indicator
    if (SdLogger::getInstance().isMounted()) {
        snprintf(statBuf, sizeof(statBuf), "SD: LOGGING (%s, %lu)",
                 SdLogger::getInstance().getFileName(),
                 (unsigned long)SdLogger::getInstance().getLoggedCount());
        drawTextRow(510, 56, 275, statBuf, COLOR_GREEN, COLOR_DARK_GRAY, 1);
    } else {
        drawTextRow(510, 56, 275, "SD: NO CARD / UNMOUNTED", COLOR_ORANGE, COLOR_DARK_GRAY, 1);
    }

    // 3. Left Panel: Discovered CAN-IDs Table Rows Area (Y: 124 to 420)
    int p1X = 8, p1W = 480;
    int rowY = 127;
    static size_t s_last_activeCount = 0;

    if (activeCount == 0) {
        if (s_scanner_needs_clear || s_last_activeCount > 0) {
            fillRect(p1X + 2, 124, p1W - 4, 280, COLOR_BLACK);
            drawString(p1X + 15, 160, "Lytter på CAN bus (500 kbit/s)...", COLOR_YELLOW, COLOR_BLACK, 1);
            drawString(p1X + 15, 185, "Ingen rammer modtaget endnu.", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
            drawString(p1X + 15, 215, "* Tjek kabling: RJ45 Pin 4=CAN-H, Pin 5=CAN-L", COLOR_CYAN, COLOR_BLACK, 1);
            drawString(p1X + 15, 235, "* Jumper 13: Skal være AF (Listen-Only passive)", COLOR_CYAN, COLOR_BLACK, 1);
            drawString(p1X + 15, 255, "* Sikr at Rosen Master (DIP 1000) er tændt.", COLOR_CYAN, COLOR_BLACK, 1);
            s_scanner_needs_clear = false;
        }
    } else {
        if (s_scanner_needs_clear || s_last_activeCount == 0) {
            fillRect(p1X + 2, 124, p1W - 4, 280, COLOR_BLACK);
            s_scanner_needs_clear = false;
        }
        for (size_t i = 0; i < 14; i++) {
            if (i < activeCount) {
                const CanIdStats& s = idStats[i];
                uint16_t rowBg = (i % 2 == 1) ? 0x0842 : COLOR_BLACK;

                uint16_t idColor = COLOR_WHITE;
                if (s.id == 0x351 || s.id == 0x355 || s.id == 0x356) idColor = COLOR_GREEN;
                else if (s.id == 0x359 || s.id == 0x35C || s.id == 0x35E) idColor = COLOR_CYAN;
                else if (s.id == 0x373 || s.id == 0x379) idColor = COLOR_YELLOW;

                char idStr[10];
                snprintf(idStr, sizeof(idStr), "0x%03X", (unsigned int)s.id);
                drawTextRow(p1X + 6, rowY, 48, idStr, idColor, rowBg, 1);
                drawTextRow(p1X + 54, rowY, 112, getCanIdName(s.id), COLOR_CYAN, rowBg, 1);

                char intvlStr[12];
                snprintf(intvlStr, sizeof(intvlStr), "%4lums", (unsigned long)s.interval_ms);
                drawTextRow(p1X + 166, rowY, 50, intvlStr, COLOR_LIGHT_GRAY, rowBg, 1);

                char countStr[12];
                snprintf(countStr, sizeof(countStr), "%5lu", (unsigned long)s.count);
                drawTextRow(p1X + 216, rowY, 48, countStr, COLOR_LIGHT_GRAY, rowBg, 1);

                char payloadSummary[48];
                decodePayloadSummary(s, payloadSummary, sizeof(payloadSummary));
                drawTextRow(p1X + 264, rowY, 212, payloadSummary, COLOR_YELLOW, rowBg, 1);
            } else if (i < s_last_activeCount) {
                // Clear unused table slot only if it previously had content
                fillRect(p1X + 2, rowY - 2, p1W - 4, 18, COLOR_BLACK);
            }
            rowY += 19;
        }
    }
    s_last_activeCount = activeCount;

    // 4. Right Panel: Box 4A Live CAN Stream Rows Area (Y: 124 to 234)
    int p2X = 496, p2W = 296, b1Y = 84;
    int streamY = b1Y + 44;
    static size_t s_last_recentCount = 0;
    if (recentCount == 0) {
        if (s_last_recentCount > 0) {
            fillRect(p2X + 2, b1Y + 40, p2W - 4, 106, COLOR_BLACK);
            drawString(p2X + 15, b1Y + 75, "Afventer live pakker...", COLOR_LIGHT_GRAY, COLOR_BLACK, 1);
        }
    } else {
        for (size_t i = 0; i < 6; i++) {
            if (i < recentCount) {
                const CanFrameRaw& f = recentFrames[i];
                char frameLine[32];
                snprintf(frameLine, sizeof(frameLine), "+%4lums 0x%03X [%u]",
                         (unsigned long)(f.timestamp_ms % 10000),
                         (unsigned int)f.id,
                         f.dlc);
                drawTextRow(p2X + 6, streamY, 144, frameLine, COLOR_CYAN, COLOR_BLACK, 1);

                char dataStr[32] = "";
                for (int b = 0; b < f.dlc && b < 8; b++) {
                    char bHex[6];
                    snprintf(bHex, sizeof(bHex), "%02X", f.data[b]);
                    strcat(dataStr, bHex);
                }
                drawTextRow(p2X + 150, streamY, 142, dataStr, COLOR_WHITE, COLOR_BLACK, 1);
            } else if (i < s_last_recentCount) {
                fillRect(p2X + 2, streamY - 2, p2W - 4, 17, COLOR_BLACK);
            }
            streamY += 17;
        }
    }
    s_last_recentCount = recentCount;
}

// -----------------------------------------------------------------------------
// Unified Bottom Navigation Bar with 4 Touch Tabs
// -----------------------------------------------------------------------------
void UIManager::drawBottomNav(uint8_t activePage) {
    fillRect(0, 430, LCD_WIDTH, 50, COLOR_NAVY);
    drawFastHLine(0, 430, LCD_WIDTH, COLOR_CYAN);

    int tY = 435, tH = 38;
    int tabW = 188;

    // Tab 0: DASHBOARD (X: 8, W: 188)
    int t0X = 8;
    if (activePage == 0) {
        fillRect(t0X, tY, tabW, tH, COLOR_DARK_BLUE);
        drawRect(t0X, tY, tabW, tH, COLOR_CYAN);
        drawString(t0X + 18, tY + 8, "[ 1. DASHBOARD ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t0X + 28, tY + 22, "Main Storage View", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t0X, tY, tabW, tH, COLOR_CARD_BG);
        drawRect(t0X, tY, tabW, tH, COLOR_MID_GRAY);
        drawString(t0X + 28, tY + 14, "1. DASHBOARD", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Tab 1: CELL DIAGNOSTICS (X: 206, W: 188)
    int t1X = 206;
    if (activePage == 1) {
        fillRect(t1X, tY, tabW, tH, COLOR_DARK_BLUE);
        drawRect(t1X, tY, tabW, tH, COLOR_CYAN);
        drawString(t1X + 14, tY + 8, "[ 2. CELLS (16S) ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t1X + 22, tY + 22, "Cell Balance & OCV", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t1X, tY, tabW, tH, COLOR_CARD_BG);
        drawRect(t1X, tY, tabW, tH, COLOR_MID_GRAY);
        drawString(t1X + 26, tY + 14, "2. CELLS (16S)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Tab 2: CAN SCANNER (X: 404, W: 188)
    int t2X = 404;
    if (activePage == 2) {
        fillRect(t2X, tY, tabW, tH, COLOR_DARK_BLUE);
        drawRect(t2X, tY, tabW, tH, COLOR_CYAN);
        drawString(t2X + 14, tY + 8, "[ 3. CAN SCANNER ]", COLOR_WHITE, COLOR_DARK_BLUE, 1);
        drawString(t2X + 20, tY + 22, "Raw Frame Inspector", COLOR_CYAN, COLOR_DARK_BLUE, 1);
    } else {
        fillRect(t2X, tY, tabW, tH, COLOR_CARD_BG);
        drawRect(t2X, tY, tabW, tH, COLOR_MID_GRAY);
        drawString(t2X + 24, tY + 14, "3. CAN SCANNER", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Tab 3: RS485 & GATEWAY MONITOR (X: 602, W: 190)
    int t3X = 602;
    int t3W = 190;
    if (activePage == 3) {
        fillRect(t3X, tY, t3W, tH, COLOR_DARK_GREEN);
        drawRect(t3X, tY, t3W, tH, COLOR_GREEN);
        drawString(t3X + 14, tY + 8, "[ 4. RS485 / GW ]", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        drawString(t3X + 20, tY + 22, "Live Bus Monitor", COLOR_YELLOW, COLOR_DARK_GREEN, 1);
    } else {
        fillRect(t3X, tY, t3W, tH, COLOR_CARD_BG);
        drawRect(t3X, tY, t3W, tH, COLOR_MID_GRAY);
        drawString(t3X + 24, tY + 14, "4. RS485 / GW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }
}

// -----------------------------------------------------------------------------
// Page 4 - RS485 Bus Monitor & CAN Gateway Status (Static Layout)
// -----------------------------------------------------------------------------
void UIManager::drawStaticConfig() {
    // 1. Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "RS485 BUS MONITOR & CAN GATEWAY STATUS", COLOR_WHITE, COLOR_NAVY, 2);

    // =========================================================================
    // BOX 1 (Top Left, X: 8, Y: 48, W: 388, H: 172)
    // RS485 INDGANG (BATTERIER -> DISPLAY)
    // =========================================================================
    int b1X = 8, b1Y = 48, b1W = 388, b1H = 172;
    fillRect(b1X, b1Y, b1W, b1H, COLOR_CARD_BG);
    drawRect(b1X, b1Y, b1W, b1H, COLOR_CARD_BORDER);
    fillRect(b1X, b1Y, b1W, 22, COLOR_CARD_HEADER);
    drawString(b1X + 10, b1Y + 6, "RS485 INDGANG (BATTERIER -> DISPLAY)", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    drawString(b1X + 12, b1Y + 30, "Port & HW   : UART1 (TX:GPIO16, RX:GPIO15)", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 48, "Transceiver : SP3485 (Auto-DIR, 2-pin Stik J7)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 66, "Protokol    : Pylon ASCII / Modbus RTU", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 84, "Baudrate    : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 102, "TX Sendt    : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 120, "RX Modtaget : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 138, "Sidste RX   : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b1X + 12, b1Y + 154, "Status      : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

    // =========================================================================
    // BOX 2 (Top Right, X: 404, Y: 48, W: 388, H: 172)
    // BATTERI ENHEDER PÅ RS485 BUSSEN
    // =========================================================================
    int b2X = 404, b2Y = 48, b2W = 388, b2H = 172;
    fillRect(b2X, b2Y, b2W, b2H, COLOR_CARD_BG);
    drawRect(b2X, b2Y, b2W, b2H, COLOR_CARD_BORDER);
    fillRect(b2X, b2Y, b2W, 22, COLOR_CARD_HEADER);
    drawString(b2X + 10, b2Y + 6, "BATTERI ENHEDER PÅ RS485 BUSSEN", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    // Sub-card 1: RPT (300 Ah)
    int p1Y = b2Y + 28, p1H = 64;
    fillRect(b2X + 8, p1Y, b2W - 16, p1H, COLOR_DARK_GRAY);
    drawRect(b2X + 8, p1Y, b2W - 16, p1H, COLOR_MID_GRAY);
    drawString(b2X + 16, p1Y + 6, "ENHED 1: RPT (300 Ah) - DIP ADRESSE 1", COLOR_WHITE, COLOR_DARK_GRAY, 1);

    // Sub-card 2: ROSEN (200 Ah)
    int p2Y = b2Y + 98, p2H = 64;
    fillRect(b2X + 8, p2Y, b2W - 16, p2H, COLOR_DARK_GRAY);
    drawRect(b2X + 8, p2Y, b2W - 16, p2H, COLOR_MID_GRAY);
    drawString(b2X + 16, p2Y + 6, "ENHED 2: ROSEN (200 Ah) - DIP ADRESSE 2", COLOR_WHITE, COLOR_DARK_GRAY, 1);

    // =========================================================================
    // BOX 3 (Bottom Left, X: 8, Y: 226, W: 388, H: 190)
    // CAN UDGANG (DISPLAY -> DEYE INVERTER)
    // =========================================================================
    int b3X = 8, b3Y = 226, b3W = 388, b3H = 190;
    fillRect(b3X, b3Y, b3W, b3H, COLOR_CARD_BG);
    drawRect(b3X, b3Y, b3W, b3H, COLOR_CARD_BORDER);
    fillRect(b3X, b3Y, b3W, 22, COLOR_CARD_HEADER);
    drawString(b3X + 10, b3Y + 6, "CAN UDGANG (DISPLAY -> DEYE INVERTER)", COLOR_CYAN, COLOR_CARD_HEADER, 1);

    drawString(b3X + 12, b3Y + 30, "Port & HW   : TWAI Controller @ 500 kbit/s", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 48, "Transceiver : TJA1051 (EXIO5 CAN_SEL = Aktiv)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 66, "CAN Stik    : Stik J8 / Pin 4 (CAN-H), Pin 5 (CAN-L)", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 84, "CAN Rammer  : 0x351, 0x355, 0x356, 0x359, 0x35C, 0x35E", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 102, "Sikkerhed   : Dynamisk dV throttling aktiv", COLOR_YELLOW, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 120, "Gateway     : ", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 144, "Displayet samler RS485 telemetri fra RPT (300Ah) og", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
    drawString(b3X + 12, b3Y + 160, "Rosen (200Ah) og leverer samlet 500Ah data til Deye.", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

    // =========================================================================
    // BOX 4 (Bottom Right, X: 404, Y: 226, W: 388, H: 190)
    // TEST & HARDWARE FEJLFINDINGSGUIDE
    // =========================================================================
    int b4X = 404, b4Y = 226, b4W = 388, b4H = 190;
    fillRect(b4X, b4Y, b4W, b4H, COLOR_CARD_BG);
    drawRect(b4X, b4Y, b4W, b4H, COLOR_CARD_BORDER);
    fillRect(b4X, b4Y, b4W, 22, COLOR_CARD_HEADER);
    drawString(b4X + 10, b4Y + 6, "TEST & HARDWARE FEJLFINDINGSGUIDE", COLOR_YELLOW, COLOR_CARD_HEADER, 1);

    // Button 1: [ SKIFT BAUD ] (X: 416, Y: 254, W: 174, H: 36)
    int btn1X = 416, btnY = 254, btnW = 174, btnH = 36;
    fillRect(btn1X, btnY, btnW, btnH, COLOR_DARK_GRAY);
    drawRect(btn1X, btnY, btnW, btnH, COLOR_CYAN);
    drawString(btn1X + 18, btnY + 8, "[ SKIFT BAUD ]", COLOR_WHITE, COLOR_DARK_GRAY, 1);
    drawString(btn1X + 22, btnY + 22, "9600 <-> 115200", COLOR_CYAN, COLOR_DARK_GRAY, 1);

    // Button 2: [ TEST POLL NU ] (X: 604, Y: 254, W: 174, H: 36)
    int btn2X = 604;
    fillRect(btn2X, btnY, btnW, btnH, COLOR_DARK_GREEN);
    drawRect(btn2X, btnY, btnW, btnH, COLOR_GREEN);
    drawString(btn2X + 18, btnY + 8, "[ TEST POLL NU ]", COLOR_WHITE, COLOR_DARK_GREEN, 1);
    drawString(btn2X + 22, btnY + 22, "Send TX Foresp.", COLOR_YELLOW, COLOR_DARK_GREEN, 1);

    // Fejlfindingstekst
    drawString(b4X + 12, b4Y + 70, "1. HVIS 'RX Modtaget' = 0:", COLOR_YELLOW, COLOR_CARD_BG, 1);
    drawString(b4X + 26, b4Y + 84, "Byt om pa A og B pa stik J7! (Meget hyppigt)", COLOR_WHITE, COLOR_CARD_BG, 1);
    drawString(b4X + 12, b4Y + 102, "2. BATTERI DIP SWITCHES:", COLOR_YELLOW, COLOR_CARD_BG, 1);
    drawString(b4X + 26, b4Y + 116, "RPT = ID 1 (1:ON, 2:OFF, 3:OFF, 4:OFF)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b4X + 26, b4Y + 130, "Rosen = ID 2 (1:OFF, 2:ON, 3:OFF, 4:OFF)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    drawString(b4X + 12, b4Y + 148, "3. RJ45 PINOUT: Pin 7=A, Pin 8=B (eller Pin 1=B, 2=A)", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
    drawString(b4X + 12, b4Y + 164, "4. Tryk [SKIFT BAUD] hvis batteri korer 115200 bps.", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

    // Bottom Navigation Bar with Tab 3 active
    drawBottomNav(3);
}

void UIManager::updateDynamicConfig(const BatteryData& bData) {
    // Header IP & Uptime update
    char wifiBuf[48];
    if (BatteryWebServer::getInstance().isConnected()) {
        snprintf(wifiBuf, sizeof(wifiBuf), "IP: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(450, 8, 160, wifiBuf, COLOR_GREEN, COLOR_NAVY, 1);
    } else {
        snprintf(wifiBuf, sizeof(wifiBuf), "WiFi: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(450, 8, 160, wifiBuf, COLOR_YELLOW, COLOR_NAVY, 1);
    }

    uint32_t upSec = millis() / 1000;
    char upBuf[32];
    snprintf(upBuf, sizeof(upBuf), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    drawTextRow(620, 8, 175, upBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // Box 1: RS485 Dynamic Fields
    int b1X = 8, b1Y = 48;
    char dynBuf[96];

    // Baudrate
    snprintf(dynBuf, sizeof(dynBuf), "%lu bps", (unsigned long)Rs485BatteryManager::getInstance().getBaudrate());
    drawTextRow(b1X + 90, b1Y + 84, 280, dynBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

    // TX Sendt
    snprintf(dynBuf, sizeof(dynBuf), "%lu foresporgsler", (unsigned long)Rs485BatteryManager::getInstance().getTxQueries());
    drawTextRow(b1X + 90, b1Y + 102, 280, dynBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

    // RX Modtaget
    uint32_t rxBytes = Rs485BatteryManager::getInstance().getRawRxBytes();
    if (rxBytes == 0) {
        snprintf(dynBuf, sizeof(dynBuf), "0 bytes (Ingen signaler endnu)");
        drawTextRow(b1X + 90, b1Y + 120, 280, dynBuf, COLOR_ORANGE, COLOR_CARD_BG, 1);
    } else {
        snprintf(dynBuf, sizeof(dynBuf), "%lu bytes modtaget OK", (unsigned long)rxBytes);
        drawTextRow(b1X + 90, b1Y + 120, 280, dynBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
    }

    // Sidste RX Hex Preview
    const char* rxHex = Rs485BatteryManager::getInstance().getLastRxHexString();
    if (rxHex && strlen(rxHex) > 0) {
        drawTextRow(b1X + 90, b1Y + 138, 280, rxHex, COLOR_YELLOW, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(b1X + 90, b1Y + 138, 280, "Afventer data...", COLOR_MID_GRAY, COLOR_CARD_BG, 1);
    }

    // Status
    if (bData.pack1_online || bData.pack2_online) {
        drawTextRow(b1X + 90, b1Y + 154, 280, "FORBUNDET & MODTAGER DATA", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else if (rxBytes > 0) {
        drawTextRow(b1X + 90, b1Y + 154, 280, "RA DATA MODTAGET (AFKODER)", COLOR_YELLOW, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(b1X + 90, b1Y + 154, 280, "POLLER... (Tjek A/B eller Baud)", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // Box 2: Pack Status Dynamic Fields
    int b2X = 404, b2Y = 48;
    int p1Y = b2Y + 28;
    int p2Y = b2Y + 98;

    // Pack 1 (RPT)
    if (bData.pack1_online) {
        snprintf(dynBuf, sizeof(dynBuf), "ONLINE: %u%% SOC  |  %+.1fA  |  Min:%.3fV Max:%.3fV",
                 bData.pack1_soc_percent, bData.pack1_current_A, bData.pack1_minV, bData.pack1_maxV);
        drawTextRow(b2X + 16, p1Y + 24, 345, dynBuf, COLOR_GREEN, COLOR_DARK_GRAY, 1);
        drawTextRow(b2X + 16, p1Y + 42, 345, "Telemetri synkroniseret | 32S Celler OK", COLOR_CYAN, COLOR_DARK_GRAY, 1);
    } else {
        drawTextRow(b2X + 16, p1Y + 24, 345, "OFFLINE: Afventer svar pa ID 1...", COLOR_ORANGE, COLOR_DARK_GRAY, 1);
        drawTextRow(b2X + 16, p1Y + 42, 345, "Tjek RPT DIP: 1=ON, 2=OFF, 3=OFF, 4=OFF", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);
    }

    // Pack 2 (Rosen)
    if (bData.pack2_online) {
        snprintf(dynBuf, sizeof(dynBuf), "ONLINE: %u%% SOC  |  %+.1fA  |  Min:%.3fV Max:%.3fV",
                 bData.pack2_soc_percent, bData.pack2_current_A, bData.pack2_minV, bData.pack2_maxV);
        drawTextRow(b2X + 16, p2Y + 24, 345, dynBuf, COLOR_GREEN, COLOR_DARK_GRAY, 1);
        drawTextRow(b2X + 16, p2Y + 42, 345, "Telemetri synkroniseret | 16S Celler OK", COLOR_CYAN, COLOR_DARK_GRAY, 1);
    } else {
        drawTextRow(b2X + 16, p2Y + 24, 345, "OFFLINE: Afventer svar pa ID 2...", COLOR_ORANGE, COLOR_DARK_GRAY, 1);
        drawTextRow(b2X + 16, p2Y + 42, 345, "Tjek Rosen DIP: 1=OFF, 2=ON, 3=OFF, 4=OFF", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);
    }

    // Box 3: CAN Gateway Dynamic Status
    int b3X = 8, b3Y = 226;
    if (bData.communicationOK) {
        drawTextRow(b3X + 90, b3Y + 120, 280, "SENDER LIVE DATA TIL DEYE", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(b3X + 90, b3Y + 120, 280, "SENDER SIKKERHEDS-DATA (0A)", COLOR_YELLOW, COLOR_CARD_BG, 1);
    }
}

// -----------------------------------------------------------------------------
// High-Level Display Update Routine
// -----------------------------------------------------------------------------
void UIManager::updateDisplay() {
    if (!_framebuffer || !_panel_handle) return;

    // Static layout is rendered ONLY when switching to a new view page
    if (_view_mode != _last_drawn_mode) {
        _last_drawn_mode = _view_mode;
        fillScreen(COLOR_BLACK);
        if (_view_mode == UI_VIEW_DASHBOARD) {
            drawStaticDashboard();
        } else if (_view_mode == UI_VIEW_CELL_DIAGNOSTICS) {
            drawStaticCellDiagnostics();
        } else if (_view_mode == UI_VIEW_CAN_SCANNER) {
            drawStaticScanner();
        } else if (_view_mode == UI_VIEW_CONFIG) {
            drawStaticConfig();
        }
    }

    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    // Dynamic updates write ONLY changed values/bars into small sub-rectangles
    if (_view_mode == UI_VIEW_DASHBOARD) {
        updateDynamicDashboard(bData, overview);
    } else if (_view_mode == UI_VIEW_CELL_DIAGNOSTICS) {
        updateDynamicCellDiagnostics(bData, overview);
    } else if (_view_mode == UI_VIEW_CAN_SCANNER) {
        updateDynamicScanner(overview, bData);
    } else if (_view_mode == UI_VIEW_CONFIG) {
        updateDynamicConfig(bData);
    }
}


