# RPT & Rosen Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)

Dette projekt er en dedikeret CAN-bus monitor, telemetrilogger og web-dashboard for et **51,2V LiFePO₄ hybridsystem: Rosen 200Ah Master + RPT Tower 300Ah (Samlet 501 Ah / 25,6 kWh)** tilsluttet en **Deye SUN-12K-SG05LP3-EU** hybridinverter.

Systemet kører på en **Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)** med 7,0" IPS RGB-skærm (800 × 480 capacitive touch), MicroSD-blackbox-logger og indbygget WiFi-webserver.

---

## 1. Systemstatus & Funktioner

* **Multi-Side Touch Dashboard (800 × 480 IPS)**:
  * **Side 1 (Hovedoversigt)**: Store hero-kort for **SOC (%)**, **Storage Power (kW)**, **Bank Spænding (V)** og **Total & Peak Strøm (A)** (med automatisk skalering for afladestrømme $> 100\text{ A}$). Statusfelter for Min/Max cellespænding, cell delta (mV), ladestatus, invertergrænser og intern 3,7V LiPo backup-batteristatus (`Lipo Bat: x.xV (xx%)`).
  * **Side 2 (Cell Diagnostics 32S)**: Komplet 32-celle visning med 16S Rosen Master og 16S RPT Tower. Dynamisk farvekodning (grøn/gul/rød/blå) og flimmerfri differential-opdatering.
  * **Side 3 (CAN Scanner)**: Real-tids tabel over alle modtagne CAN ID'er, frekvens, antal, payload og live frame-terminal.
  * **Touch Styring**: Touch er eksklusivt afgrænset til de 3 navigationsknapper i bunden ($Y \ge 420$) for at forhindre utilsigtede sideskift.
* **Integreret Web-Interface (WiFi)**:
  * Responsivt web-dashboard tilgåeligt via `http://rpt-batteri.local` eller ESP32's IP-adresse.
  * 4x forhøjede spændingssøjler for maksimal visualisering af cellebalance i 16S pack.
  * REST API endpoints: `/api/data` og `/api/scanner` med JSON telemetri.
* **Batteri Backup Monitor**:
  * Analog måling af tilsluttet 3,7V LiPo-batteri via GPIO 4 (ADC) med SOC estimering.
* **MicroSD Blackbox Logging**:
  * Automatisk logning af samtlige rå CAN-rammer til MicroSD i standard CSV-format (`/can_log_XXX.csv`).
* **Høj Stabilitet & Multi-Kerne Arkitektur**:
  * Dedikerede FreeRTOS tasks på Kerne 0 (CAN + Logger + WebServer) og Kerne 1 (UI Display Rendering).
  * Ikke-blokerende CAN queue drain, FreeRTOS mutexes i stedet for spinlocks, og beskyttelse mod Task Watchdog timeouts.

---

## 2. Systemarkitektur & Forbindelser

Inverteren (**Deye SUN-12K-SG05LP3-EU**) og batterierne (**Rosen 200Ah Master** og **RPT Tower 300Ah**) kommunikerer via batteriets BMS CAN-bus:

```text
+-----------------------------------------------------------------------------------------+
|                                    SYSTEMOVERSIGT                                       |
|                                                                                         |
|   +-----------------------+              CAN Bus            +-----------------------+   |
|   |  Deye SUN-12K         |<===============================>|  Rosen Master 200Ah   |   |
|   |  Inverter             |   (Eksisterende bus)            |  + RPT Tower 300Ah    |   |
|   +-----------------------+                                 +-----------+-----------+   |
|                                                                         |               |
|                                                     CAN-Port (RJ45)     |               |
|                                                     Pin 4 = CAN-H       |               |
|                                                     Pin 5 = CAN-L       |               |
|                                                                         v               |
|                                                             +-----------------------+   |
|                                                             |  Waveshare 7" Rev1.2  |   |
|                                                             |  ESP32-S3 (800x480)   |   |
|                                                             +-----------------------+   |
+-----------------------------------------------------------------------------------------+
```

---

## 3. Hardwareguide for Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)

### Fysisk placering af stik og jumpere:

1. **Bunden af printet (Hvide JST-PH2.0 stik fra venstre mod højre)**:
   * `Battery` (2-ben): `-` (Venstre, GND), `+` (Højre, 3,7V LiPo/Li-ion).
   * `RS-485` (2-ben): `A`, `B`.
   * `I2C` (4-ben): **`VCC`**, **`GND`**, `SDA`, `SCL`.
   * `CAN` (2-ben): **`L`** (Venstre, CAN-L), **`H`** (Højre, CAN-H).
   * `Sensor AD` (3-ben): `3V3`, `GND`, `AD` (GPIO 4 ADC).
