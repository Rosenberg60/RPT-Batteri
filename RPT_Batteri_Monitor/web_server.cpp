#include "web_server.h"
#include "wifi_config.h"
#include "board_config.h"
#include "deye_bms_decoder.h"
#include "sd_logger.h"
#include "ui.h"

// -----------------------------------------------------------------------------
// Embedded Modern Web Dashboard (HTML5, CSS3, JavaScript - Standalone & Zero-CDN)
// -----------------------------------------------------------------------------
static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="da">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>RPT & Rosen Batterimonitor</title>
<style>
:root {
  --bg-main: #0b0f19;
  --bg-card: #151d2c;
  --bg-card-header: #1c2638;
  --border: #233148;
  --text-main: #f0f6fc;
  --text-muted: #8b949e;
  --cyan: #38bdf8;
  --green: #22c55e;
  --yellow: #eab308;
  --orange: #f97316;
  --red: #ef4444;
}
* { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; }
body { background: var(--bg-main); color: var(--text-main); padding: 16px; min-height: 100vh; }
.container { max-width: 1200px; margin: 0 auto; }

/* Header */
header { display: flex; flex-wrap: wrap; justify-content: space-between; align-items: center; padding-bottom: 16px; border-bottom: 1px solid var(--border); margin-bottom: 20px; gap: 12px; }
.title-box h1 { font-size: 1.5rem; font-weight: 700; color: #fff; display: flex; align-items: center; gap: 8px; }
.title-box p { font-size: 0.85rem; color: var(--text-muted); margin-top: 2px; }
.badges { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; }
.badge { font-size: 0.75rem; font-weight: 600; padding: 5px 10px; border-radius: 6px; display: inline-flex; align-items: center; gap: 5px; }
.badge-green { background: rgba(34, 197, 94, 0.15); color: var(--green); border: 1px solid rgba(34, 197, 94, 0.3); }
.badge-cyan { background: rgba(56, 189, 248, 0.15); color: var(--cyan); border: 1px solid rgba(56, 189, 248, 0.3); }
.badge-yellow { background: rgba(234, 179, 8, 0.15); color: var(--yellow); border: 1px solid rgba(234, 179, 8, 0.3); }
.badge-red { background: rgba(239, 68, 68, 0.15); color: var(--red); border: 1px solid rgba(239, 68, 68, 0.3); }

/* Grid layout */
.grid-4 { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 16px; margin-bottom: 20px; }
.grid-2 { display: grid; grid-template-columns: repeat(auto-fit, minmax(360px, 1fr)); gap: 16px; margin-bottom: 20px; }

/* Cards */
.card { background: var(--bg-card); border: 1px solid var(--border); border-radius: 10px; overflow: hidden; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.2); }
.card-header { background: var(--bg-card-header); padding: 10px 16px; font-size: 0.8rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; color: var(--cyan); border-bottom: 1px solid var(--border); display: flex; justify-content: space-between; align-items: center; }
.card-body { padding: 16px; }

/* Hero Metric */
.metric-hero { font-size: 2.2rem; font-weight: 800; line-height: 1.1; margin-bottom: 10px; }
.metric-sub { font-size: 0.85rem; margin-top: 6px; display: flex; justify-content: space-between; border-bottom: 1px dashed rgba(255,255,255,0.08); padding-bottom: 4px; }
.metric-sub:last-child { border-bottom: none; padding-bottom: 0; }
.metric-sub span.lbl { color: var(--text-muted); }
.metric-sub span.val { font-weight: 600; }

/* Bar gauge */
.bar-track { width: 100%; height: 12px; background: rgba(255,255,255,0.08); border-radius: 6px; overflow: hidden; margin: 10px 0; border: 1px solid rgba(255,255,255,0.1); }
.bar-fill { height: 100%; border-radius: 6px; transition: width 0.4s ease; }

