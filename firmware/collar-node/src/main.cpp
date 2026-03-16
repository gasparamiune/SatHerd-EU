/**
 * main.cpp – SatHerd-EU Collar Node firmware
 *
 * Operational cycle (executed on every wake from deep sleep):
 *   1. Boot & power-on self-test
 *   2. Read battery state
 *   3. Acquire GNSS fix (timeout = GNSS_TIMEOUT_MS)
 *   4. Build TelemetryPacket and append to flash storage
 *   5. Attempt radio transmission
 *   6. Enter deep sleep for SLEEP_INTERVAL_S seconds
 */

#include <Arduino.h>
#include "IGnss.h"
#include "IRadio.h"
#include "IStorage.h"
#include "IPower.h"
#include "TelemetryPacket.h"

// ---- Build-time configuration -----------------------------------------------

// How long to wait for a GNSS fix before giving up (milliseconds).
static constexpr uint32_t GNSS_TIMEOUT_MS  = 120'000UL;  // 2 minutes

// How long the collar sleeps between telemetry cycles (seconds).
static constexpr uint32_t SLEEP_INTERVAL_S = 15 * 60UL;  // 15 minutes

// ---- Hardware drivers (forward declarations) --------------------------------
//
// Replace these with real driver instances when hardware is available.
// The firmware uses interface pointers so the production driver and the
// unit-test mock are interchangeable without changing this file.

// TODO: replace with real driver objects once hardware layer is implemented.
// e.g. #include "Ublox8M.h"
//      static Ublox8M gnssDriver;
//      static IGnss* gnss = &gnssDriver;
//
// For now we declare the pointers and initialise them to nullptr.
// In production builds these must be wired up before setup() uses them.
static IGnss*    gnss    = nullptr;
static IRadio*   radio   = nullptr;
static IStorage* storage = nullptr;
static IPower*   power   = nullptr;

// ---- Helper: derive a stable 32-bit device ID from the ESP32 MAC -----------

static uint32_t getDeviceId() {
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    uint64_t mac = ESP.getEfuseMac();
    return (uint32_t)(mac & 0xFFFFFFFF) ^ (uint32_t)(mac >> 32);
#else
    return 0xDEADBEEFUL;  // Placeholder for non-ESP32 targets / tests
#endif
}

// ---- Main entry point -------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("\n[BOOT] SatHerd-EU collar-node v1");

    // Subsystem init --------------------------------------------------------
    if (power && !power->begin()) {
        Serial.println("[WARN] Power init failed");
    }
    if (storage && !storage->begin()) {
        Serial.println("[WARN] Storage init failed");
    }

    // Battery state ---------------------------------------------------------
    uint16_t battMv  = power ? power->batteryMillivolts() : 0;
    uint8_t  battPct = power ? power->batteryPercent()    : 0;
    Serial.printf("[PWR]  battery=%umV  %u%%\n", battMv, battPct);

    // GNSS acquisition ------------------------------------------------------
    IGnss::Fix fix = {};
    bool hasFix = false;

    if (gnss && gnss->begin()) {
        Serial.printf("[GNSS] waiting for fix (timeout %us)…\n", GNSS_TIMEOUT_MS / 1000);
        hasFix = gnss->waitForFix(GNSS_TIMEOUT_MS);
        fix    = gnss->getFix();
        gnss->end();

        if (hasFix && fix.valid) {
            Serial.printf("[GNSS] fix  lat=%.6f  lon=%.6f  alt=%.1fm  hdop=%.1f  sats=%u\n",
                          fix.lat, fix.lon, fix.altMeters, fix.hdop, fix.satellites);
        } else {
            Serial.println("[GNSS] no valid fix obtained");
        }
    } else {
        Serial.println("[WARN] GNSS init failed");
    }

    // Build telemetry packet ------------------------------------------------
    TelemetryPacket pkt = {};
    pkt.version      = TELEMETRY_PACKET_VERSION;
    pkt.deviceId     = getDeviceId();
    pkt.timestampUtc = (hasFix && fix.valid) ? fix.timestampUtc : 0;
    pkt.latitude     = fix.lat;
    pkt.longitude    = fix.lon;
    pkt.altitudeM    = (int16_t)fix.altMeters;
    pkt.hdop10       = (uint8_t)(fix.hdop * 10.0f);
    pkt.satellites   = fix.satellites;
    pkt.batteryMv    = battMv;
    pkt.batteryPct   = battPct;
    pkt.storageKb    = storage ? (uint16_t)(storage->usedBytes() / 1024) : 0;
    pkt.flags        = (hasFix && fix.valid) ? FLAG_GNSS_FIX_VALID : 0;
    pkt.crc          = telemetryCalcCrc(pkt);

    // Persist to flash ------------------------------------------------------
    uint8_t buf[TELEMETRY_PACKET_SIZE];
    telemetrySerialize(pkt, buf);

    if (storage && storage->write(buf, sizeof(buf))) {
        Serial.printf("[STORE] packet written  used=%uKB / %uKB\n",
                      storage->usedBytes() / 1024,
                      storage->totalBytes() / 1024);
        storage->end();
    } else {
        Serial.println("[WARN] storage write failed");
    }

    // Radio transmission ----------------------------------------------------
    if (radio && radio->begin()) {
        if (radio->send(buf, sizeof(buf))) {
            pkt.flags |= FLAG_RADIO_SENT;
            Serial.println("[RADIO] packet sent");
        } else {
            Serial.println("[WARN] radio send failed");
        }
        radio->end();
    } else {
        Serial.println("[WARN] radio init failed");
    }

    // Deep sleep ------------------------------------------------------------
    Serial.printf("[SLEEP] next cycle in %us\n", SLEEP_INTERVAL_S);
    Serial.flush();

    if (power) {
        power->sleep(SLEEP_INTERVAL_S * 1000UL, PowerState::DeepSleep);
    } else {
        // Fallback: use native ESP32 deep sleep when no IPower driver wired.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_INTERVAL_S * 1'000'000ULL);
        esp_deep_sleep_start();
#else
        delay(SLEEP_INTERVAL_S * 1000UL);
#endif
    }
}

void loop() {
    // Intentionally empty.
    // All logic executes in setup() once per deep-sleep wake cycle.
}
