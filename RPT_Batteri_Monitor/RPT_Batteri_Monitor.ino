// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// VERSION : v1.5.0 (Rock-Solid 10MHz Display Timing, Zero-Flicker & Cache Fix)
// DATO/TID: 2026-09-06 11:34:00
// =============================================================================

#include <Arduino.h>
#include "board_config.h"
#include "wifi_config.h"
#include "battery_data.h"
#include "can_receiver.h"
#include "sd_logger.h"
#include "ui.h"
#include "deye_bms_decoder.h"
#include "web_server.h"
#include "board_battery.h"

// Tracking variables for periodic serial output
static uint32_t last_serial_stats_ms = 0;
static uint32_t last_watchdog_ms = 0;
static uint32_t last_heartbeat_ms = 0;

void printStartupBanner() {
    LOG_PRINTLN("\n================================================================================");
    LOG_PRINTLN("   RPT & ROSEN BATTERY STORAGE DASHBOARD - PHASE 2");
    LOG_PRINTLN("   Target Hardware: Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)");
    LOG_PRINTLN("   Screen: 7.0 inch IPS RGB (800x480) - ST7262 Driver");
    LOG_PRINTLN("   Firmware Date  : 2026-09-06 11:34:00 (v1.5.0 Rock-Solid Display)");
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
    LOG_PRINTF("   LiPo ADC       : GPIO %d (Sensor AD Pin 2 to TP1, Ratio: %.2fx)\n",
                  (int)BOARD_LIPO_ADC_PIN, BOARD_LIPO_DIVIDER_RATIO);
    LOG_PRINTLN("   Multiplexer    : CH422G EXIO5 = HIGH (CAN_SEL routed to TJA1051)");
    LOG_PRINTLN("   Serial Port    : USB-C port UART1 (CH343) at 115200 baud");
    LOG_PRINTLN("================================================================================\n");
}

