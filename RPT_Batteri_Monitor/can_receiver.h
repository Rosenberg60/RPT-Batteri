#pragma once

// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : can_receiver.h (TWAI CAN Receiver & Gateway Transmitter Header)
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "board_config.h"
#include "battery_data.h"

// Maximum distinct CAN-IDs tracked in the statistics table
#define MAX_TRACKED_CAN_IDS     64

// Size of the raw frame queue for consumer tasks (SD logger / UI)
#define CAN_FRAME_QUEUE_SIZE    64

// Size of recent circular history buffer for live rolling display
#define RECENT_FRAMES_BUF_SIZE  16

class CanReceiver {
public:
    static CanReceiver& getInstance();

    // Initialize TWAI controller in Listen-Only mode and start RX task
    bool begin(uint32_t baudrate = BOARD_CAN_DEFAULT_BAUDRATE);

    // Stop TWAI driver safely
    void stop();

    // Fetch the next raw frame from the queue (non-blocking or with timeout)
    bool getNextFrame(CanFrameRaw& out_frame, TickType_t wait_ticks = 0);

    // Get snapshot of overall statistics (thread-safe)
    void getOverview(ScannerOverview& out_overview);

    // Get snapshot of all discovered CAN-ID statistics (thread-safe)
    size_t getIdStatistics(CanIdStats* out_array, size_t max_count);

    // Get snapshot of most recent frames for live display (thread-safe)
    size_t getRecentFrames(CanFrameRaw* out_array, size_t max_count);

    // Transmit a raw CAN frame over TWAI (thread-safe, timeout 15ms)
    bool transmitFrame(uint32_t id, const uint8_t* data, uint8_t dlc, bool extended = false);

    // Check if CAN driver is installed and running
    bool isRunning() const { return _driver_installed; }

private:
    CanReceiver();
    ~CanReceiver();

    // FreeRTOS RX Task function
    static void rxTaskTrampoline(void* arg);
    void rxTask();

    // Internal stats update (called from RX task)
    void processReceivedFrame(const CanFrameRaw& frame);

    bool _driver_installed;
    QueueHandle_t _frame_queue;
    SemaphoreHandle_t _mutex;

    // Statistics storage
    CanIdStats _id_table[MAX_TRACKED_CAN_IDS];
    size_t _id_count;

    // Recent frames ring buffer for UI display
    CanFrameRaw _recent_frames[RECENT_FRAMES_BUF_SIZE];
    size_t _recent_head;
    size_t _recent_total;

    // Aggregates
    uint32_t _total_packets;
    uint32_t _bus_error_count;
    uint32_t _rx_missed_count;
    uint32_t _rx_error_counter;
    uint32_t _tx_error_counter;
    uint8_t _twai_state;
    uint32_t _last_packet_time_ms;
    uint32_t _last_rate_calc_ms;
    uint32_t _packets_since_last_calc;
    float _current_rate_fps;
};
