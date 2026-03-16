# Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      SatHerd-EU System                      │
│                                                             │
│  ┌──────────────┐   LoRa RF   ┌──────────────┐             │
│  │  Collar Node │────────────▶│ Gateway Node │             │
│  │  (ESP32)     │             │  (ESP32)     │             │
│  └──────────────┘             └──────┬───────┘             │
│                                      │ MQTT                 │
│                               ┌──────▼───────┐             │
│                               │ mqtt-ingest  │             │
│                               └──────┬───────┘             │
│                                      │                      │
│                               ┌──────▼───────┐             │
│                               │   Database   │             │
│                               └──────┬───────┘             │
│                                      │                      │
│                               ┌──────▼───────┐             │
│                               │  Dashboard   │             │
│                               └──────────────┘             │
└─────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

### Collar Node (`firmware/collar-node/`)
- Acquires GNSS fix and power readings each wake cycle.
- Serialises a `TelemetryPacket` and appends it to on-device flash.
- Broadcasts the packet over LoRa to any gateway in range.
- Spends ~85 % of its lifetime in deep sleep to maximise battery life.

### Gateway Node (`firmware/gateway-node/`)
- Passively listens on LoRa channel.
- Validates incoming packets (CRC check) and re-publishes them via MQTT.
- Maintains uplink connectivity (Wi-Fi / LTE) to the backend broker.

### MQTT Ingest Service (`backend/mqtt-ingest/`)
- Subscribes to `satherd/telemetry/#`.
- Deserialises each `TelemetryPacket`, validates the CRC, and writes records to the database.

### Database Service (`backend/db/`)
- Stores telemetry records (device ID, timestamp, coordinates, battery, flags).
- Provides a query API consumed by the dashboard.

### Dashboard (`backend/dashboard/`)
- Web UI showing animal tracks on a map.
- Displays battery trends, storage usage, and fix quality per collar.

## Interface Layer (Firmware)

The collar firmware is decoupled from hardware drivers via four pure-virtual C++ interfaces in `lib/core/interfaces/`:

| Interface | Responsibility |
|-----------|---------------|
| `IGnss`   | GNSS acquisition (begin, waitForFix, getFix, end) |
| `IRadio`  | Radio transmit (begin, send, end) |
| `IStorage`| Persistent storage append (begin, write, usedBytes, totalBytes, end) |
| `IPower`  | Battery reading + sleep control (begin, batteryMillivolts, batteryPercent, sleep) |

Production driver objects are injected into `main.cpp`; mock objects are injected in unit tests. No `#ifdef` switches needed.

## Data Path

```
GNSS module → IGnss::getFix()
                    │
                    ▼
             TelemetryPacket
             (29 bytes + CRC)
                    │
           ┌────────┴────────┐
           ▼                 ▼
     IStorage::write()  IRadio::send()
     (local flash)      (LoRa air)
                              │
                        Gateway Node
                              │ MQTT
                        mqtt-ingest
                              │
                          Database
                              │
                         Dashboard
```

## Architectural Constraints

- Packet format is versioned (`version` field). Future fields must be added with a new version byte.
- `TelemetryPacket` is `#pragma pack(1)` and little-endian; any parser (gateway, backend) must match.
- Deep sleep causes a full MCU reset on ESP32; no heap or stack state survives between cycles.
- The firmware is driver-agnostic by design; real hardware drivers live outside `lib/core/`.