void printSerialIdStatistics() {
    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    CanIdStats stats[MAX_TRACKED_CAN_IDS];
    size_t count = CanReceiver::getInstance().getIdStatistics(stats, MAX_TRACKED_CAN_IDS);

    LOG_PRINTLN("\n--- [BATTERY TELEMETRY & CAN REPORT] -----------------------------------------");
    LOG_PRINTF("Total Frames: %lu | Rate: %.1f fps | Active IDs: %u | Bus Errors: %lu | SD: %s\n",
                  (unsigned long)overview.total_packets,
                  overview.packets_per_sec,
                  (unsigned int)overview.active_ids_count,
                  (unsigned long)overview.bus_error_count,
                  overview.sd_card_mounted ? overview.sd_filename : "No SD card");

    BatteryData bData;
    if (DeyeBmsDecoder::getInstance().getBatteryData(bData)) {
        LOG_PRINTF(">>> BATTERY STATUS: SOC: %u%% | SOH: %u%% | Volt: %.2fV | Curr: %+.1fA | Power: %+.2fkW | Temp: %.1fC\n",
                   bData.soc_percent, bData.soh_percent, bData.voltage_V, bData.current_A, bData.power_W / 1000.0f, bData.temperature_C);
        LOG_PRINTF(">>> DUAL-PACK: Total(Målt): %+.1fA | Rosen(Est.40%%): %+.1fA (%+.2fkW) | RPT(Est.60%%): %+.1fA (%+.2fkW)\n",
                   bData.current_A,
                   bData.pack1_current_A, bData.pack1_power_W / 1000.0f,
                   bData.pack2_current_A, bData.pack2_power_W / 1000.0f);
        LOG_PRINTF("    Cells: Min %.3fV, Max %.3fV (dV: %.0fmV) | Capacity: %uAh | Modules: %u\n",
                   bData.minCellVoltage_V, bData.maxCellVoltage_V, bData.cellDelta_mV, bData.totalCapacity_Ah, bData.moduleCount);
        LOG_PRINTF("    Limits: MaxChg %.1fA, MaxDchg %.1fA, ChgV %.2fV, Cutoff %.2fV\n",
                   bData.chargeCurrentLimit_A, bData.dischargeCurrentLimit_A, bData.chargeVoltageLimit_V, bData.dischargeCutoffVoltage_V);
    }

    // Report ESP32 Onboard LiPo Battery (Option A via TP1 & GPIO 6)
    LipoBatteryStatus lipo = BoardBattery::getInstance().getStatus();
    if (lipo.connected) {
        LOG_PRINTF(">>> ESP32 LIPO BATTERY: %.1fV (%u%%) [Option A: TP1 -> GPIO %d, Ratio: %.2fx]\n",
                   lipo.voltage_V, lipo.soc_percent, (int)BOARD_LIPO_ADC_PIN, BOARD_LIPO_DIVIDER_RATIO);
    } else {
        LOG_PRINTLN(">>> ESP32 LIPO BATTERY: N/A (Disconnected or < 2.0V)");
    }

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
    LOG_PRINTF( ">>> ESP32-S3 RPT BATTERY MONITOR - COMPILED: %s %s (v1.5.0) <<<\n", __DATE__, __TIME__);
    printStartupBanner();

    // 1. Initialize Board LiPo Battery ADC (Option A via TP1 & GPIO 6)
    LOG_PRINTLN("[BOOT 1/6] Initializing Board LiPo Battery ADC (Option A: GPIO 6)...");
    BoardBattery::getInstance().begin();

    // 2. Initialize Protocol Decoder
    LOG_PRINTLN("[BOOT 2/6] Initializing Protocol Decoder...");
    DeyeBmsDecoder::getInstance().begin();

    // 3. Initialize WiFi FIRST (Fast 1-2 sec connection before display starts!)
    // By connecting WiFi before screen initialization, the ST7262 never experiences
    // RF power bursts, channel hopping, or Flash NVS locks during display startup.
    LOG_PRINTLN("[BOOT 3/6] Initializing WiFi & Web Server Subsystem (Fast-Scan)...");
    BatteryWebServer::getInstance().begin();

    // 4. Initialize UI & Board Hardware
    // Sets up Wire on SDA=8/SCL=9, pulses LCD_RST, turns on Backlight, and starts the ST7262 display.
    LOG_PRINTLN("[BOOT 4/6] Initializing Board Hardware, I2C, and ST7262 RGB Display...");
    if (!UIManager::getInstance().begin()) {
        LOG_PRINTLN("[BOOT WARNING] UI / Display initialization reported an issue.");
    } else {
        LOG_PRINTLN("[BOOT OK] Display initialized successfully!");
    }

    // 5. Initialize MicroSD Card Logger (after I2C/CH422G is ready)
    LOG_PRINTLN("[BOOT 5/6] Initializing MicroSD Logger...");
    SdLogger::getInstance().begin();

    // 6. Initialize TWAI CAN Receiver (500 kbit/s, Point-to-Point / Hardware ACK)
    LOG_PRINTLN("[BOOT 6/6] Activating CAN Mode (CH422G EXIO5 = HIGH) & TWAI Receiver...");
    UIManager::getInstance().setCanMode(true);
    delay(50);
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

    // Drain up to 25 frames non-blocking (0 wait) per cycle to prevent loop starvation and watchdog resets
    uint8_t processedFrames = 0;
    while (processedFrames < 25 && CanReceiver::getInstance().getNextFrame(frame, 0)) {
        processedFrames++;
        // 1. Write to MicroSD card CSV file
        SdLogger::getInstance().logFrame(frame);

        // 2. Pass to decoder hook
        DeyeBmsDecoder::getInstance().decodeFrame(frame);

        // 3. Print individual frame to Serial Monitor in real-time (non-blocking)
        if (Serial.availableForWrite() >= 96) {
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
    }

    uint32_t now = millis();

    // Heartbeat every 2 seconds to prove board is alive and report physical bus state
    if (now - last_heartbeat_ms >= 2000) {
        ScannerOverview ov;
        CanReceiver::getInstance().getOverview(ov);
        const char* stateStr = "RUNNING";
        if (ov.twai_state == 0) stateStr = "STOPPED";
        else if (ov.twai_state == 2) stateStr = "BUS_OFF";
        else if (ov.twai_state == 3) stateStr = "RECOVERING";

        LOG_PRINTF("[HEARTBEAT %lus] State: %s | Frames: %lu | Bus Errors: %lu | REC: %lu | TEC: %lu\n",
                   (unsigned long)(now / 1000),
                   stateStr,
                   (unsigned long)ov.total_packets,
                   (unsigned long)ov.bus_error_count,
                   (unsigned long)ov.rx_error_counter,
                   (unsigned long)ov.tx_error_counter);
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

    // Handle Web Server client requests & background WiFi reconnect
    BatteryWebServer::getInstance().loop();

    // Sample Board LiPo Battery (rate-limited internally to 500ms)
    BoardBattery::getInstance().update();

    // Yield to FreeRTOS scheduler
    vTaskDelay(pdMS_TO_TICKS(5));
}
