# Packet Format

## Version 1 – Wire Layout

All multi-byte fields are **little-endian**. The struct is `#pragma pack(1)` (no padding).

| Offset | Size (B) | Field          | Type     | Notes |
|--------|----------|----------------|----------|-------|
| 0      | 1        | `version`      | uint8    | Always `0x01` for this format |
| 1      | 4        | `deviceId`     | uint32   | Lower 4 bytes of ESP32 MAC XOR upper 4 bytes |
| 5      | 4        | `timestampUtc` | uint32   | UNIX epoch (seconds); `0` if no GNSS fix |
| 9      | 4        | `latitude`     | float32  | Decimal degrees; positive = North |
| 13     | 4        | `longitude`    | float32  | Decimal degrees; positive = East |
| 17     | 2        | `altitudeM`    | int16    | Metres above MSL |
| 19     | 1        | `hdop10`       | uint8    | HDOP × 10 (e.g. `12` → HDOP 1.2) |
| 20     | 1        | `satellites`   | uint8    | Number of satellites used in fix |
| 21     | 2        | `batteryMv`    | uint16   | Battery terminal voltage in millivolts |
| 23     | 1        | `batteryPct`   | uint8    | State-of-charge 0–100 % |
| 24     | 2        | `storageKb`    | uint16   | On-device flash used, in kilobytes |
| 26     | 1        | `flags`        | uint8    | See flags table below |
| 27     | 2        | `crc`          | uint16   | CRC-16/CCITT over bytes [0..26] |
| **Total** | **29** |              |          | |

## Flags Bitmask

| Bit | Mask   | Name               | Meaning |
|-----|--------|--------------------|---------|
| 0   | `0x01` | `FLAG_GNSS_FIX_VALID` | Set when the GNSS fix met minimum quality thresholds |
| 1   | `0x02` | `FLAG_RADIO_SENT`     | Set when the packet was successfully transmitted over LoRa |
| 2–7 | —      | _reserved_         | Must be zero in v1 |

## CRC Algorithm

**CRC-16/CCITT-FALSE**

- Polynomial: `0x1021`
- Initial value: `0xFFFF`
- Input/output reflection: none
- Coverage: bytes `[0 .. 26]` (27 bytes; excludes the 2-byte `crc` field itself)

## Versioning Strategy

- The `version` field occupies the first byte and must always be present.
- Parsers that encounter an unknown version **must** discard the packet rather than attempting to parse it.
- New fields may be appended in a new version only if the total size does not exceed the LoRa maximum payload for the configured SF/BW combination.

## Integrity Validation

1. Confirm `len >= 29`.
2. Read the `version` byte and reject if unsupported.
3. Compute CRC-16/CCITT over bytes `[0..26]`.
4. Compare with the stored `crc` field; reject if mismatch.

## Example Payload (hex)

```
01                    version = 1
EFBEADDE              deviceId = 0xDEADBEEF (LE)
004CE865              timestampUtc = 1710000000 (2024-03-09 16:00:00 UTC, LE)
6B9A1F42              latitude  = 48.8566°N  (IEEE-754 LE)
3B411640              longitude = 2.3522°E   (IEEE-754 LE)
2A00                  altitudeM = 42 m (LE)
0C                    hdop10 = 12 → HDOP 1.2
08                    satellites = 8
D80E                  batteryMv = 3800 mV (LE)
4B                    batteryPct = 75 %
0000                  storageKb = 0 KB
01                    flags = FLAG_GNSS_FIX_VALID
XXXX                  crc (CRC-16/CCITT, computed)
```
