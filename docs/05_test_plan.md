# Test Plan

## Scope

The test suite covers three layers:

1. **Unit tests** – run on the host machine (no hardware) via PlatformIO `native` env.
2. **Integration tests** – run on real ESP32 hardware with GNSS/radio/storage attached.
3. **Hardware-in-the-Loop (HIL)** – end-to-end collar → gateway → backend → dashboard.

## Unit Tests (`env:native`)

Run with: `pio test -e native`

### TelemetryPacket (`test_sanity.cpp`)

| Test ID | Description | Pass Criterion |
|---------|-------------|----------------|
| TP-01 | `sizeof(TelemetryPacket) == 29` | Compile-time and runtime assertion pass |
| TP-02 | CRC roundtrip | `telemetryValidateCrc(pkt)` returns `true` after `telemetryCalcCrc` |
| TP-03 | CRC detects corruption | Mutating `latitude` causes `telemetryValidateCrc` to return `false` |
| TP-04 | Serialize / deserialize roundtrip | All fields survive binary round-trip; `telemetryDeserialize` returns `true` |
| TP-05 | Deserialize rejects short buffer | Returns `false` when buffer length < 29 |

### Mock Interfaces

| Test ID | Description | Pass Criterion |
|---------|-------------|----------------|
| MK-01 | `MockGnss` returns valid fix | `fix.valid == true`, `lat ≈ 48.8566` |
| MK-02 | `MockGnss` simulates no-fix | `waitForFix` returns `false` when `fixAvailable = false` |
| MK-03 | `MockRadio` records sent payload | `lastPayload` matches sent bytes; `sendCallCount == 1` |
| MK-04 | `MockRadio` simulates failure | `send` returns `false` when `sendShouldSucceed = false` |
| MK-05 | `MockStorage` tracks capacity | `usedBytes` increments after `write` |
| MK-06 | `MockPower` returns battery state | Returns configured `stubbedMillivolts` and `stubbedPercent` |
| MK-07 | `MockPower` records sleep call | `sleptCalled == true` with correct duration and state |

## Integration Tests (ESP32 hardware required)

> Not yet automated. Manual procedure until HIL rig is available.

| Test ID | Description | Pass Criterion |
|---------|-------------|----------------|
| INT-01 | GNSS cold-start fix | Valid fix obtained within 120 s under open sky |
| INT-02 | Packet stored to flash | Byte-for-byte match on readback; CRC valid |
| INT-03 | LoRa packet received by gateway | Gateway RSSI > −120 dBm at 100 m LOS |
| INT-04 | Deep sleep current | INA219 measures < 0.05 mA during sleep phase |
| INT-05 | Full cycle time | Total active time < 130 s per 15-min cycle |

## Hardware-in-the-Loop (HIL)

> Planned for v2 milestone once gateway firmware is complete.

| Test ID | Description | Pass Criterion |
|---------|-------------|----------------|
| HIL-01 | Collar → gateway → MQTT → database | Record visible in DB within 10 s of transmission |
| HIL-02 | Dashboard displays track | GPS coordinates render correctly on map |
| HIL-03 | Multi-collar disambiguation | Two collars tracked simultaneously without data mixing |

## Acceptance Criteria Summary

- All unit tests pass on `pio test -e native`.
- No compiler warnings at `-Wall -Wextra` on both `esp32dev` and `native` environments.
- Battery life ≥ 30 days on a 3000 mAh cell under field conditions.
- Packet loss rate ≤ 5 % at 500 m LOS with default LoRa settings.
