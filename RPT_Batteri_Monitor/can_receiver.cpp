#include "can_receiver.h"
#include "driver/twai.h"
#include "esp_log.h"

static const char* TAG = "CAN_RX";

CanReceiver& CanReceiver::getInstance() {
    static CanReceiver instance;
    return instance;
}

CanReceiver::CanReceiver()
    : _driver_installed(false),
      _frame_queue(nullptr),
      _mutex(nullptr),
      _id_count(0),
      _recent_head(0),
      _recent_total(0),
      _total_packets(0),
      _bus_error_count(0),
      _rx_missed_count(0),
      _last_packet_time_ms(0),
      _last_rate_calc_ms(0),
      _packets_since_last_calc(0),
      _current_rate_fps(0.0f)
{
    memset(_id_table, 0, sizeof(_id_table));
    memset(_recent_frames, 0, sizeof(_recent_frames));
}

CanReceiver::~CanReceiver() {
    stop();
}

bool CanReceiver::begin(uint32_t baudrate) {
    if (_driver_installed) {
        return true;
    }

    // Create FreeRTOS primitives if not already created
    if (!_mutex) {
        _mutex = xSemaphoreCreateMutex();
    }
    if (!_frame_queue) {
        _frame_queue = xQueueCreate(CAN_FRAME_QUEUE_SIZE, sizeof(CanFrameRaw));
    }

    LOG_PRINTF("[CAN] Initializing TWAI on TX=GPIO%d, RX=GPIO%d (Listen-Only)...\n",
                  BOARD_CAN_TX_PIN, BOARD_CAN_RX_PIN);

    // 1. General Configuration
#if BOARD_CAN_POINT_TO_POINT
    twai_mode_t twai_mode = TWAI_MODE_NORMAL;
    LOG_PRINTLN("[CAN] Point-to-Point mode: Hardware ACK active (prevents battery ACK errors). No software TX.");
#else
    twai_mode_t twai_mode = TWAI_MODE_LISTEN_ONLY;
    LOG_PRINTLN("[CAN] Listen-Only mode: Completely passive (no ACK).");
#endif

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        BOARD_CAN_TX_PIN,
        BOARD_CAN_RX_PIN,
        twai_mode
    );
    g_config.rx_queue_len = 64; // Generous hardware RX queue to absorb bursts

    // 2. Timing Configuration: 500 kbit/s (or custom if specified)
    twai_timing_config_t t_config;
    if (baudrate == 250000) {
        t_config = TWAI_TIMING_CONFIG_250KBITS();
    } else if (baudrate == 1000000) {
        t_config = TWAI_TIMING_CONFIG_1MBITS();
    } else {
        t_config = TWAI_TIMING_CONFIG_500KBITS(); // Default for Deye & LiFePO4
    }

    // 3. Filter Configuration: Accept ALL standard (11-bit) and extended (29-bit) frames
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        LOG_PRINTF("[CAN ERROR] twai_driver_install failed (0x%X)\n", err);
        return false;
    }

    // Start TWAI driver
    err = twai_start();
    if (err != ESP_OK) {
        LOG_PRINTF("[CAN ERROR] twai_start failed (0x%X)\n", err);
        twai_driver_uninstall();
        return false;
    }

    // Reconfigure alerts to detect incoming data and bus errors
    uint32_t alerts = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS |
                      TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
    twai_reconfigure_alerts(alerts, NULL);

    _driver_installed = true;
    _last_rate_calc_ms = millis();
    LOG_PRINTLN("[CAN] TWAI driver successfully installed & started in LISTEN-ONLY mode.");

    // Spawn dedicated CAN RX FreeRTOS task on Core 0
    xTaskCreatePinnedToCore(
        rxTaskTrampoline,
        "can_rx_task",
        4096,
        this,
        5,              // High priority to prevent RX queue overruns
        NULL,
        0               // Pinned to Core 0 (Radio/CAN core)
    );

    return true;
}

void CanReceiver::stop() {
    if (_driver_installed) {
        twai_stop();
        twai_driver_uninstall();
        _driver_installed = false;
        LOG_PRINTLN("[CAN] TWAI driver stopped.");
    }
}

void CanReceiver::rxTaskTrampoline(void* arg) {
    reinterpret_cast<CanReceiver*>(arg)->rxTask();
}