/* Cells 16S chart - 4x forstørret højde for maksimal synlighed af cellespændinger */
.cells-container { display: grid; grid-template-columns: repeat(16, 1fr); gap: 6px; align-items: flex-end; height: 440px; padding: 25px 6px 8px 6px; margin-top: 10px; border-bottom: 1px solid var(--border); background: linear-gradient(to top, rgba(255,255,255,0.02) 0%, transparent 100%); border-radius: 6px; }
.cell-col { display: flex; flex-direction: column; align-items: center; height: 100%; justify-content: flex-end; position: relative; }
.cell-bar { width: 100%; border-radius: 4px 4px 0 0; background: var(--green); min-height: 12px; transition: height 0.4s cubic-bezier(0.4, 0, 0.2, 1); box-shadow: 0 0 8px rgba(0,0,0,0.4); }
.cell-val { font-size: 0.72rem; color: #fff; margin-bottom: 6px; font-weight: 700; writing-mode: vertical-rl; transform: rotate(180deg); letter-spacing: 0.5px; }
.cell-lbl { font-size: 0.70rem; color: var(--text-muted); margin-top: 6px; font-weight: 600; }

/* Live Stream Box */
.terminal { background: #000; border: 1px solid var(--border); border-radius: 6px; padding: 10px; font-family: monospace; font-size: 0.8rem; color: #38bdf8; max-height: 220px; overflow-y: auto; line-height: 1.4; }

/* Footer */
footer { text-align: center; font-size: 0.8rem; color: var(--text-muted); margin-top: 30px; padding-top: 16px; border-top: 1px solid var(--border); }
</style>
</head>
<body>
<div class="container">
  <!-- Header -->
  <header>
    <div class="title-box">
      <h1>⚡ RPT & ROSEN BATTERIMONITOR</h1>
      <p>51.2V LiFePO4 Hybrid Storage (501 Ah / 25.6 kWh) &bull; Deye CAN Bus Telemetri</p>
    </div>
    <div class="badges">
      <span id="bms-badge" class="badge badge-green">● BMS ONLINE</span>
      <span id="lipo-badge" class="badge badge-cyan">🔋 Lipo Bat: <span id="lipo-val">--</span></span>
      <span id="wifi-badge" class="badge badge-cyan">WiFi: OK (<span id="wifi-rssi">-</span> dBm)</span>
      <span id="uptime-badge" class="badge badge-cyan">UP: <span id="uptime">-</span></span>
    </div>
  </header>

  <!-- 4 Top Metric Cards -->
  <div class="grid-4">
    <!-- Card 0: SOC -->
    <div class="card">
      <div class="card-header">
        <span>State of Charge (SOC)</span>
        <span id="card-soh" style="color:#fff; font-size:0.75rem;">SOH: 100%</span>
      </div>
      <div class="card-body">
        <div id="val-soc" class="metric-hero" style="color:var(--green);">-- %</div>
        <div class="bar-track">
          <div id="bar-soc" class="bar-fill" style="width:0%; background:var(--green);"></div>
        </div>
        <div class="metric-sub"><span class="lbl">Rosen 200Ah Master:</span><span id="val-p1-soc" class="val" style="color:var(--cyan);">-- %</span></div>
        <div class="metric-sub"><span class="lbl">RPT 300Ah Slave:</span><span id="val-p2-soc" class="val" style="color:var(--green);">-- %</span></div>
        <div class="metric-sub"><span class="lbl">Total Bank Kapacitet:</span><span id="val-cap" class="val">501 Ah</span></div>
      </div>
    </div>

    <!-- Card 1: Strøm -->
    <div class="card">
      <div class="card-header">
        <span>Bank Strøm & Fordeling</span>
        <span id="val-temp" style="color:#fff; font-size:0.75rem;">Temp: -- °C</span>
      </div>
      <div class="card-body">
        <div id="val-current" class="metric-hero">--.- A</div>
        <div class="metric-sub"><span class="lbl">Rosen 200Ah (Est 40%):</span><span id="val-p1-curr" class="val" style="color:var(--cyan);">--.- A</span></div>
        <div class="metric-sub"><span class="lbl">RPT 300Ah (Est 60%):</span><span id="val-p2-curr" class="val" style="color:var(--green);">--.- A</span></div>
        <div class="metric-sub"><span class="lbl">Maks Bank Ladegrænse:</span><span id="val-chg-lim" class="val">390 A</span></div>
        <div class="metric-sub"><span class="lbl">Maks Bank Afladegrænse:</span><span id="val-dchg-lim" class="val">390 A</span></div>
      </div>
    </div>

    <!-- Card 2: Spænding -->
    <div class="card">
      <div class="card-header">
        <span>DC Bus Spænding</span>
        <span id="val-avg-cell" style="color:#fff; font-size:0.75rem;">Avg: -.--- V</span>
      </div>
      <div class="card-body">
        <div id="val-voltage" class="metric-hero" style="color:var(--yellow);">--.-- V</div>
        <div class="metric-sub"><span class="lbl">Maks Ladespænding:</span><span id="val-v-lim" class="val">57.60 V</span></div>
        <div class="metric-sub"><span class="lbl">Afladning Cut-off:</span><span id="val-v-cut" class="val">44.80 V</span></div>
        <div class="metric-sub"><span class="lbl">Fælles Busbar:</span><span class="val">51.2V LiFePO4 (16S)</span></div>
        <div class="metric-sub"><span class="lbl">BMS Kommunikation:</span><span id="val-bms-stat" class="val" style="color:var(--green);">OK</span></div>
      </div>
    </div>

    <!-- Card 3: Effekt -->
    <div class="card">
      <div class="card-header">
        <span>Batterieffekt (kW)</span>
        <span id="badge-mode" class="badge badge-cyan">STANDBY</span>
      </div>
      <div class="card-body">
        <div id="val-power" class="metric-hero">0.00 kW</div>
        <div class="metric-sub"><span class="lbl">Rosen 200Ah Effekt:</span><span id="val-p1-pwr" class="val" style="color:var(--cyan);">0.00 kW</span></div>
        <div class="metric-sub"><span class="lbl">RPT 300Ah Effekt:</span><span id="val-p2-pwr" class="val" style="color:var(--green);">0.00 kW</span></div>
        <div class="metric-sub"><span class="lbl">Estimeret Energi:</span><span id="val-energy" class="val">--.- kWh</span></div>
        <div class="metric-sub"><span class="lbl">Sikkerhedskontakter:</span><span id="val-switches" class="val" style="color:var(--green);">LAD / AFLAD OK</span></div>
      </div>
    </div>
  </div>

  <!-- Cells Diagnostic Section (32 Cells) -->
  <div class="grid-2">
    <!-- Rosen Pack 1 (16 cells) -->
    <div class="card">
      <div class="card-header">
        <span>Batteri 1: Rosen Master (200Ah &bull; 16 Celler)</span>
        <span id="p1-delta-lbl" style="color:#fff; font-size:0.75rem;">Min: -.---V | Max: -.---V</span>
      </div>
      <div class="card-body">
        <div id="p1-cells" class="cells-container"></div>
      </div>
    </div>

    <!-- RPT Tower Pack 2 (16 cells) -->
    <div class="card">
      <div class="card-header">
        <span>Batteri 2: RPT Tower Slave (300Ah &bull; 16 Celler)</span>
        <span id="p2-delta-lbl" style="color:#fff; font-size:0.75rem;">Min: -.---V | Max: -.---V</span>
      </div>
      <div class="card-body">
        <div id="p2-cells" class="cells-container"></div>
      </div>
    </div>
  </div>

  <!-- System Info & CAN Bus Telemetry -->
  <div class="card" style="margin-bottom:20px;">
    <div class="card-header">
      <span>CAN-Bus Telemetri & Systemdiagnostik</span>
      <span id="can-rate">Rate: -- fps</span>
    </div>
    <div class="card-body" style="display:grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap:12px;">
      <div><span style="color:var(--text-muted); font-size:0.8rem;">Samlet Modtagne Rammer:</span><br><b id="can-total">-</b></div>
      <div><span style="color:var(--text-muted); font-size:0.8rem;">Busfejl Tæller:</span><br><b id="can-errors">0</b></div>
      <div><span style="color:var(--text-muted); font-size:0.8rem;">Celledelta (Balance):</span><br><b id="cell-delta" style="color:var(--yellow);">- mV</b></div>
      <div><span style="color:var(--text-muted); font-size:0.8rem;">SD Kort Logfil:</span><br><b id="sd-status">-</b></div>
    </div>
  </div>

  <footer>
    RPT & Rosen Batterimonitor v1.5.2 &bull; ESP32-S3 (Rev 1.2) &bull; Deye CAN 500k &bull; Live opdatering aktiv
  </footer>
</div>

<script>
function buildCellsHtml(containerId, voltages, minV, maxV) {
  const c = document.getElementById(containerId);
  if (!c) return;
  let html = '';

  // Find lokal min og max spænding for denne pakke
  let localMin = 999, localMax = 0;
  for (let i = 0; i < 16; i++) {
    const v = (voltages && voltages[i] > 2.0) ? voltages[i] : 3.37;
    if (v < localMin) localMin = v;
    if (v > localMax) localMax = v;
  }
  if (localMin >= localMax) { localMin = 3.25; localMax = 3.45; }

  // Højopløsnings zoom: 3.20V til 3.45V som standard aktivt LiFePO4 arbejdsområde
  let floorV = 3.20;
  let ceilV = 3.45;
  if (localMin < floorV) floorV = Math.max(2.80, localMin - 0.05);
  if (localMax > ceilV) ceilV = Math.min(3.65, localMax + 0.05);
  let span = ceilV - floorV;
  if (span < 0.05) span = 0.05;

  for (let i = 0; i < 16; i++) {
    const v = (voltages && voltages[i] > 2.0) ? voltages[i] : 3.37;
    const clamped = Math.max(floorV, Math.min(ceilV, v));
    // 4x forstørret skala: 15% til 92% i den 440px høje boks
    const pct = Math.round(((clamped - floorV) / span) * 77 + 15);
    
    let color = '#22c55e'; // Normal grøn
    let border = 'rgba(34, 197, 94, 0.5)';
    if (Math.abs(v - minV) < 0.0015) {
      color = '#38bdf8'; // Cyan for laveste celle i banken
      border = '#0284c7';
    } else if (Math.abs(v - maxV) < 0.0015) {
      color = '#eab308'; // Gul for højeste celle i banken
      border = '#ca8a04';
    }
    
    html += `<div class="cell-col">
      <div class="cell-val" style="color:${color};">${v.toFixed(3)}V</div>
      <div class="cell-bar" style="height:${pct}%; background:${color}; border:1px solid ${border};"></div>
      <div class="cell-lbl">C${(i+1).toString().padStart(2,'0')}</div>
    </div>`;
  }
  c.innerHTML = html;
}

async function updateData() {
  try {
    const res = await fetch('/api/data');
    if (!res.ok) return;
    const d = await res.json();

    // Badges & Uptime
    document.getElementById('uptime').textContent = d.uptime || '-';
    document.getElementById('wifi-rssi').textContent = d.wifi_rssi || '-';
    const bmsBadge = document.getElementById('bms-badge');
    if (d.comm_ok) {
      bmsBadge.textContent = '● BMS ONLINE';
      bmsBadge.className = 'badge badge-green';
    } else {
      bmsBadge.textContent = '● BMS OFFLINE';
      bmsBadge.className = 'badge badge-red';
    }

    // LiPo Battery Badge (Option A via TP1 & GPIO 6)
    const lipoBadge = document.getElementById('lipo-badge');
    if (d.lipo_connected) {
      document.getElementById('lipo-val').textContent = d.lipo_voltage.toFixed(1) + 'V (' + d.lipo_percent + '%)';
      if (d.lipo_percent >= 40) {
        lipoBadge.className = 'badge badge-green';
      } else if (d.lipo_percent >= 20) {
        lipoBadge.className = 'badge badge-yellow';
      } else {
        lipoBadge.className = 'badge badge-red';
      }
    } else {
      document.getElementById('lipo-val').textContent = 'N/A';
      lipoBadge.className = 'badge badge-cyan';
    }

    // Card 0: SOC
    document.getElementById('val-soc').textContent = d.soc + ' %';
    document.getElementById('bar-soc').style.width = Math.min(100, Math.max(0, d.soc)) + '%';
    document.getElementById('card-soh').textContent = 'SOH: ' + d.soh + '%';
    document.getElementById('val-p1-soc').textContent = d.pack1.soc + ' %';
    document.getElementById('val-p2-soc').textContent = d.pack2.soc + ' %';
    document.getElementById('val-cap').textContent = (d.capacity || 501) + ' Ah';

    // Card 1: Strøm
    const currEl = document.getElementById('val-current');
    const curr = d.current;
    currEl.textContent = (curr > 0 ? '+' : '') + curr.toFixed(1) + ' A';
    if (curr > 0.5) currEl.style.color = 'var(--green)';
    else if (curr < -0.5) currEl.style.color = 'var(--orange)';
    else currEl.style.color = '#fff';

    document.getElementById('val-p1-curr').textContent = (d.pack1.current > 0 ? '+' : '') + d.pack1.current.toFixed(1) + ' A';
    document.getElementById('val-p2-curr').textContent = (d.pack2.current > 0 ? '+' : '') + d.pack2.current.toFixed(1) + ' A';
    document.getElementById('val-temp').textContent = 'Temp: ' + d.temp.toFixed(1) + ' °C';
    document.getElementById('val-chg-lim').textContent = d.limits.chargeA.toFixed(0) + ' A';
    document.getElementById('val-dchg-lim').textContent = d.limits.dischargeA.toFixed(0) + ' A';

    // Card 2: Spænding
    document.getElementById('val-voltage').textContent = d.voltage.toFixed(2) + ' V';
    const avg = d.voltage > 10 ? (d.voltage / 16).toFixed(3) : '-.---';
    document.getElementById('val-avg-cell').textContent = 'Avg: ' + avg + ' V';
    document.getElementById('val-v-lim').textContent = d.limits.chargeV.toFixed(2) + ' V';
    document.getElementById('val-v-cut').textContent = d.limits.cutoffV.toFixed(2) + ' V';
    document.getElementById('val-bms-stat').textContent = d.comm_ok ? 'OK (500k)' : 'Afventer';

    // Card 3: Effekt
    const pwr = d.power / 1000;
    const pwrEl = document.getElementById('val-power');
    pwrEl.textContent = (pwr > 0 ? '+' : '') + pwr.toFixed(2) + ' kW';
    const modeEl = document.getElementById('badge-mode');
    if (d.power > 50) {
      pwrEl.style.color = 'var(--green)';
      modeEl.textContent = 'OPLADNING';
      modeEl.className = 'badge badge-green';
    } else if (d.power < -50) {
      pwrEl.style.color = 'var(--orange)';
      modeEl.textContent = 'AFLADNING';
      modeEl.className = 'badge badge-red';
    } else {
      pwrEl.style.color = 'var(--cyan)';
      modeEl.textContent = 'STANDBY';
      modeEl.className = 'badge badge-cyan';
    }
    document.getElementById('val-p1-pwr').textContent = (d.pack1.power / 1000).toFixed(2) + ' kW';
    document.getElementById('val-p2-pwr').textContent = (d.pack2.power / 1000).toFixed(2) + ' kW';
    const estKwh = (d.capacity * d.voltage * (d.soc / 100) / 1000).toFixed(1);
    document.getElementById('val-energy').textContent = estKwh + ' kWh';

    // Cells Diagnostics
    document.getElementById('p1-delta-lbl').textContent = `Min: ${d.pack1.minV.toFixed(3)}V | Max: ${d.pack1.maxV.toFixed(3)}V`;
    document.getElementById('p2-delta-lbl').textContent = `Min: ${d.pack2.minV.toFixed(3)}V | Max: ${d.pack2.maxV.toFixed(3)}V`;
    buildCellsHtml('p1-cells', d.pack1.cells, d.minCellV, d.maxCellV);
    buildCellsHtml('p2-cells', d.pack2.cells, d.minCellV, d.maxCellV);

    // Diagnostics & CAN
    document.getElementById('cell-delta').textContent = Math.round(d.delta_mv) + ' mV ' + (d.delta_mv < 20 ? '(Perfekt)' : (d.delta_mv < 50 ? '(God)' : '(Ubalance)'));
    document.getElementById('can-total').textContent = d.can_packets;
    document.getElementById('can-rate').textContent = 'Rate: ' + d.can_rate.toFixed(1) + ' fps';
    document.getElementById('can-errors').textContent = d.can_errors;
    document.getElementById('sd-status').textContent = d.sd_mounted ? d.sd_file : 'Intet SD-kort';

  } catch (err) {
    console.warn('Fejl ved hentning af telemetri:', err);
  }
}

// Start auto-polling
setInterval(updateData, 1500);
updateData();
</script>
</body>
</html>
)rawliteral";

// -----------------------------------------------------------------------------
// BatteryWebServer Implementation
// -----------------------------------------------------------------------------
BatteryWebServer& BatteryWebServer::getInstance() {
    static BatteryWebServer instance;
    return instance;
}

BatteryWebServer::BatteryWebServer()
    : _server(WEB_SERVER_PORT),
      _last_reconnect_attempt(0),
      _wifi_started(false),
      _mdns_started(false)
{
}

BatteryWebServer::~BatteryWebServer() {
}

void BatteryWebServer::begin() {
#if WIFI_ENABLED
    // Check if user has updated wifi_config.h with their real WiFi SSID
    if (strcmp(WIFI_SSID, "DIT_WIFI_NAVN") == 0 || strlen(WIFI_SSID) == 0) {
        LOG_PRINTLN("[WIFI] Standard SSID 'DIT_WIFI_NAVN' fundet. WiFi afventer konfiguration i wifi_config.h.");
        _wifi_started = false;
        return;
    }

    LOG_PRINTLN("[WIFI] Initializing WiFi subsystem...");
    WiFi.persistent(false); // CRITICAL: Stop writing AP config to SPI Flash NVS! Prevents Flash locks & LCD DMA starve.
    WiFi.mode(WIFI_STA);
    WiFi.setScanMethod(WIFI_FAST_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    WiFi.setAutoReconnect(true);

    // Event handler: ensures TX power is clamped to 8.5 dBm BEFORE any channel scanning or probe bursts
    WiFi.onEvent([](arduino_event_id_t event, arduino_event_info_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_START) {
            WiFi.setSleep(false);
            WiFi.setTxPower(WIFI_POWER_8_5dBm);
            LOG_PRINTLN("[WIFI] STA started: RF power set to 8.5 dBm, sleep disabled.");
        } else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
            WiFi.setSleep(false);
            WiFi.setTxPower(WIFI_POWER_8_5dBm);
            LOG_PRINTLN("[WIFI] STA connected: RF power confirmed at 8.5 dBm.");
        } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            WiFi.setSleep(false);
            WiFi.setTxPower(WIFI_POWER_8_5dBm);
            LOG_PRINTF("[WIFI] STA IP assigned: %s (RSSI: %d dBm)\n",
                       WiFi.localIP().toString().c_str(), WiFi.RSSI());
            // Resynchronize RGB GDMA scanout immediately after connection burst
            UIManager::getInstance().resyncDisplay();
        } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            WiFi.setSleep(false);
            WiFi.setTxPower(WIFI_POWER_8_5dBm);
            LOG_PRINTLN("[WIFI] STA disconnected: maintaining low RF power for reconnection.");
        }
    });

    LOG_PRINTF("[WIFI] Fast-connecting to SSID: %s ...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Allow fast connection attempt during boot (typically 1-2 sec with FAST_SCAN)
    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs < (WIFI_CONNECT_TIMEOUT_SEC * 1000))) {
        delay(100);
        yield();
    }

    if (WiFi.status() == WL_CONNECTED) {
        WiFi.setSleep(false);
        WiFi.setTxPower(WIFI_POWER_8_5dBm);
        LOG_PRINTF("[WIFI] Connected! IP Address: %s (RSSI: %d dBm)\n",
                   WiFi.localIP().toString().c_str(), WiFi.RSSI());

        if (MDNS.begin(WIFI_HOSTNAME)) {
            _mdns_started = true;
            MDNS.addService("http", "tcp", WEB_SERVER_PORT);
            LOG_PRINTF("[WIFI] mDNS responder active: http://%s.local\n", WIFI_HOSTNAME);
        }
    } else {
        LOG_PRINTLN("[WIFI WARNING] Could not connect immediately. Background auto-reconnect active.");
    }

    _wifi_started = true;
    setupRoutes();
    _server.begin();
    LOG_PRINTF("[WIFI] HTTP Web Server running on port %d.\n", WEB_SERVER_PORT);
