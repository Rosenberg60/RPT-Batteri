#pragma once

#include <Arduino.h>

/**
 * ============================================================================
 * Data structures for CAN-Scanner & Battery Monitor
 * ============================================================================
 */

// Raw CAN telegram structure (used for queues, SD logging, and UI feed)
struct CanFrameRaw {
    uint32_t timestamp_ms;
    uint32_t id;
    bool extended;
    bool rtr;
    uint8_t dlc;
    uint8_t data[8];
};

// Statistics tracked per distinct CAN-ID
struct CanIdStats {
    uint32_t id;
    bool extended;
    uint32_t count;
    uint32_t first_timestamp_ms;
    uint32_t last_timestamp_ms;
    uint32_t interval_ms;       // Rolling delta between frames (ms)
    uint8_t dlc;
    uint8_t last_data[8];
};

// Overall system / scanner status
struct ScannerOverview {
    uint32_t total_packets;
    float packets_per_sec;
    uint32_t bus_error_count;
    uint32_t rx_missed_count;
    uint32_t active_ids_count;
    bool can_listening;
    bool sd_card_mounted;
    char sd_filename[32];
    uint32_t last_packet_time_ms;
    uint32_t rx_error_counter;
    uint32_t tx_error_counter;
    uint8_t twai_state; // 0=STOPPED, 1=RUNNING, 2=BUS_OFF, 3=RECOVERING
};

// Battery data model as specified for Phase 2 & 3
struct BatteryData {
    float voltage_V;
    float current_A;
    float power_W;
    float temperature_C;
    float chargeVoltageLimit_V;
    float chargeCurrentLimit_A;
    float dischargeCurrentLimit_A;
    float dischargeCutoffVoltage_V;

    uint16_t soc_percent;
    uint16_t soh_percent;
    uint16_t totalCapacity_Ah;
    uint8_t  moduleCount;

    // Cell extremes & balance
    float minCellVoltage_V;
    float maxCellVoltage_V;
    float cellDelta_mV;
    float minCellTemp_C;
    float maxCellTemp_C;
    float cellVoltages[16];
    float pack1_cellVoltages[16]; // Battery 1: Rosen Master (16 cells)
    float pack2_cellVoltages[16]; // Battery 2: RPT Slave (16 cells)
    float pack1_minV;
    float pack1_maxV;
    float pack2_minV;
    float pack2_maxV;
    bool  individualCellsKnown;

    // Dual Pack Telemetry (Pack 1: Rosen 200Ah Master, Pack 2: RPT 300Ah Slave)
    uint16_t pack1_soc_percent;
    uint16_t pack2_soc_percent;
    float pack1_capacity_Ah;
    float pack2_capacity_Ah;
    float pack1_current_A;
    float pack2_current_A;
    float pack1_power_W;
    float pack2_power_W;
    float pack1_energy_kwh;
    float pack2_energy_kwh;
    float pack1_chargeLimit_A;
    float pack2_chargeLimit_A;
    float pack1_dischargeLimit_A;
    float pack2_dischargeLimit_A;

    char manufacturer[16];

    bool chargeAllowed;
    bool dischargeAllowed;
    bool communicationOK;
    bool warningActive;
    bool protectionActive;

    // ESP32-S3 LiPo 3.7V Battery Telemetry (Option A: TP1 via GPIO 6)
    float    lipo_voltage_V;
    uint8_t  lipo_soc_percent;
    bool     lipo_connected;

    uint32_t lastUpdate_ms;
};