void CanReceiver::rxTask() {
    LOG_PRINTLN("[CAN] RX Task running on Core 0.");
    twai_message_t rx_msg;

    while (_driver_installed) {
        uint32_t alerts_triggered = 0;
        // Wait up to 50ms for TWAI alerts (alert-driven, low CPU load)
        if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(50)) == ESP_OK) {
            twai_status_info_t status;
            twai_get_status_info(&status);

            if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
                if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                    _bus_error_count = status.bus_error_count;
                    xSemaphoreGive(_mutex);
                }
            }

            if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
                if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                    _rx_missed_count = status.rx_missed_count;
                    xSemaphoreGive(_mutex);
                }
            }

            // Drain all available messages from TWAI hardware queue
            if (alerts_triggered & TWAI_ALERT_RX_DATA) {
                while (twai_receive(&rx_msg, 0) == ESP_OK) {
                    CanFrameRaw frame;
                    frame.timestamp_ms = millis();
                    frame.id = rx_msg.identifier;
                    frame.extended = rx_msg.extd;
                    frame.rtr = rx_msg.rtr;
                    frame.dlc = rx_msg.data_length_code;
                    if (frame.dlc > 8) frame.dlc = 8;
                    memcpy(frame.data, rx_msg.data, frame.dlc);

                    // Update statistics and live queues
                    processReceivedFrame(frame);

                    // Dispatch to consumer queue (SD logger, non-blocking)
                    if (_frame_queue) {
                        xQueueSend(_frame_queue, &frame, 0);
                    }
                }
            }
        }

        // Calculate packet rate every 1000ms
        uint32_t now = millis();
        if (now - _last_rate_calc_ms >= 1000) {
            if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                float seconds = (now - _last_rate_calc_ms) / 1000.0f;
                _current_rate_fps = (seconds > 0) ? (_packets_since_last_calc / seconds) : 0.0f;
                _packets_since_last_calc = 0;
                _last_rate_calc_ms = now;
                xSemaphoreGive(_mutex);
            }
        }
    }

    vTaskDelete(NULL);
}

void CanReceiver::processReceivedFrame(const CanFrameRaw& frame) {
    if (!_mutex || xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
        return;
    }

    _total_packets++;
    _packets_since_last_calc++;
    _last_packet_time_ms = frame.timestamp_ms;

    // 1. Add to recent circular buffer for display
    _recent_frames[_recent_head] = frame;
    _recent_head = (_recent_head + 1) % RECENT_FRAMES_BUF_SIZE;
    if (_recent_total < RECENT_FRAMES_BUF_SIZE) {
        _recent_total++;
    }

    // 2. Find or create CAN-ID entry in statistics table
    int found_idx = -1;
    for (size_t i = 0; i < _id_count; i++) {
        if (_id_table[i].id == frame.id && _id_table[i].extended == frame.extended) {
            found_idx = (int)i;
            break;
        }
    }

    if (found_idx >= 0) {
        // Update existing ID stats
        CanIdStats& entry = _id_table[found_idx];
        entry.count++;
        if (frame.timestamp_ms > entry.last_timestamp_ms) {
            entry.interval_ms = frame.timestamp_ms - entry.last_timestamp_ms;
        }
        entry.last_timestamp_ms = frame.timestamp_ms;
        entry.dlc = frame.dlc;
        memcpy(entry.last_data, frame.data, frame.dlc);
    } else if (_id_count < MAX_TRACKED_CAN_IDS) {
        // Register new CAN-ID
        CanIdStats& entry = _id_table[_id_count++];
        entry.id = frame.id;
        entry.extended = frame.extended;
        entry.count = 1;
        entry.first_timestamp_ms = frame.timestamp_ms;
        entry.last_timestamp_ms = frame.timestamp_ms;
        entry.interval_ms = 0;
        entry.dlc = frame.dlc;
        memcpy(entry.last_data, frame.data, frame.dlc);

        LOG_PRINTF("[CAN DISCOVERY] New CAN ID: 0x%03X (%s) DLC: %d\n",
                      frame.id, frame.extended ? "EXT" : "STD", frame.dlc);
    }

    xSemaphoreGive(_mutex);
}

bool CanReceiver::getNextFrame(CanFrameRaw& out_frame, TickType_t wait_ticks) {
    if (!_frame_queue) return false;
    return (xQueueReceive(_frame_queue, &out_frame, wait_ticks) == pdTRUE);
}

void CanReceiver::getOverview(ScannerOverview& out_overview) {
    memset(&out_overview, 0, sizeof(out_overview));
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        out_overview.total_packets = _total_packets;
        out_overview.packets_per_sec = _current_rate_fps;
        out_overview.bus_error_count = _bus_error_count;
        out_overview.rx_missed_count = _rx_missed_count;
        out_overview.active_ids_count = _id_count;
        out_overview.can_listening = _driver_installed;
        out_overview.last_packet_time_ms = _last_packet_time_ms;
        xSemaphoreGive(_mutex);
    }
}

size_t CanReceiver::getIdStatistics(CanIdStats* out_array, size_t max_count) {
    if (!out_array || max_count == 0) return 0;
    size_t copied = 0;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        copied = (_id_count < max_count) ? _id_count : max_count;
        memcpy(out_array, _id_table, copied * sizeof(CanIdStats));
        xSemaphoreGive(_mutex);
    }
    return copied;
}

size_t CanReceiver::getRecentFrames(CanFrameRaw* out_array, size_t max_count) {
    if (!out_array || max_count == 0) return 0;
    size_t copied = 0;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        size_t available = _recent_total;
        copied = (available < max_count) ? available : max_count;

        // Copy frames from newest to oldest
        for (size_t i = 0; i < copied; i++) {
            size_t idx = (_recent_head + RECENT_FRAMES_BUF_SIZE - 1 - i) % RECENT_FRAMES_BUF_SIZE;
            out_array[i] = _recent_frames[idx];
        }
        xSemaphoreGive(_mutex);
    }
    return copied;
}
