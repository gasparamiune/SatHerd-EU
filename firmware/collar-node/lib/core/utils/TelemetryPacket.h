// TelemetryPacket.h - Telemetry packet definition, serialisation, and CRC utilities.
#pragma once

#include <stdint.h>
#include <stddef.h>

// ---- Packet format version ----
static constexpr uint8_t TELEMETRY_PACKET_VERSION = 1;

// ---- Flags bitmask definitions ----
static constexpr uint8_t FLAG_GNSS_FIX_VALID  = 0x01;  // Bit 0: GNSS fix was valid
static constexpr uint8_t FLAG_RADIO_SENT      = 0x02;  // Bit 1: packet was transmitted

// ---- Wire layout (little-endian, packed) ----
// Field          Offset  Size   Notes
// version            0     1   Always TELEMETRY_PACKET_VERSION
// deviceId           1     4   Lower 4 bytes of ESP32 MAC / unique ID
// timestampUtc       5     4   UNIX epoch seconds (0 if no fix)
// latitude           9     4   IEEE-754 float, degrees (+N / -S)
// longitude         13     4   IEEE-754 float, degrees (+E / -W)
// altitudeM         17     2   int16, metres above MSL
// hdop10            19     1   uint8, HDOP × 10  (range 0–255 → 0.0–25.5)
// satellites        20     1   uint8, satellites used
// batteryMv         21     2   uint16, millivolts
// batteryPct        22     1   uint8, percent 0–100  (NOTE: follows batteryMv at 21)
// storageKb         24     2   uint16, kilobytes used
// flags             26     1   See FLAG_* constants
// crc               27     2   CRC-16/CCITT over bytes [0..26]
// TOTAL             29 bytes

#pragma pack(push, 1)
struct TelemetryPacket {
    uint8_t  version;        // 1  – packet format version
    uint32_t deviceId;       // 4  – unique device identifier
    uint32_t timestampUtc;   // 4  – UNIX epoch (seconds)
    float    latitude;       // 4  – degrees
    float    longitude;      // 4  – degrees
    int16_t  altitudeM;      // 2  – metres above MSL
    uint8_t  hdop10;         // 1  – HDOP × 10
    uint8_t  satellites;     // 1  – satellites used
    uint16_t batteryMv;      // 2  – millivolts
    uint8_t  batteryPct;     // 1  – percent 0–100
    uint16_t storageKb;      // 2  – kilobytes used
    uint8_t  flags;          // 1  – FLAG_* bitmask
    uint16_t crc;            // 2  – CRC-16/CCITT (excl. this field)
};
#pragma pack(pop)

static constexpr size_t TELEMETRY_PACKET_SIZE = sizeof(TelemetryPacket);  // 29 bytes

// Compute CRC-16/CCITT over all packet bytes except the crc field.
uint16_t telemetryCalcCrc(const TelemetryPacket& pkt);

// Return true when pkt.crc matches the computed CRC.
bool telemetryValidateCrc(const TelemetryPacket& pkt);

// Serialise pkt into buf (caller must supply at least TELEMETRY_PACKET_SIZE bytes).
void telemetrySerialize(const TelemetryPacket& pkt, uint8_t* buf);

// Deserialise buf into out.  Returns false if len is too small or CRC fails.
bool telemetryDeserialize(const uint8_t* buf, size_t len, TelemetryPacket& out);
