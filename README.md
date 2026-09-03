# RPT-Batterimonitor med Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)

Dette projekt er en dedikeret CAN-bus monitor og analysator for et **RPT Tower LiFePO₄-batteri (RPES-51.2V300AH, 15,36 kWh)**.

Systemet kører på en **Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)** med 7,0" IPS RGB-touchskærm (800 × 480).

Projektet er opdelt i tre faser:
* **Fase 1 (Nuværende status – Fuldført & Verificeret)**: Komplet CAN-scanner, der opsamler og analyserer alle CAN-telegrammer punkt-til-punkt fra batteriet, viser statistik og live-trafik på 7" skærmen/serielporten og logger data til MicroSD-kort i CSV-format.
* **Fase 2 (Afventer rå log)**: Dekodning af batteriparametre (Spænding, Strøm, SOC, SOH, Temperatur, Ladegrænser, Alarmer).
* **Fase 3**: Fuld grafisk brugerflade (LVGL / touch-dashboard med alarmer, grafer og historik).

---

## 1. Systemarkitektur & Forbindelser

Inverteren (**Deye SUN-12K-SG05LP3-EU**) og batteriet (**RPT Tower 51.2V 300Ah**) er forbundet via **RS485**. Denne forbindelse forbliver fuldstændig urørt.

Waveshare 7"-displayet forbindes direkte til batteriets **CAN-port** som en separat punkt-til-punkt forbindelse:

```text
+-----------------------------------------------------------------------------------------+
|                                    SYSTEMOVERSIGT                                       |
|                                                                                         |
|   +-----------------------+              RS485              +-----------------------+   |
|   |  Deye SUN-12K         |<===============================>|  RPT Tower Batteri    |   |
|   |  Inverter             |   (Eksisterende bus, urørt)     |  51.2V 300Ah / 15kWh  |   |
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

## 2. Hardwareguide for Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)

### Fysisk placering af stik og jumpere (se foto i `docs/waveshare_7_board_rev1.2.jpg`):

1. **Bunden af printet (Hvide JST-PH2.0 stik fra venstre mod højre)**:
   * `Battery` (2-ben): `-` (Venstre, GND), `+` (Højre, 3,7V Li-ion).
   * `RS-485` (2-ben): `A`, `B`.
   * `I2C` (4-ben): **`VCC`**, **`GND`**, `SDA`, `SCL`.
   * `CAN` (2-ben): **`L`** (Venstre, CAN-L), **`H`** (Højre, CAN-H).
   * `Sensor AD` (3-ben): `3V3`, `GND`, `AD`.
2. **Venstre side af printet (Gule jumpere)**:
   * Øverste gule jumper: `RS-485 120R / NC`.
   * Nederste gule jumper: **`CAN 120R / NC`**. *(Skal sidde på `120R` – se nedenfor)*.
3. **Midt-højre på printet (Sort 3-bens jumper)**:
   * Jumper mærket **`I2C 3V3 / 5V`**. *(Bruges til strømforsyning uden USB – se nedenfor)*.
4. **Toppen af printet**:
   * Venstre USB Type-C: `USB` (Native ESP32-S3 USB).
   * Højre USB Type-C: `UART1` (Onboard CH343 USB-to-UART converter til programmering/seriel monitor).
   * MicroSD-kortlæser (TF-card).

---

## 3. Strømforsyning (5,0 V DC) – Uden brug af USB-kabel

Kortet skal forsynes med **5,0 V DC (±5%)** og trækker ca. 450–600 mA (op til 1,0 A ved fuld skærmbelysning).

> [!CAUTION]
> **Advarsel om højspænding**:
> Batteriet leverer 51,2 V nominel (48–58,4 V DC). Forbind **aldrig** batterispænding direkte til kortet!
> Monter altid en isoleret **DC/DC step-down konverter (40–60V ind $\rightarrow$ 5,0V 3A ud)** med en **1–2A DC-sikring** på batteriets pluspol.

### Metoden til at forsyne kortet via skrueterminal / PH2.0 (uden USB):

1. Flyt den sorte jumper mærket **`I2C 3V3 / 5V`** over på **`5V`**-positionen.
   * Dette forbinder kortets interne 5V-strømskinne direkte til `VCC`-benet på det hvide I2C-stik.
2. Forbind fra din 5V DC/DC-konverter til det hvide 4-bens **`I2C`**-stik i bunden:
   * **+5,0 V DC** $\rightarrow$ Forbindes til **`VCC`** (Pin 1, yderst til venstre).
   * **GND (Minus)** $\rightarrow$ Forbindes til **`GND`** (Pin 2, nr. to fra venstre).
   * *`SDA` og `SCL` efterlades tomme.*

*(Alternativt kan anvendes et vinklet industrielt USB-C "pigtail" kabel med afisolerede ender direkte i DC/DC-konverteren).*

---

## 4. CAN-forbindelse & 120 Ω Bus-terminering

Da der er tale om et punkt-til-punkt kabel mellem batteri og Waveshare:

### Kabelforbindelse (RJ45 til 2-bens CAN-stik):
Fra et standard netværkskabel forbundet til RPT-batteriets CAN-port:
* **Pin 4 (Blå leder)** $\rightarrow$ Forbindes til **`H`** (CAN-H) på Waveshare CAN-stikket.
* **Pin 5 (Blå/Hvid leder)** $\rightarrow$ Forbindes til **`L`** (CAN-L) på Waveshare CAN-stikket.

### 120 Ω Terminering:
* RPT-batteriets CAN-port har en intern 120 Ω modstand i batterienden.
* Waveshare 7"-kortet skal have 120 Ω i displayenden for at opnå de standardiserede **60 Ω samlet modstand**.
* **Status**: Den nederste gule jumper mærket **`CAN 120R / NC`** sidder allerede i position **`120R`** på dit kort. Den er hermed korrekt konfigureret fra fabrikken.

### Hardware ACK:
Da batteriet sender som eneste anden node på linjen, kræver CAN-standarden (ISO 11898) en kvitteringsbit (ACK). Firmwaren er konfigureret med `BOARD_CAN_POINT_TO_POINT = true`:
* ESP32 TWAI-hardwaren kvitterer automatisk modtagne pakker med den nødvendige ACK-bit.
* Firmwaren sender **0 datarammer** ud på bussen, så scanneren forbliver 100% passiv.

---

## 5. Softwarearkitektur

Projektet er fuldt opbygget i `c:\MIRO Github repos\RPT Batteri\`:

```text
├── platformio.ini              # ESP32-S3 miljø (16MB Flash, 8MB OPI PSRAM)
├── include/
│   ├── board_config.h          # Hardwarepins, I2C, SPI, CAN (TX=20, RX=19), CH422G
│   ├── battery_data.h          # Datastrukturer for CAN-rammer, ID-statistik og batteridata
│   ├── can_receiver.h          # TWAI driver, ringbuffer, thread-safe statistik
│   ├── sd_logger.h             # MicroSD-logning til nummererede CSV-filer
│   ├── ui.h                    # 7" 800x480 RGB display dashboard og fontmotor
│   └── deye_bms_decoder.h      # Forberedt dekoder-klasse til Fase 2
├── src/
│   ├── main.cpp                # Setup, FreeRTOS task-orkestrering, seriel udlæsning
│   ├── can_receiver.cpp        # Alert-drevet CAN-modtagelse uden malloc i modtagestien
│   ├── sd_logger.cpp           # CSV-skrivning med periodisk flush (1 sek / 50 pakker)
│   ├── ui.cpp                  # 800x480 PSRAM-framebuffer rendering (10 Hz)
│   └── deye_bms_decoder.cpp    # Stub til senere implementering af Deye/RPT-protokol
└── docs/
    ├── ESP32-S3-Touch-LCD-7-Sch.pdf   # Komplet fabriks-skematik fra Waveshare
    └── waveshare_7_board_rev1.2.jpg   # Foto af det fysiske kort med annoteringer
