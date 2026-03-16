# Glossary

## Hardware

| Term | Definition |
|------|------------|
| **Collar Node** | The wearable device attached to an animal. Houses an ESP32, GNSS module, radio transceiver, battery, and flash storage. |
| **Gateway Node** | A fixed or portable receiver that listens for collar radio transmissions and forwards them to the backend via Wi-Fi/LTE. |
| **GNSS** | Global Navigation Satellite System – collective term for GPS, GLONASS, Galileo, and BeiDou. |
| **MCU** | Microcontroller Unit – the ESP32 SoC that runs the collar firmware. |
| **PCB** | Printed Circuit Board – the custom hardware on which components are soldered. |
| **HDOP** | Horizontal Dilution of Precision – a dimensionless factor (lower = better) describing horizontal positioning accuracy. |

## Firmware

| Term | Definition |
|------|------------|
| **Deep Sleep** | ESP32 low-power state where the CPU is off and only the RTC remains active. Wake-up triggers a full reboot. |
| **Fix** | A valid GNSS position reading with latitude, longitude, altitude, and timestamp. |
| **TelemetryPacket** | The 29-byte binary record assembled by the collar each wake cycle and stored/transmitted. |
| **CRC-16/CCITT** | 16-bit cyclic redundancy check used to verify packet integrity. |
| **IPower / IGnss / IRadio / IStorage** | Pure-virtual C++ interface classes that decouple core logic from hardware drivers. |
| **Mock** | A test-only implementation of an interface that runs in RAM on the host machine. |
| **Wake Cycle** | One full execute-then-sleep iteration: boot → sense → store → transmit → sleep. |

## Backend

| Term | Definition |
|------|------------|
| **MQTT** | Message Queuing Telemetry Transport – lightweight publish/subscribe protocol used by gateway-to-backend data ingestion. |
| **Dashboard** | Web UI that visualises collar tracks and sensor data on a map. |

## Radio

| Term | Definition |
|------|------------|
| **LoRa** | Long Range radio modulation (Semtech SX127x) – planned physical layer for collar-to-gateway links. |
| **SF** | Spreading Factor – LoRa parameter controlling range vs. data rate trade-off (SF7–SF12). |
| **BW** | Bandwidth – LoRa channel width in kHz (125 / 250 / 500 kHz). |
