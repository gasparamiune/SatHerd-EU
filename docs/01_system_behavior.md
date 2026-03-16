# System Behavior

## Collar Node – Nominal Wake Cycle

Each wake cycle runs once per `SLEEP_INTERVAL_S` (default 15 minutes):

```
[BOOT]   Power-on self-test, serial log initialised
  │
[PWR]    Read battery millivolts and state-of-charge
  │
[GNSS]   Power on GNSS module
         Wait up to GNSS_TIMEOUT_MS (2 min) for valid fix
         Power off GNSS module
  │
[BUILD]  Assemble TelemetryPacket (29 bytes)
         Compute CRC-16/CCITT
  │
[STORE]  Append serialised packet to flash storage log
  │
[RADIO]  Power on LoRa transceiver
         Transmit packet
         Power off transceiver
  │
[SLEEP]  Enter ESP32 deep sleep for SLEEP_INTERVAL_S seconds
         → CPU off, RTC timer armed
```

## Collar Node – Degraded Modes

| Condition | Behavior |
|-----------|----------|
| GNSS timeout (no fix) | Packet stored with `FLAG_GNSS_FIX_VALID = 0` and `lat/lon = 0.0`. Cycle continues normally. |
| Storage full / write error | Log warning to serial. Packet not persisted; transmission still attempted. |
| Radio transmit failure | Log warning. Packet already stored locally; will be retransmitted on next successful cycle (future: store-and-forward queue). |
| Low battery | Logged but no early-termination logic in v1; future versions will skip radio to conserve power. |

## Gateway Node – Nominal Operation

1. Listens continuously on the designated LoRa frequency and spreading factor.
2. On receipt of a valid packet (CRC check passes), forwards raw bytes over MQTT to the configured broker.
3. MQTT topic: `satherd/telemetry/<deviceId>`.

## Timing Assumptions

- Deep sleep duration: 15 min (configurable at compile time).
- GNSS cold start: ≤ 60 s under open sky; up to 2 min worst-case.
- Radio transmit window: < 1 s for a 29-byte payload at SF9 / 125 kHz BW.
- Total active time per cycle: ≤ 130 s (dominated by GNSS acquisition).
- Estimated duty cycle: ≤ 15 %.
