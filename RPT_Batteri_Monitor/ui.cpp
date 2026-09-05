#include "ui.h"
#include "can_receiver.h"
#include "sd_logger.h"
#include "deye_bms_decoder.h"
#include "web_server.h"
#include <Wire.h>
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp32s3/rom/cache.h"

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
static volatile bool s_vsync_occurred = false;

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
#if !defined(ESP_IDF_VERSION) || (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
    panel_conf.on_frame_trans_done = [](esp_lcd_panel_handle_t, esp_lcd_rgb_panel_event_data_t*, void*) -> bool {
        s_vsync_occurred = true;
        return false;
    };
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
    cbs.on_vsync = [](esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void*) -> bool {
        s_vsync_occurred = true;
        return false;
    };
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
    Cache_WriteBack_All();
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
            // Wait for VSYNC frame boundary before rendering
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
                    // Only active within bottom navigation bar tabs (Y: 420 to 480)
                    if (touchY >= 420 && touchY <= 480) {
                        if (touchX >= 5 && touchX <= 265) {
                            setPage(0); // Button 1: Dashboard
                            _last_touch_ms = now;
                        } else if (touchX >= 270 && touchX <= 530) {
                            setPage(1); // Button 2: Cell Diagnostics (32S)
                            _last_touch_ms = now;
                        } else if (touchX >= 535 && touchX <= 795) {
                            setPage(2); // Button 3: CAN Scanner
                            _last_touch_ms = now;
                        }
                    }
                    // Outside the 3 buttons: completely ignored
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
// Phase 2: Page 1 - Graphical Battery Storage Dashboard (Static Layout)
// Drawn ONCE when entering Dashboard view to eliminate PSRAM bus saturation
// -----------------------------------------------------------------------------
void UIManager::drawStaticDashboard() {
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
        drawString(p1X + 15, midY + 28, "WAITING FOR BMS TELEMETRY...", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 46, "* Check CAN cabling: Pin 4=CAN-H, Pin 5=CAN-L", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 62, "* Inverter baud rate: 500 kbit/s (standard)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 78, "* Jumper 13: ON (120 Ohm Point-to-Point)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 94, "* Rosen Master: DIP 1000 skal være tændt", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 114, "Når batteri sender, vises celledata straks.", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p1X + 15, midY + 132, "Se fane [3. CAN SCANNER] for rå CAN rammer.", COLOR_MID_GRAY, COLOR_CARD_BG, 1);

        // Right Panel Static Elements (when offline)
        drawString(p2X + 15, midY + 28, "STANDBY KONFIGURATION", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 46, "Total Bank Kapacitet : 501 Ah (25.6 kWh)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 62, "Master Batteri       : Rosen 200Ah (DIP 1000)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 78, "Slave Batteri        : RPT 300Ah   (DIP 0100)", COLOR_WHITE, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 94, "Maks Ladestrøm       : 390 A (Deye Inverter)", COLOR_CYAN, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 114, "Forventet Fordeling  : 40% Rosen / 60% RPT", COLOR_YELLOW, COLOR_CARD_BG, 1);
        drawString(p2X + 15, midY + 132, "Status               : Afventer CAN telemetri...", COLOR_ORANGE, COLOR_CARD_BG, 1);
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

    // 1. Header Bar Updates (in-place text updates without wiping whole navy header)
    char wifiBuf[48];
    if (BatteryWebServer::getInstance().isConnected()) {
        snprintf(wifiBuf, sizeof(wifiBuf), "IP: %s", BatteryWebServer::getInstance().getIpAddress().c_str());
        drawTextRow(325, 8, 140, wifiBuf, COLOR_GREEN, COLOR_NAVY, 1);
    } else {
        drawTextRow(325, 8, 140, "WiFi: Standby", COLOR_YELLOW, COLOR_NAVY, 1);
    }

    uint32_t upSec = millis() / 1000;
    char upBuf[32];
    snprintf(upBuf, sizeof(upBuf), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    drawTextRow(470, 8, 75, upBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // LiPo Battery Status (Option A via TP1 & GPIO 6)
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

    char rxBuf[32];
    if (bData.communicationOK && bData.lastUpdate_ms > 0) {
        uint32_t ageMs = (millis() >= bData.lastUpdate_ms) ? (millis() - bData.lastUpdate_ms) : 0;
        snprintf(rxBuf, sizeof(rxBuf), "RX: %lums", (unsigned long)ageMs);
    } else {
        snprintf(rxBuf, sizeof(rxBuf), "RX: Wait");
    }
    drawTextRow(470, 24, 75, rxBuf, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // Header Right Badge: BMS Online / Modules (updated only when badge state changes)
    static int s_last_badge_state = -1;
    int cur_badge_state = bData.communicationOK ? (bData.moduleCount >= 2 ? 2 : 1) : 0;
    if (cur_badge_state != s_last_badge_state) {
        s_last_badge_state = cur_badge_state;
        fillRect(548, 6, 242, 32, COLOR_NAVY);
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
    }

    const int cardY = 48;
    const int cardW = 190;
    char valBuf[32];
    char subBuf[64];

    // --- CARD 0: STATE OF CHARGE (SOC) - ENLARGED HERO ---
    int c0X = 8;
    if (bData.communicationOK) {
        uint16_t socColor = (bData.soc_percent >= 40) ? COLOR_GREEN :
                            ((bData.soc_percent >= 20) ? COLOR_YELLOW : COLOR_RED);
        snprintf(valBuf, sizeof(valBuf), "%u %%", bData.soc_percent);
        drawTextRow(c0X + 14, 76, 120, valBuf, socColor, COLOR_CARD_BG, 4);

        int socFillW = (156 * bData.soc_percent) / 100;
        if (socFillW > 156) socFillW = 156;
        if (socFillW > 0) fillRect(c0X + 14, 112, socFillW, 12, socColor);
        if (socFillW < 156) fillRect(c0X + 14 + socFillW, 112, 156 - socFillW, 12, COLOR_CARD_BG);

        snprintf(subBuf, sizeof(subBuf), "Rosen Master: %u %%", bData.pack1_soc_percent);
        drawTextRow(c0X + 10, 136, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "RPT   Slave : %u %%", bData.pack2_soc_percent);
        drawTextRow(c0X + 10, 154, cardW - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Total Kapacitet: %u Ah",
                 bData.totalCapacity_Ah > 0 ? bData.totalCapacity_Ah : 501);
        drawTextRow(c0X + 10, 172, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Sundhed SOH    : %u%%",
                 bData.soh_percent > 0 ? bData.soh_percent : 100);
        drawTextRow(c0X + 10, 190, cardW - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        float estKwh = (bData.totalCapacity_Ah * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
        snprintf(subBuf, sizeof(subBuf), "Est. Energi    : ~%.1f kWh", estKwh);
        drawTextRow(c0X + 10, 208, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);

        drawTextRow(c0X + 10, 228, cardW - 14, "BMS Drift: Normal OK", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(c0X + 14, 76, 120, "-- %", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
        fillRect(c0X + 14, 112, 156, 12, COLOR_CARD_BG);
        drawTextRow(c0X + 10, 136, cardW - 14, "Rosen: -- % (Master 200A)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c0X + 10, 154, cardW - 14, "RPT  : -- % (Slave 300A)", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c0X + 10, 172, cardW - 14, "Total Kapacitet: 501 Ah", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c0X + 10, 190, cardW - 14, "Sundhed SOH    : 100%", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c0X + 10, 208, cardW - 14, "Est. Energi    : --.- kWh", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c0X + 10, 228, cardW - 14, "Status: Afventer CAN...", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // --- CARD 1: STORAGE POWER - ENLARGED HERO ---
    int c1X = 206;
    if (bData.communicationOK) {
        float pKw = bData.power_W / 1000.0f;
        // Always clear hero value box before rendering to prevent digit overlaps
        fillRect(c1X + 10, 74, 172, 30, COLOR_CARD_BG);
        if (bData.power_W > 50.0f) {
            if (pKw >= 10.0f) {
                snprintf(valBuf, sizeof(valBuf), "+%.1f kW", pKw);
            } else {
                snprintf(valBuf, sizeof(valBuf), "+%.2f kW", pKw);
            }
            drawTextRow(c1X + 10, 76, 172, valBuf, COLOR_GREEN, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GREEN);
            drawRect(c1X + 12, 106, 166, 22, COLOR_GREEN);
            drawString(c1X + 22, 112, "CHARGING / OPLADNING", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else if (bData.power_W < -50.0f) {
            if (pKw <= -10.0f) {
                snprintf(valBuf, sizeof(valBuf), "%.1f kW", pKw);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%.2f kW", pKw);
            }
            drawTextRow(c1X + 10, 76, 172, valBuf, COLOR_ORANGE, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, 106, 166, 22, 0x8200);
            drawRect(c1X + 12, 106, 166, 22, COLOR_ORANGE);
            drawString(c1X + 22, 112, "DISCHARGING / AFLAD", COLOR_WHITE, 0x8200, 1);
        } else {
            drawTextRow(c1X + 10, 76, 172, "0.00 kW", COLOR_CYAN, COLOR_CARD_BG, 3);
            fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GRAY);
            drawRect(c1X + 12, 106, 166, 22, COLOR_MID_GRAY);
            drawString(c1X + 38, 112, "STANDBY DRIFT", COLOR_WHITE, COLOR_DARK_GRAY, 1);
        }
        snprintf(subBuf, sizeof(subBuf), "Rosen Effekt: %+.2f kW", bData.pack1_power_W / 1000.0f);
        drawTextRow(c1X + 10, 136, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "RPT   Effekt: %+.2f kW", bData.pack2_power_W / 1000.0f);
        drawTextRow(c1X + 10, 154, cardW - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Ladning: %.0f A (~21kW)", bData.chargeCurrentLimit_A);
        drawTextRow(c1X + 10, 172, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Aflad  : %.0f A (~20kW)", bData.dischargeCurrentLimit_A);
        drawTextRow(c1X + 10, 190, cardW - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Effekt Total: %+.2f kW", pKw);
        drawTextRow(c1X + 10, 208, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);

        drawTextRow(c1X + 10, 228, cardW - 14, "Inverter Link: Aktiv", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        fillRect(c1X + 10, 74, 172, 30, COLOR_CARD_BG);
        drawTextRow(c1X + 10, 76, 172, "--- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 3);
        fillRect(c1X + 12, 106, 166, 22, COLOR_DARK_GRAY);
        drawRect(c1X + 12, 106, 166, 22, COLOR_MID_GRAY);
        drawString(c1X + 38, 112, "STANDBY DRIFT", COLOR_WHITE, COLOR_DARK_GRAY, 1);

        drawTextRow(c1X + 10, 136, cardW - 14, "Rosen Effekt: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c1X + 10, 154, cardW - 14, "RPT   Effekt: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c1X + 10, 172, cardW - 14, "Maks Ladning: 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c1X + 10, 190, cardW - 14, "Maks Aflad  : 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c1X + 10, 208, cardW - 14, "Effekt Total: --- kW", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c1X + 10, 228, cardW - 14, "Status: Afventer CAN...", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // --- CARD 2: BANK VOLTAGE - ENLARGED HERO ---
    int c2X = 404;
    if (bData.communicationOK) {
        snprintf(valBuf, sizeof(valBuf), "%.2f V", bData.voltage_V);
        drawTextRow(c2X + 12, 76, 168, valBuf, COLOR_YELLOW, COLOR_CARD_BG, 4);

        float avgCell = (bData.voltage_V > 10.0f) ? (bData.voltage_V / 16.0f) : 0.0f;
        fillRect(c2X + 12, 108, 166, 20, COLOR_DARK_GRAY);
        drawRect(c2X + 12, 108, 166, 20, COLOR_MID_GRAY);
        snprintf(subBuf, sizeof(subBuf), "Avg Celle: %.3f V", avgCell);
        drawString(c2X + 18, 114, subBuf, COLOR_CYAN, COLOR_DARK_GRAY, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Ladesp. : %.2f V", bData.chargeVoltageLimit_V);
        drawTextRow(c2X + 10, 136, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Aflad Cut-off: %.2f V",
                 bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f);
        drawTextRow(c2X + 10, 154, cardW - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        drawTextRow(c2X + 10, 172, cardW - 14, "Fælles Busbar: 51.2V 16S", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Celledelta dV: %.0f mV", bData.cellDelta_mV);
        drawTextRow(c2X + 10, 190, cardW - 14, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Spænding Min : %.3f V", bData.minCellVoltage_V);
        drawTextRow(c2X + 10, 208, cardW - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

        drawTextRow(c2X + 10, 228, cardW - 14, "BMS CAN Link : 500k OK", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(c2X + 12, 76, 168, "--.-- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
        fillRect(c2X + 12, 108, 166, 20, COLOR_DARK_GRAY);
        drawRect(c2X + 12, 108, 166, 20, COLOR_MID_GRAY);
        drawString(c2X + 18, 114, "Avg Celle: -.--- V", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);

        drawTextRow(c2X + 10, 136, cardW - 14, "Maks Ladesp. : 57.60 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c2X + 10, 154, cardW - 14, "Aflad Cut-off: 44.80 V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c2X + 10, 172, cardW - 14, "Fælles Busbar: 51.2V 16S", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c2X + 10, 190, cardW - 14, "Celledelta dV: -- mV", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c2X + 10, 208, cardW - 14, "Spænding Min : -.--- V", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c2X + 10, 228, cardW - 14, "Status: Afventer CAN...", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // --- CARD 3: TOTAL & PEAK CURRENT - ENLARGED HERO ---
    int c3X = 602;
    int c3W = 190;
    static float s_peak_chg_A = 0.0f;
    static float s_peak_dchg_A = 0.0f;
    if (bData.communicationOK) {
        if (bData.current_A > s_peak_chg_A) s_peak_chg_A = bData.current_A;
        if (bData.current_A < s_peak_dchg_A) s_peak_dchg_A = bData.current_A;

        uint16_t curColor = (bData.current_A > 0.5f) ? COLOR_GREEN :
                            ((bData.current_A < -0.5f) ? COLOR_ORANGE : COLOR_WHITE);

        // Always clean hero current bounding box to guarantee no digit ghosting
        fillRect(c3X + 10, 74, 172, 32, COLOR_CARD_BG);

        // Format Total Current: if >= 100A or <= -100A, use Size 3 to comfortably fit inside card width!
        snprintf(valBuf, sizeof(valBuf), "%+.1f A", bData.current_A);
        if (fabsf(bData.current_A) >= 100.0f) {
            // Size 3: 8 chars * 18px = 144px, centered in 170px width
            drawTextRow(c3X + 14, 78, 164, valBuf, curColor, COLOR_CARD_BG, 3);
        } else {
            // Size 4: 7 chars * 24px = 168px
            drawTextRow(c3X + 11, 76, 168, valBuf, curColor, COLOR_CARD_BG, 4);
        }

        // Peak Current badge: 166x20 box
        fillRect(c3X + 12, 108, 166, 20, 0x18C3);
        drawRect(c3X + 12, 108, 166, 20, COLOR_CYAN);
        float dchgPeak = (s_peak_dchg_A < -0.1f) ? s_peak_dchg_A : 0.0f;
        snprintf(subBuf, sizeof(subBuf), "Peak: +%.0fA / %.0fA", s_peak_chg_A, dchgPeak);
        drawTextRow(c3X + 16, 114, 158, subBuf, COLOR_YELLOW, 0x18C3, 1);

        snprintf(subBuf, sizeof(subBuf), "Rosen Strøm : %+.1f A", bData.pack1_current_A);
        drawTextRow(c3X + 10, 136, c3W - 14, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "RPT   Strøm : %+.1f A", bData.pack2_current_A);
        drawTextRow(c3X + 10, 154, c3W - 14, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Batteri Temp: %.1f C", bData.temperature_C);
        drawTextRow(c3X + 10, 172, c3W - 14, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Ladestr: %.0f A", bData.chargeCurrentLimit_A);
        drawTextRow(c3X + 10, 190, c3W - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Aflad  : %.0f A", bData.dischargeCurrentLimit_A);
        drawTextRow(c3X + 10, 208, c3W - 14, subBuf, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);

        drawTextRow(c3X + 10, 228, c3W - 14, "Fordeling: 40% / 60% OK", COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        fillRect(c3X + 10, 74, 172, 32, COLOR_CARD_BG);
        drawTextRow(c3X + 10, 76, 170, "---.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 4);
        fillRect(c3X + 12, 108, 166, 20, COLOR_DARK_GRAY);
        drawRect(c3X + 12, 108, 166, 20, COLOR_MID_GRAY);
        drawTextRow(c3X + 16, 114, 158, "Peak: +0A / 0A", COLOR_LIGHT_GRAY, COLOR_DARK_GRAY, 1);

        drawTextRow(c3X + 10, 136, c3W - 14, "Rosen Strøm : --.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c3X + 10, 154, c3W - 14, "RPT   Strøm : --.- A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c3X + 10, 172, c3W - 14, "Batteri Temp: --.- C", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c3X + 10, 190, c3W - 14, "Maks Ladestr: 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c3X + 10, 208, c3W - 14, "Maks Aflad  : 390 A", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        drawTextRow(c3X + 10, 228, c3W - 14, "Status: Afventer CAN...", COLOR_ORANGE, COLOR_CARD_BG, 1);
    }

    // 3. Lower Section: Compacted Detail Panels In-Place Updates (midY = 274, midH = 150)
    const int midY = 274;
    int p1X = 8;
    int p1W = 388;
    int p2X = 404;
    int p2W = 388;

    if (bData.communicationOK) {
        // --- LEFT PANEL DYNAMIC UPDATES ---
        // Explicitly clear value area in each metric box to eliminate previous digit remnants
        fillRect(p1X + 10, midY + 38, 112, 20, COLOR_DARK_GRAY);
        snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.minCellVoltage_V);
        drawTextRow(p1X + 14, midY + 39, 104, subBuf, COLOR_CYAN, COLOR_DARK_GRAY, 2);

        fillRect(p1X + 132, midY + 38, 112, 20, COLOR_DARK_GRAY);
        snprintf(subBuf, sizeof(subBuf), "%.3f V", bData.maxCellVoltage_V);
        drawTextRow(p1X + 136, midY + 39, 104, subBuf, COLOR_YELLOW, COLOR_DARK_GRAY, 2);

        // Delta Hero Box
        fillRect(p1X + 254, midY + 38, 122, 20, COLOR_DARK_GRAY);
        snprintf(subBuf, sizeof(subBuf), "%.0f mV", bData.cellDelta_mV);
        uint16_t deltaColor = (bData.cellDelta_mV < 20.0f) ? COLOR_GREEN :
                              ((bData.cellDelta_mV < 50.0f) ? COLOR_YELLOW : COLOR_ORANGE);
        drawTextRow(p1X + 258, midY + 39, 112, subBuf, deltaColor, COLOR_DARK_GRAY, 2);

        // Graphical Spread Gauge Fill (3.00V to 3.65V)
        int gaugeX = p1X + 10;
        int gaugeY = midY + 66;
        int gaugeW = 368;
        int gaugeH = 8;
        fillRect(gaugeX + 1, gaugeY + 1, gaugeW - 2, gaugeH - 2, COLOR_DARK_GRAY);

        float minV = bData.minCellVoltage_V > 2.8f ? bData.minCellVoltage_V : 3.0f;
        float maxV = bData.maxCellVoltage_V > 2.8f ? bData.maxCellVoltage_V : 3.0f;
        int minPx = gaugeX + (int)(((minV - 3.0f) / 0.65f) * gaugeW);
        int maxPx = gaugeX + (int)(((maxV - 3.0f) / 0.65f) * gaugeW);
        if (minPx < gaugeX + 2) minPx = gaugeX + 2;
        if (maxPx > gaugeX + gaugeW - 4) maxPx = gaugeX + gaugeW - 4;
        if (maxPx < minPx + 4) maxPx = minPx + 4;
        fillRect(minPx, gaugeY + 1, maxPx - minPx, gaugeH - 2, COLOR_CYAN);
        // Needles stay strictly inside gauge interior to prevent un-erased streaks
        fillRect(minPx - 1, gaugeY + 1, 3, gaugeH - 2, COLOR_YELLOW);
        fillRect(maxPx - 1, gaugeY + 1, 3, gaugeH - 2, COLOR_GREEN);

        // Compact details in-place
        snprintf(subBuf, sizeof(subBuf), "Celle Temp  :  Min %.1f C  /  Max %.1f C",
                 bData.minCellTemp_C, bData.maxCellTemp_C);
        drawTextRow(p1X + 10, midY + 88, p1W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "BMS Temp/SOH:  %.1f C   (Sundhed: %u%% SOH)",
                 bData.temperature_C, bData.soh_percent);
        drawTextRow(p1X + 10, midY + 104, p1W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        float estKwh = (bData.totalCapacity_Ah * bData.voltage_V * (bData.soc_percent / 100.0f)) / 1000.0f;
        snprintf(subBuf, sizeof(subBuf), "Est. Energi :  ~%.1f kWh af 25.6 kWh", estKwh);
        drawTextRow(p1X + 10, midY + 120, p1W - 20, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);

        // --- RIGHT PANEL DYNAMIC UPDATES ---
        // Operational Switches Badges side-by-side at midY + 26 = 300, H = 22
        if (bData.chargeAllowed) {
            fillRect(p2X + 8, midY + 26, 116, 22, COLOR_DARK_GREEN);
            drawRect(p2X + 8, midY + 26, 116, 22, COLOR_GREEN);
            drawString(p2X + 16, midY + 32, "LAD: TILLADT", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 8, midY + 26, 116, 22, COLOR_RED);
            drawRect(p2X + 8, midY + 26, 116, 22, COLOR_WHITE);
            drawString(p2X + 16, midY + 32, "LAD: STOPPET", COLOR_WHITE, COLOR_RED, 1);
        }

        if (bData.dischargeAllowed) {
            fillRect(p2X + 130, midY + 26, 116, 22, COLOR_DARK_GREEN);
            drawRect(p2X + 130, midY + 26, 116, 22, COLOR_GREEN);
            drawString(p2X + 134, midY + 32, "AFLAD: TILLADT", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 130, midY + 26, 116, 22, COLOR_RED);
            drawRect(p2X + 130, midY + 26, 116, 22, COLOR_WHITE);
            drawString(p2X + 134, midY + 32, "AFLAD: STOPPET", COLOR_WHITE, COLOR_RED, 1);
        }

        if (!bData.protectionActive && !bData.warningActive) {
            fillRect(p2X + 252, midY + 26, 126, 22, COLOR_DARK_GREEN);
            drawRect(p2X + 252, midY + 26, 126, 22, COLOR_GREEN);
            drawString(p2X + 260, midY + 32, "ALARM: NORMAL", COLOR_WHITE, COLOR_DARK_GREEN, 1);
        } else {
            fillRect(p2X + 252, midY + 26, 126, 22, COLOR_RED);
            drawRect(p2X + 252, midY + 26, 126, 22, COLOR_WHITE);
            drawString(p2X + 260, midY + 32, "ALARM: AKTIV", COLOR_WHITE, COLOR_RED, 1);
        }

        snprintf(subBuf, sizeof(subBuf), "Ladespænding  :  %.2f V   (Cut-off: %.2f V)",
                 bData.chargeVoltageLimit_V,
                 bData.dischargeCutoffVoltage_V > 0 ? bData.dischargeCutoffVoltage_V : 44.8f);
        drawTextRow(p2X + 10, midY + 54, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Ladestrøm:  %.1f A   (~%.0f kW)",
                 bData.chargeCurrentLimit_A, (bData.chargeCurrentLimit_A * 54.0f) / 1000.0f);
        drawTextRow(p2X + 10, midY + 70, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Maks Afladning:  %.1f A   (~%.0f kW)",
                 bData.dischargeCurrentLimit_A, (bData.dischargeCurrentLimit_A * 51.0f) / 1000.0f);
        drawTextRow(p2X + 10, midY + 86, p2W - 20, subBuf, COLOR_WHITE, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "Rosen (Master):  %u%% SOC | %+.1f A | %+.2f kW",
                 bData.pack1_soc_percent, bData.pack1_current_A, bData.pack1_power_W / 1000.0f);
        drawTextRow(p2X + 10, midY + 104, p2W - 20, subBuf, COLOR_CYAN, COLOR_CARD_BG, 1);

        snprintf(subBuf, sizeof(subBuf), "RPT   (Slave) :  %u%% SOC | %+.1f A | %+.2f kW",
                 bData.pack2_soc_percent, bData.pack2_current_A, bData.pack2_power_W / 1000.0f);
        drawTextRow(p2X + 10, midY + 120, p2W - 20, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
    }
}

// State tracking for Page 2 and Page 3 flicker elimination
static bool s_diag_needs_full_redraw = true;
static bool s_scanner_needs_clear = true;

// -----------------------------------------------------------------------------
// Phase 2: Page 2 - Cell Balance & Voltage Diagnostics (Static Layout)
// Drawn ONCE when entering Cell Diagnostics view to eliminate PSRAM bus saturation
// -----------------------------------------------------------------------------
void UIManager::drawStaticCellDiagnostics() {
    s_diag_needs_full_redraw = true;

    // 1. Top Header Bar (Y: 0 to 44)
    fillRect(0, 0, LCD_WIDTH, 44, COLOR_NAVY);
    drawFastHLine(0, 44, LCD_WIDTH, COLOR_CYAN);
    drawString(15, 6, "CELL VOLTAGE DIAGNOSTICS (32 CELLS)", COLOR_WHITE, COLOR_NAVY, 2);

    int pW = 784, pX = 8;

    // --- PANEL 1: BATTERI 1: ROSEN MASTER (Y: 48 to 232, H: 184) ---
    int p1Y = 48, p1H = 184;
    fillRect(pX, p1Y, pW, p1H, COLOR_CARD_BG);
    drawRect(pX, p1Y, pW, p1H, COLOR_CARD_BORDER);

    // Header Strip (H: 22)
    fillRect(pX, p1Y, pW, 22, COLOR_DARK_BLUE);
    drawString(pX + 10, p1Y + 6, "BATTERI 1: ROSEN MASTER (51.2V 200Ah) - 16 CELLS", COLOR_CYAN, COLOR_DARK_BLUE, 1);

    // Pack 1 Chart Baseline & Static Cell Labels
    int base1Y = p1Y + 155;
    drawFastHLine(pX + 10, base1Y, pW - 20, COLOR_MID_GRAY);

    for (int i = 0; i < 16; i++) {
        int barX = pX + 16 + i * 47;
        char cellLbl[6];
        snprintf(cellLbl, sizeof(cellLbl), "C%02d", i + 1);
        drawString(barX + 5, base1Y + 5, cellLbl, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // --- PANEL 2: BATTERI 2: RPT SLAVE (Y: 238 to 422, H: 184) ---
    int p2Y = 238, p2H = 184;
    fillRect(pX, p2Y, pW, p2H, COLOR_CARD_BG);
    drawRect(pX, p2Y, pW, p2H, COLOR_CARD_BORDER);

    // Header Strip (H: 22)
    fillRect(pX, p2Y, pW, 22, COLOR_DARK_BLUE);
    drawString(pX + 10, p2Y + 6, "BATTERI 2: RPT SLAVE (51.2V 300Ah) - 16 CELLS", COLOR_GREEN, COLOR_DARK_BLUE, 1);

    // Pack 2 Chart Baseline & Static Cell Labels
    int base2Y = p2Y + 155;
    drawFastHLine(pX + 10, base2Y, pW - 20, COLOR_MID_GRAY);

    for (int i = 0; i < 16; i++) {
        int barX = pX + 16 + i * 47;
        char cellLbl[6];
        snprintf(cellLbl, sizeof(cellLbl), "C%02d", i + 1);
        drawString(barX + 5, base2Y + 5, cellLbl, COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // 3. Bottom Status & Navigation Bar (Tab 1 active)
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
        drawTextRow(450, 8, 160, "WiFi: Standby", COLOR_YELLOW, COLOR_NAVY, 1);
    }

    uint32_t upSec = millis() / 1000;
    char upBuf2[32];
    snprintf(upBuf2, sizeof(upBuf2), "UP: %02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);
    drawTextRow(620, 8, 175, upBuf2, COLOR_LIGHT_GRAY, COLOR_NAVY, 1);

    // LiPo Battery Status (Option A)
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

    char subBuf[128];
    int pW = 784, pX = 8;
    int chartH = 95;

    // --- PANEL 1: ROSEN MASTER ---
    int p1Y = 48;
    float p1Min = bData.pack1_minV > 2.0f ? bData.pack1_minV : (bData.minCellVoltage_V > 2.0f ? bData.minCellVoltage_V : 3.367f);
    float p1Max = bData.pack1_maxV > 2.0f ? bData.pack1_maxV : (bData.maxCellVoltage_V > 2.0f ? bData.maxCellVoltage_V : 3.378f);

    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "SOC: %u%%  Min: %.3fV  Max: %.3fV  dV: %.0fmV",
                 bData.pack1_soc_percent, p1Min, p1Max, (p1Max - p1Min) * 1000.0f);
    } else {
        snprintf(subBuf, sizeof(subBuf), "Afventer CAN telemetri...");
    }
    drawTextRow(pX + 440, p1Y + 6, 335, subBuf, COLOR_WHITE, COLOR_DARK_BLUE, 1);

    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "SOC: %u%%   Strøm: ~%+.1f A (Est. 40%%)   Effekt: %+.2f kW   Energi: ~%.1f kWh / 10.2 kWh",
                 bData.pack1_soc_percent, bData.pack1_current_A, bData.pack1_power_W / 1000.0f,
                 bData.pack1_energy_kwh);
        drawTextRow(pX + 15, p1Y + 26, pW - 30, subBuf, COLOR_YELLOW, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(pX + 15, p1Y + 26, pW - 30, "SOC: --%   Strøm: ~--.- A (Est. 40%)   Effekt: --- kW   Energi: ~--.- kWh   Afventer CAN telemetri...", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    // Cache cell bar values to completely eliminate repetitive erasing and flickering
    static int s_p1_last_mv[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    static uint16_t s_p1_last_col[16] = { 0 };
    static int s_p2_last_mv[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    static uint16_t s_p2_last_col[16] = { 0 };

    bool forceRedraw = s_diag_needs_full_redraw;
    s_diag_needs_full_redraw = false;

    int base1Y = p1Y + 155;
    for (int i = 0; i < 16; i++) {
        int barX = pX + 16 + i * 47;
        int barW = 34;
        float v = bData.pack1_cellVoltages[i];
        if (v < 2.0f) v = p1Min;

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

        int curMv = (int)(v * 1000.0f + 0.5f);
        if (!forceRedraw && curMv == s_p1_last_mv[i] && bColor == s_p1_last_col[i]) {
            continue; // Unchanged: skip redrawing to eliminate PSRAM scanout flicker!
        }
        s_p1_last_mv[i] = curMv;
        s_p1_last_col[i] = bColor;

        if (bData.communicationOK) {
            fillRect(barX - 2, base1Y - chartH - 14, barW + 4, chartH + 14, COLOR_CARD_BG);
            fillRect(barX, barY, barW, bH, bColor);
            drawRect(barX, barY, barW, bH, COLOR_WHITE);

            char mvStr[8];
            snprintf(mvStr, sizeof(mvStr), "%d", curMv);
            drawTextRow(barX + 5, barY - 10, barW - 6, mvStr, COLOR_WHITE, COLOR_CARD_BG, 1);
        } else {
            fillRect(barX - 2, base1Y - chartH - 14, barW + 4, chartH + 14, COLOR_CARD_BG);
            drawRect(barX, base1Y - 15, barW, 15, COLOR_MID_GRAY);
            drawTextRow(barX + 5, base1Y - 25, barW - 6, "---", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }
    }

    // --- PANEL 2: RPT SLAVE ---
    int p2Y = 238;
    float p2Min = bData.pack2_minV > 2.0f ? bData.pack2_minV : (bData.minCellVoltage_V > 2.0f ? bData.minCellVoltage_V : 3.369f);
    float p2Max = bData.pack2_maxV > 2.0f ? bData.pack2_maxV : (bData.maxCellVoltage_V > 2.0f ? bData.maxCellVoltage_V : 3.380f);

    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "SOC: %u%%  Min: %.3fV  Max: %.3fV  dV: %.0fmV",
                 bData.pack2_soc_percent, p2Min, p2Max, (p2Max - p2Min) * 1000.0f);
    } else {
        snprintf(subBuf, sizeof(subBuf), "Afventer CAN telemetri...");
    }
    drawTextRow(pX + 440, p2Y + 6, 335, subBuf, COLOR_WHITE, COLOR_DARK_BLUE, 1);

    if (bData.communicationOK) {
        snprintf(subBuf, sizeof(subBuf), "SOC: %u%%   Strøm: ~%+.1f A (Est. 60%%)   Effekt: %+.2f kW   Energi: ~%.1f kWh / 15.4 kWh",
                 bData.pack2_soc_percent, bData.pack2_current_A, bData.pack2_power_W / 1000.0f,
                 bData.pack2_energy_kwh);
        drawTextRow(pX + 15, p2Y + 26, pW - 30, subBuf, COLOR_GREEN, COLOR_CARD_BG, 1);
    } else {
        drawTextRow(pX + 15, p2Y + 26, pW - 30, "SOC: --%   Strøm: ~--.- A (Est. 60%)   Effekt: --- kW   Energi: ~--.- kWh   Afventer CAN telemetri...", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
    }

    int base2Y = p2Y + 155;
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

        int curMv = (int)(v * 1000.0f + 0.5f);
        if (!forceRedraw && curMv == s_p2_last_mv[i] && bColor == s_p2_last_col[i]) {
            continue; // Unchanged: skip redrawing to eliminate PSRAM scanout flicker!
        }
        s_p2_last_mv[i] = curMv;
        s_p2_last_col[i] = bColor;

        if (bData.communicationOK) {
            fillRect(barX - 2, base2Y - chartH - 14, barW + 4, chartH + 14, COLOR_CARD_BG);
            fillRect(barX, barY, barW, bH, bColor);
            drawRect(barX, barY, barW, bH, COLOR_WHITE);

            char mvStr[8];
            snprintf(mvStr, sizeof(mvStr), "%d", curMv);
            drawTextRow(barX + 5, barY - 10, barW - 6, mvStr, COLOR_WHITE, COLOR_CARD_BG, 1);
        } else {
            fillRect(barX - 2, base2Y - chartH - 14, barW + 4, chartH + 14, COLOR_CARD_BG);
            drawRect(barX, base2Y - 15, barW, 15, COLOR_MID_GRAY);
            drawTextRow(barX + 5, base2Y - 25, barW - 6, "---", COLOR_LIGHT_GRAY, COLOR_CARD_BG, 1);
        }
    }
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
    drawString(p2X + 8, infoY, "* 0x351: Spændings- og strømgrænser.", COLOR_GREEN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x355: Total SOC % og Sundhed (SOH).", COLOR_GREEN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x356: Total Spænding, Strøm og Temp.", COLOR_GREEN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x359: BMS Beskyttelse & Advarsler.", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x35C: Charge / Discharge anmodning.", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x35E: Fabrikant-streng (PYLON).", COLOR_CYAN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x373: Min/Max cellespænding & temp.", COLOR_YELLOW, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* 0x379: Nominel kapacitet (Ah).", COLOR_YELLOW, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* PROTOKOL: Deye / Pylontech (500 kbit/s).", COLOR_GREEN, COLOR_CARD_BG, 1); infoY += 14;
    drawString(p2X + 8, infoY, "* SD KORT: Logger alle rammer til CSV-fil.", COLOR_YELLOW, COLOR_CARD_BG, 1);

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
        drawTextRow(325, 8, 140, "WiFi: Standby", COLOR_YELLOW, COLOR_NAVY, 1);
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

    // Static layout is rendered ONLY when switching to a new view page
    if (_view_mode != _last_drawn_mode) {
        _last_drawn_mode = _view_mode;
        fillScreen(COLOR_BLACK);
        if (_view_mode == UI_VIEW_DASHBOARD) {
            drawStaticDashboard();
        } else if (_view_mode == UI_VIEW_CELL_DIAGNOSTICS) {
            drawStaticCellDiagnostics();
        } else {
            drawStaticScanner();
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
    } else {
        updateDynamicScanner(overview, bData);
    }

    // Update display: if using direct scanout framebuffer, flush CPU dirty cache lines to PSRAM.
    // GDMA continuously streams directly from this PSRAM buffer, completely eliminating 768KB memcpy bus contention!
    if (_is_direct_fb) {
        Cache_WriteBack_All();
    } else {
        esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);
    }
}

