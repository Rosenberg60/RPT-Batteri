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

void printStartupBanner() {
    Serial.println("\n================================================================================");
    Serial.println("   RPT BATTERY CAN-BUS PASSIVE SCANNER - PHASE 1");
    Serial.println("   Target Hardware: Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)");
    Serial.println("   Screen: 7.0 inch IPS RGB (800x480)");
    Serial.println("================================================================================");
    Serial.printf("   CAN Controller : ESP32-S3 TWAI (TX: GPIO%d, RX: GPIO%d)\n",
                  BOARD_CAN_TX_PIN, BOARD_CAN_RX_PIN);
#if BOARD_CAN_POINT_TO_POINT
    Serial.println("   CAN Mode       : POINT-TO-POINT (Hardware ACK active, Zero Software TX)");
    Serial.println("   Termination    : Jumper 13 MUST BE INSTALLED/ON (120 Ohm bus termination)");
#else
    Serial.println("   CAN Mode       : LISTEN-ONLY (Passive, No Transmit, No ACK)");
    Serial.println("   Termination    : Jumper 13 MUST BE REMOVED/OFF (External bus already terminated)");
#endif
    Serial.printf("   Baud Rate      : %d kbit/s\n", BOARD_CAN_DEFAULT_BAUDRATE / 1000);
    Serial.println("   Multiplexer    : CH422G EXIO5 = HIGH (CAN_SEL routed to TJA1051)");
    Serial.println("   Power Supply   : 5V DC via Type-C or 5V Header (DO NOT CONNECT 51.2V DIRECTLY!)");
    Serial.println("================================================================================\n");
}

void printSerialIdStatistics() {
    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    CanIdStats stats[MAX_TRACKED_CAN_IDS];
    size_t count = CanReceiver::getInstance().getIdStatistics(stats, MAX_TRACKED_CAN_IDS);

    Serial.println("\n--- [CAN SCANNER PERIODIC REPORT] ---------------------------------------------");
    Serial.printf("Total Frames: %lu | Rate: %.1f fps | Active IDs: %u | Bus Errors: %lu | SD: %s\n",
                  (unsigned long)overview.total_packets,
                  overview.packets_per_sec,
                  (unsigned int)overview.active_ids_count,
                  (unsigned long)overview.bus_error_count,
                  overview.sd_card_mounted ? overview.sd_filename : "No SD card");
    Serial.println("--------------------------------------------------------------------------------");
    Serial.println("CAN-ID   TYPE  DLC   COUNT    INTERVAL   LAST PAYLOAD (HEX)");
    Serial.println("--------------------------------------------------------------------------------");

    if (count == 0) {
        Serial.println("  (No CAN traffic observed yet. Check wiring: RJ45 Pin 4=CAN-H, Pin 5=CAN-L)");
    } else {
        for (size_t i = 0; i < count; i++) {
            const CanIdStats& s = stats[i];
            char payloadStr[32] = "";
            for (int b = 0; b < s.dlc && b < 8; b++) {
                char hex[5];
                snprintf(hex, sizeof(hex), "%02X ", s.last_data[b]);
                strcat(payloadStr, hex);
            }

            Serial.printf("0x%03X%s  %s   %u   %6lu   %5lums    %s\n",
                          (unsigned int)s.id,
                          (s.id < 0x100 ? " " : ""),
                          s.extended ? "EXT" : "STD",
                          s.dlc,
                          (unsigned long)s.count,
                          (unsigned long)s.interval_ms,
                          payloadStr);
        }
    }
    Serial.println("--------------------------------------------------------------------------------\n");
}

void setup() {
    // 1. Initialize Serial Communication for debug & data logging
    Serial.begin(115200);

    // For ESP32-S3 Native USB (COM5): Wait up to 3 seconds for Serial Monitor to connect
    uint32_t startWait = millis();
    while (!Serial && (millis() - startWait < 3000)) {
        delay(10);
    }
    delay(200);

    Serial.println("\n\n================================================================================");
    Serial.println(">>> ESP32-S3 RPT BATTERY MONITOR BOOTED SUCCESSFULLY! <<<");
    printStartupBanner();

    // 2. Initialize UI & Board Hardware
    // This sets up I2C, turns on Backlight, sets CH422G EXIO5=HIGH (CAN mode)
    // and starts the 7.0-inch 800x480 RGB display task.
    Serial.println("[MAIN] Initializing Board Hardware, I2C, and Display...");
    if (!UIManager::getInstance().begin()) {
        Serial.println("[MAIN WARNING] UI / Display initialization reported an issue.");
    }

    // 3. Initialize MicroSD Card Logger (if card is inserted)
    Serial.println("[MAIN] Initializing MicroSD Logger...");
    SdLogger::getInstance().begin();

    // 4. Initialize Protocol Decoder
    Serial.println("[MAIN] Initializing Protocol Decoder...");
    DeyeBmsDecoder::getInstance().begin();

    // 5. Initialize TWAI CAN Receiver in Listen-Only mode (500 kbit/s)
    Serial.println("[MAIN] Initializing CAN Receiver (Listen-Only)...");
    if (!CanReceiver::getInstance().begin(BOARD_CAN_DEFAULT_BAUDRATE)) {
        Serial.println("[MAIN FATAL] Failed to start CAN Receiver! Check pins and TWAI driver.");
    }

    Serial.println("[MAIN] Setup complete. System listening on CAN bus.");
    last_serial_stats_ms = millis();
    last_watchdog_ms = millis();
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

        Serial.printf("[%7lums] ID: 0x%03X (%s) DLC: %d Data: %-24s [Total: %lu]\n",
                      (unsigned long)frame.timestamp_ms,
                      (unsigned int)frame.id,
                      frame.extended ? "EXT" : "STD",
                      frame.dlc,
                      dataHex,
                      (unsigned long)overview.total_packets);
    }

    uint32_t now = millis();

    // Periodic statistics report to Serial every 3 seconds
    if (now - last_serial_stats_ms >= 3000) {
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