```

### Kritiske GPIO- og IO-Expander mappings:
* **CAN TX**: GPIO 20
* **CAN RX**: GPIO 19
* **CH422G EXIO5**: Sættes automatisk **HIGH** i koden for at omskifte GPIO 19/20 til den indbyggede TJA1051 CAN-transceiver.
* **CH422G EXIO2**: Sættes automatisk **HIGH** for at aktivere skærmens baggrundsbelysning.
* **I2C**: GPIO 8 (SDA), GPIO 9 (SCL).
* **MicroSD SPI**: GPIO 11 (MOSI), GPIO 12 (SCK), GPIO 13 (MISO), `CH422G EXIO4` (CS).
* **Display 800×480 RGB**: Kører via hardware-RGB interface i PSRAM ved 16 MHz pixel clock.

---

## 6. MicroSD CSV Logformat

Hvis et FAT32-formateret MicroSD-kort er isat ved opstart, oprettes automatisk næste ledige logfil (f.eks. `/can_log_001.csv`, `/can_log_002.csv`).

Formatet er standardiseret CSV:
```csv
timestamp_ms,can_id,extended,dlc,data0,data1,data2,data3,data4,data5,data6,data7
12450,0x356,0,8,34,02,FF,00,1A,0B,00,00
12550,0x351,0,8,38,02,D0,07,D0,07,00,00
```

---

## 7. Kompilering & Upload

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
1. Installer ESP32 board-pakken (v2.0.x eller v3.x).
2. Vælg board: **ESP32S3 Dev Module**.
3. Under **Tools**:
   * **Flash Size**: `16MB (128Mb)`
   * **Partition Scheme**: `16M Flash (3MB APP/9.9MB FATFS)` eller `Default 16MB`
   * **PSRAM**: `OPI PSRAM`
   * **CPU Frequency**: `240MHz`
   * **Upload Speed**: `921600`
   * **Port**: Vælg USB-TO-UART porten (CH343).
4. Klik **Upload**.

---

## 8. Næste skridt (Klar til Fase 2)

Når kortet er monteret og tændt på anlægget:
1. Skærmen viser live CAN-statistik over alle fundne ID'er.
2. Seriel monitoren og MicroSD-kortet opsamler rå data under drift.
3. Send CSV-logfilen eller et udtræk fra seriel monitor til analysering, hvorefter Fase 2 (BMS dekodning af spænding, strøm, SOC, SOH, cellebalancer og temperaturer) implementeres direkte.
