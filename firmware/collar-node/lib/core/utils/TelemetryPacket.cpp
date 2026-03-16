// TelemetryPacket.cpp - Telemetry packet serialisation and CRC-16/CCITT utilities.

#include "TelemetryPacket.h"
#include <string.h>

// CRC-16/CCITT-FALSE polynomial (0x1021), initial value 0xFFFF.
static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

uint16_t telemetryCalcCrc(const TelemetryPacket& pkt) {
    // Cover all bytes of the struct except the trailing crc field (2 bytes).
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&pkt);
    return crc16(raw, TELEMETRY_PACKET_SIZE - sizeof(pkt.crc));
}

bool telemetryValidateCrc(const TelemetryPacket& pkt) {
    return pkt.crc == telemetryCalcCrc(pkt);
}

void telemetrySerialize(const TelemetryPacket& pkt, uint8_t* buf) {
    memcpy(buf, &pkt, TELEMETRY_PACKET_SIZE);
}

bool telemetryDeserialize(const uint8_t* buf, size_t len, TelemetryPacket& out) {
    if (len < TELEMETRY_PACKET_SIZE) {
        return false;
    }
    memcpy(&out, buf, TELEMETRY_PACKET_SIZE);
    return telemetryValidateCrc(out);
}
