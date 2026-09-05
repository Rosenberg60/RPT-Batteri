#pragma once

// =============================================================================
// PROJEKT : RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
// FIL     : wifi_config.h - WiFi & Netværksindstillinger
// =============================================================================
// Indtast dit lokale WiFi-netværk (SSID) og adgangskode herunder.
// Hvis du ikke ønsker WiFi aktiveret, kan WIFI_ENABLED sættes til false.
// =============================================================================

#define WIFI_ENABLED              true

// Dit trådløse netværk (2.4 GHz understøttes af ESP32-S3)
#define WIFI_SSID                 "Rosenberg2017_24GHz"
#define WIFI_PASSWORD             "Mads1234"

// Valgfrit værtsnavn (mDNS): gør at du kan åbne http://rpt-batteri.local i browseren
#define WIFI_HOSTNAME             "rpt-batteri"

// Webserver lytteport (standard HTTP = 80)
#define WEB_SERVER_PORT           80

// Hvor mange sekunder der forsøges forbundet ved boot før skærm og CAN starter
#define WIFI_CONNECT_TIMEOUT_SEC  8
