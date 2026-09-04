// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// VERSION : v1.0.5 (ST7262 LCD Reset & Dual Serial Output Fix)
// DATO/TID: 2026-09-04 16:15:00
// =============================================================================

#include <Arduino.h>
#include "board_config.h"
#include "battery_data.h"
#include "can_receiver.h"
#include "sd_logger.h"
#include "ui.h"
#include "deye_bms_decoder.h"

// Tracking variables for periodic serial output
static uint32_t last_serial_stats_ms = 0;
static uint32_t last_watchdog_ms = 0;
static uint32_t last_heartbeat_ms = 0;

void printStartupBanner() {
    LOG_PRINTLN("\n================================================================================");
    LOG_PRINTLN("   RPT BATTERY CAN-BUS PASSIVE SCANNER - PHASE 1");
    LOG_PRINTLN("   Target Hardware: Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)");
    LOG_PRINTLN("   Screen: 7.0 inch IPS RGB (800x480) - ST7262 Driver");
    LOG_PRINTLN("   Firmware Date  : 2026-09-04 16:15:00 (v1.0.5 ST7262/Dual-Serial)");
    LOG_PRINTF( "   Compile Time   : %s %s\n", __DATE__, __TIME__);
    LOG_PRINTLN("================================================================================");
    LOG_PRINTF("   CAN Controller : ESP32-S3 TWAI (TX: GPIO%d, RX: GPIO%d)\n",
                  BOARD_CAN_TX_PIN, BOARD_CAN_RX_PIN);
#if BOARD_CAN_POINT_TO_POINT
    LOG_PRINTLN("   CAN Mode       : POINT-TO-POINT (Hardware ACK active, Zero Software TX)");
    LOG_PRINTLN("   Termination    : Jumper 13 MUST BE INSTALLED/ON (120 Ohm bus termination)");
#else
    LOG_PRINTLN("   CAN Mode       : LISTEN-ONLY (Passive, No Transmit, No ACK)");
    LOG_PRINTLN("   Termination    : Jumper 13 MUST BE REMOVED/OFF (External bus already terminated)");
#endif
    LOG_PRINTF("   Baud Rate      : %d kbit/s\n", BOARD_CAN_DEFAULT_BAUDRATE / 1000);
    LOG_PRINTLN("   Multiplexer    : CH422G EXIO5 = HIGH (CAN_SEL routed to TJA1051)");
    LOG_PRINTLN("   Serial Port    : USB-C port UART1 (CH343) at 115200 baud");
    LOG_PRINTLN("================================================================================\n");
}

void printSerialIdStatistics() {
    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    CanIdStats stats[MAX_TRACKED_CAN_IDS];
    size_t count = CanReceiver::getInstance().getIdStatistics(stats, MAX_TRACKED_CAN_IDS);

    LOG_PRINTLN("\n--- [CAN SCANNER PERIODIC REPORT] ---------------------------------------------");
    LOG_PRINTF("Total Frames: %lu | Rate: %.1f fps | Active IDs: %u | Bus Errors: %lu | SD: %s\n",
                  (unsigned long)overview.total_packets,
                  overview.packets_per_sec,
                  (unsigned int)overview.active_ids_count,
                  (unsigned long)overview.bus_error_count,
                  overview.sd_card_mounted ? overview.sd_filename : "No SD card");
    LOG_PRINTLN("--------------------------------------------------------------------------------");
    LOG_PRINTLN("CAN-ID   TYPE  DLC   COUNT    INTERVAL   LAST PAYLOAD (HEX)");
    LOG_PRINTLN("--------------------------------------------------------------------------------");

    if (count == 0) {
        LOG_PRINTLN("  (No CAN traffic observed yet. Check wiring: RJ45 Pin 4=CAN-H, Pin 5=CAN-L)");
    } else {
        for (size_t i = 0; i < count; i++) {
            const CanIdStats& s = stats[i];
            char payloadStr[32] = "";
            for (int b = 0; b < s.dlc && b < 8; b++) {
                char hex[5];
                snprintf(hex, sizeof(hex), "%02X ", s.last_data[b]);
                strcat(payloadStr, hex);
            }

            LOG_PRINTF("0x%03X%s  %s   %u   %6lu   %5lums    %s\n",
                          (unsigned int)s.id,
                          (s.id < 0x100 ? " " : ""),
                          s.extended ? "EXT" : "STD",
                          s.dlc,
                          (unsigned long)s.count,
                          (unsigned long)s.interval_ms,
                          payloadStr);
        }
    }
    LOG_PRINTLN("--------------------------------------------------------------------------------\n");
}