#else
    LOG_PRINTLN("[WIFI] WiFi disabled in wifi_config.h.");
#endif
}

void BatteryWebServer::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/api/data", HTTP_GET, [this]() { handleApiData(); });
    _server.on("/api/scanner", HTTP_GET, [this]() { handleApiScanner(); });
    _server.onNotFound([this]() { handleNotFound(); });
}

void BatteryWebServer::loop() {
#if WIFI_ENABLED
    if (!_wifi_started) return;

    // Background WiFi reconnect handler
    uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        if (now - _last_reconnect_attempt > 30000) {
            _last_reconnect_attempt = now;
            LOG_PRINTLN("[WIFI] Reconnecting to WiFi...");
            WiFi.reconnect();
        }
    } else if (!_mdns_started) {
        if (MDNS.begin(WIFI_HOSTNAME)) {
            _mdns_started = true;
            MDNS.addService("http", "tcp", WEB_SERVER_PORT);
            LOG_PRINTF("[WIFI] mDNS responder active: http://%s.local\n", WIFI_HOSTNAME);
        }
    }

    // Process incoming client requests
    _server.handleClient();
#endif
}

bool BatteryWebServer::isConnected() const {
#if WIFI_ENABLED
    return (WiFi.status() == WL_CONNECTED);
#else
    return false;
#endif
}

