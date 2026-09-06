// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// MODUL   : web_server.h
// DATO/TID: 2026-09-06 19:32:00
// =============================================================================

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "battery_data.h"
#include "can_receiver.h"

/**
 * ============================================================================
 * BatteryWebServer: Integrated WiFi Management & Web Dashboard
 * ============================================================================
 */
class BatteryWebServer {
public:
    static BatteryWebServer& getInstance();

    // Start WiFi connection & HTTP Web Server
    void begin();

    // Handle client requests (call in Arduino loop)
    void loop();

    // Connection status queries
    bool isConnected() const;
    String getIpAddress() const;
    int8_t getRssi() const;
    String getHostname() const;

private:
    BatteryWebServer();
    ~BatteryWebServer();

    void setupRoutes();
    void handleRoot();
    void handleApiData();
    void handleApiScanner();
    void handleNotFound();

    WebServer _server;
    uint32_t _last_reconnect_attempt;
    bool _wifi_started;
    bool _mdns_started;
};