void setup() {
    // 1. Initialize Serial Communication for debug & data logging
    Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
    Serial0.begin(115200); // Hardware UART0 on GPIO 43/44 (CH343 USB-to-UART bridge)
#endif

    delay(200);

    LOG_PRINTLN("\n\n================================================================================");
    LOG_PRINTLN(">>> ESP32-S3 RPT BATTERY MONITOR - TIMESTAMP: 2026-09-04 16:15:00 (v1.0.5) <<<");
    printStartupBanner();

    // 2. Initialize UI & Board Hardware
    // Sets up Wire on SDA=8/SCL=9, pulses LCD_RST, turns on Backlight, sets CH422G EXIO5=HIGH (CAN mode)
    // and starts the 7.0-inch 800x480 RGB display task.
    LOG_PRINTLN("[BOOT 1/4] Initializing Board Hardware, I2C, and ST7262 RGB Display...");
    if (!UIManager::getInstance().begin()) {
        LOG_PRINTLN("[BOOT WARNING] UI / Display initialization reported an issue.");
    } else {
        LOG_PRINTLN("[BOOT OK] Display initialized successfully!");
    }

    // 3. Initialize MicroSD Card Logger (if card is inserted)
    LOG_PRINTLN("[BOOT 2/4] Initializing MicroSD Logger...");
    SdLogger::getInstance().begin();

    // 4. Initialize Protocol Decoder
    LOG_PRINTLN("[BOOT 3/4] Initializing Protocol Decoder...");
    DeyeBmsDecoder::getInstance().begin();

    // 5. Initialize TWAI CAN Receiver (500 kbit/s, Point-to-Point / Hardware ACK)
    LOG_PRINTLN("[BOOT 4/4] Initializing CAN Receiver (Point-to-Point, 500 kbps)...");
    if (!CanReceiver::getInstance().begin(BOARD_CAN_DEFAULT_BAUDRATE)) {
        LOG_PRINTLN("[BOOT FATAL] Failed to start CAN Receiver! Check pins and TWAI driver.");
    } else {
        LOG_PRINTLN("[BOOT OK] CAN Receiver listening!");
    }

    LOG_PRINTLN("\n[SYSTEM READY] All subsystems started. Listening for RPT battery CAN frames.\n");
    last_serial_stats_ms = millis();
    last_watchdog_ms = millis();
    last_heartbeat_ms = millis();
}

void loop() {
    CanFrameRaw frame;

    // Drain incoming frames from the queue (non-blocking, wait up to 10ms)
    while (CanReceiver::getInstance().getNextFrame(frame, pdMS_TO_TICKS(10))) {
        // 1. Write to MicroSD card CSV file
        SdLogger::getInstance().logFrame(frame);

        // 2. Pass to decoder hook
        DeyeBmsDecoder::getInstance().decodeFrame(frame);

        // 3. Print individual frame to Serial Monitor in real-time
        char dataHex[32] = "";
        for (int i = 0; i < frame.dlc && i < 8; i++) {
            char b[5];
            snprintf(b, sizeof(b), "%02X ", frame.data[i]);
            strcat(dataHex, b);
        }

        ScannerOverview overview;
        CanReceiver::getInstance().getOverview(overview);

        LOG_PRINTF("[%7lums] ID: 0x%03X (%s) DLC: %d Data: %-24s [Total: %lu]\n",
                      (unsigned long)frame.timestamp_ms,
                      (unsigned int)frame.id,
                      frame.extended ? "EXT" : "STD",
                      frame.dlc,
                      dataHex,
                      (unsigned long)overview.total_packets);
    }

    uint32_t now = millis();

    // Heartbeat every 2 seconds to prove board is alive on Serial Monitor
    if (now - last_heartbeat_ms >= 2000) {
        ScannerOverview ov;
        CanReceiver::getInstance().getOverview(ov);
        LOG_PRINTF("[HEARTBEAT %lus] Total Frames: %lu | Rate: %.1ffps | Errors: %lu\n",
                   (unsigned long)(now / 1000),
                   (unsigned long)ov.total_packets,
                   ov.packets_per_sec,
                   (unsigned long)ov.bus_error_count);
        last_heartbeat_ms = now;
    }

    // Periodic statistics report to Serial every 5 seconds
    if (now - last_serial_stats_ms >= 5000) {
        printSerialIdStatistics();
        last_serial_stats_ms = now;
    }

    // Watchdog check for protocol decoder
    if (now - last_watchdog_ms >= 1000) {
        DeyeBmsDecoder::getInstance().checkWatchdog(5000);
        last_watchdog_ms = now;
    }

    // Yield to FreeRTOS scheduler
    vTaskDelay(pdMS_TO_TICKS(5));
}