String BatteryWebServer::getIpAddress() const {
#if WIFI_ENABLED
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    if (strcmp(WIFI_SSID, "DIT_WIFI_NAVN") == 0) {
        return String("Afventer SSID");
    }
    return String("Forbinder...");
#else
    return String("Deaktiveret");
#endif
}

int8_t BatteryWebServer::getRssi() const {
#if WIFI_ENABLED
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
#endif
    return 0;
}

String BatteryWebServer::getHostname() const {
    return String(WIFI_HOSTNAME);
}

void BatteryWebServer::handleRoot() {
    _server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void BatteryWebServer::handleApiData() {
    BatteryData bData;
    DeyeBmsDecoder::getInstance().getBatteryData(bData);

    ScannerOverview overview;
    CanReceiver::getInstance().getOverview(overview);

    uint32_t upSec = millis() / 1000;
    char upStr[32];
    snprintf(upStr, sizeof(upStr), "%02lu:%02lu:%02lu", upSec / 3600, (upSec % 3600) / 60, upSec % 60);

    String json;
    json.reserve(2048);
    json = "{";
    json += "\"comm_ok\":" + String(bData.communicationOK ? "true" : "false") + ",";
    json += "\"uptime\":\"" + String(upStr) + "\",";
    json += "\"wifi_rssi\":" + String(getRssi()) + ",";
    json += "\"voltage\":" + String(bData.voltage_V, 2) + ",";
    json += "\"current\":" + String(bData.current_A, 1) + ",";
    json += "\"power\":" + String(bData.power_W, 1) + ",";
    json += "\"temp\":" + String(bData.temperature_C, 1) + ",";
    json += "\"soc\":" + String(bData.soc_percent) + ",";
    json += "\"soh\":" + String(bData.soh_percent) + ",";
    json += "\"capacity\":" + String(bData.totalCapacity_Ah) + ",";
    json += "\"minCellV\":" + String(bData.minCellVoltage_V, 3) + ",";
    json += "\"maxCellV\":" + String(bData.maxCellVoltage_V, 3) + ",";
    json += "\"delta_mv\":" + String(bData.cellDelta_mV, 1) + ",";
    json += "\"lipo_voltage\":" + String(bData.lipo_voltage_V, 1) + ",";
    json += "\"lipo_percent\":" + String(bData.lipo_soc_percent) + ",";
    json += "\"lipo_connected\":" + String(bData.lipo_connected ? "true" : "false") + ",";

    // Pack 1 (Rosen 200Ah)
    json += "\"pack1\":{";
    json += "\"name\":\"Rosen Master\",";
    json += "\"soc\":" + String(bData.pack1_soc_percent) + ",";
    json += "\"current\":" + String(bData.pack1_current_A, 1) + ",";
    json += "\"power\":" + String(bData.pack1_power_W, 1) + ",";
    json += "\"energy\":" + String(bData.pack1_energy_kwh, 1) + ",";
    json += "\"minV\":" + String(bData.pack1_minV, 3) + ",";
    json += "\"maxV\":" + String(bData.pack1_maxV, 3) + ",";
    json += "\"cells\":[";
    for (int i = 0; i < 16; i++) {
        json += String(bData.pack1_cellVoltages[i], 3);
        if (i < 15) json += ",";
    }
    json += "]},";

    // Pack 2 (RPT 300Ah)
    json += "\"pack2\":{";
    json += "\"name\":\"RPT Tower\",";
    json += "\"soc\":" + String(bData.pack2_soc_percent) + ",";
    json += "\"current\":" + String(bData.pack2_current_A, 1) + ",";
    json += "\"power\":" + String(bData.pack2_power_W, 1) + ",";
    json += "\"energy\":" + String(bData.pack2_energy_kwh, 1) + ",";
    json += "\"minV\":" + String(bData.pack2_minV, 3) + ",";
    json += "\"maxV\":" + String(bData.pack2_maxV, 3) + ",";
    json += "\"cells\":[";
    for (int i = 0; i < 16; i++) {
        json += String(bData.pack2_cellVoltages[i], 3);
        if (i < 15) json += ",";
    }
    json += "]},";

    // Inverter Limits & Safety
    json += "\"limits\":{";
    json += "\"chargeV\":" + String(bData.chargeVoltageLimit_V, 2) + ",";
    json += "\"chargeA\":" + String(bData.chargeCurrentLimit_A, 1) + ",";
    json += "\"dischargeA\":" + String(bData.dischargeCurrentLimit_A, 1) + ",";
    json += "\"cutoffV\":" + String(bData.dischargeCutoffVoltage_V, 2) + ",";
    json += "\"chgAllowed\":" + String(bData.chargeAllowed ? "true" : "false") + ",";
    json += "\"dchgAllowed\":" + String(bData.dischargeAllowed ? "true" : "false");
    json += "},";

    // Scanner / SD
    json += "\"can_packets\":" + String((unsigned long)overview.total_packets) + ",";
    json += "\"can_rate\":" + String(overview.packets_per_sec, 1) + ",";
    json += "\"can_errors\":" + String((unsigned long)overview.bus_error_count) + ",";
    json += "\"sd_mounted\":" + String(overview.sd_card_mounted ? "true" : "false") + ",";
    json += "\"sd_file\":\"" + String(overview.sd_filename) + "\"";

    json += "}";

    _server.send(200, "application/json", json);
}

void BatteryWebServer::handleApiScanner() {
    CanIdStats idStats[14];
    size_t count = CanReceiver::getInstance().getIdStatistics(idStats, 14);

    String json;
    json.reserve(1024);
    json = "[";
    for (size_t i = 0; i < count; i++) {
        char idHex[10];
        snprintf(idHex, sizeof(idHex), "0x%03X", (unsigned int)idStats[i].id);

        char payloadHex[32] = "";
        for (int b = 0; b < idStats[i].dlc && b < 8; b++) {
            char bHex[6];
            snprintf(bHex, sizeof(bHex), "%02X ", idStats[i].last_data[b]);
            strcat(payloadHex, bHex);
        }

        json += "{";
        json += "\"id\":\"" + String(idHex) + "\",";
        json += "\"dlc\":" + String(idStats[i].dlc) + ",";
        json += "\"count\":" + String((unsigned long)idStats[i].count) + ",";
        json += "\"interval\":" + String((unsigned long)idStats[i].interval_ms) + ",";
        json += "\"payload\":\"" + String(payloadHex) + "\"";
        json += "}";
        if (i < count - 1) json += ",";
    }
    json += "]";

    _server.send(200, "application/json", json);
}

void BatteryWebServer::handleNotFound() {
    _server.send(404, "text/plain", "404: Siden blev ikke fundet");
}