2. **Venstre side af printet (Gule jumpere)**:
   * Øverste gule jumper: `RS-485 120R / NC`.
   * Nederste gule jumper: **`CAN 120R / NC`** *(Skal sidde på `120R`)*.
3. **Midt-højre på printet (Sort 3-bens jumper)**:
   * Jumper mærket **`I2C 3V3 / 5V`** *(Sættes på `5V` hvis kortet forsynes via I2C-stikket)*.
4. **Toppen af printet**:
   * Venstre USB Type-C: `USB` (Native ESP32-S3 USB).
   * Højre USB Type-C: `UART1` (Onboard CH343 USB-to-UART programmeringsport).
   * MicroSD-kortlæser (TF-card).

---

## 4. CAN-forbindelse & Terminering

Fra et standard RJ45 netværkskabel forbundet til batteriets CAN-port:
* **Pin 4 (Blå leder)** $\rightarrow$ Forbindes til **`H`** (CAN-H) på Waveshare CAN-stikket.
* **Pin 5 (Blå/Hvid leder)** $\rightarrow$ Forbindes til **`L`** (CAN-L) på Waveshare CAN-stikket.
* Jumperen **`CAN 120R / NC`** skal sidde i **`120R`** position.

---

## 5. Softwarearkitektur

Kildekoden er struktureret modulært i `RPT_Batteri_Monitor/`:

```text
├── platformio.ini              # PlatformIO konfiguration (bygger direkte fra sketch-mappen)
├── RPT_Batteri_Monitor/        # [Arduino Sketch Mappe] - Åbnes i Arduino IDE eller PlatformIO
│   ├── RPT_Batteri_Monitor.ino # Hoved-sketch (setup, loop, task scheduling, CAN drain)
│   ├── board_config.h          # Pin-definitioner (CAN TX=20, RX=19, CH422G, I2C, SPI)
│   ├── board_battery.h/.cpp    # 3,7V LiPo spændingsmåling via ADC (GPIO 4)
│   ├── battery_data.h          # Komplette datastrukturer for 32S telemetri, limits & statistik
│   ├── can_receiver.h/.cpp     # TWAI driver med ringbuffer og alert-drevet RX
│   ├── deye_bms_decoder.h/.cpp # Deye/Rosen/RPT CAN protokol-dekoder med FreeRTOS mutex
│   ├── sd_logger.h/.cpp        # MicroSD CSV logger (flush hvert 1000ms / 50 rammer)
│   ├── ui.h/.cpp               # 800x480 PSRAM-framebuffer rendering, 3-siders UI & touch
│   ├── web_server.h/.cpp       # WiFi webserver, live dashboard og REST JSON endpoints
│   └── wifi_config.h           # WiFi SSID, adgangskode og værtsnavn
└── docs/
    ├── ESP32-S3-Touch-LCD-7-Sch.pdf   # Waveshare skematik
    └── waveshare_7_board_rev1.2.jpg   # Foto af printet med stikforklaringer
```

---

## 6. Kompilering & Upload

### Mulighed A: PlatformIO (Anbefalet)
1. Forbind kortet til PC via **UART1** USB-C porten (højre port).
2. Byg og upload med én kommando:
   ```bash
   pio run -t upload
   ```
3. Åbn seriel monitor ved 115200 baud:
   ```bash
   pio device monitor
   ```

### Mulighed B: Arduino IDE
1. Åbn filen: **`RPT_Batteri_Monitor/RPT_Batteri_Monitor.ino`** i Arduino IDE.
2. Vælg board: **ESP32S3 Dev Module**.
3. Under **Tools** vælges:
   * **Flash Size**: `16MB (128Mb)`
   * **Partition Scheme**: `16M Flash (3MB APP/9.9MB FATFS)` eller `Default 16MB`
   * **PSRAM**: `OPI PSRAM`
   * **CPU Frequency**: `240MHz`
   * **Upload Speed**: `921600`
   * **USB CDC On Boot**: `Disabled` (eller `Enabled`)
   * **Port**: Vælg COM-porten for USB-TO-UART forbindelsen (CH343 på UART1).
4. Klik på **Upload**.
