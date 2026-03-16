/**
 * test_sanity.cpp – Collar-node unit tests (PlatformIO / Unity framework)
 *
 * These tests run on the native host target (no hardware required) and
 * exercise core logic via mock implementations of every interface.
 */

#include <unity.h>

#include "TelemetryPacket.h"
#include "MockGnss.h"
#include "MockRadio.h"
#include "MockStorage.h"
#include "MockPower.h"

// ---- TelemetryPacket tests -------------------------------------------------

void test_packet_size_is_29_bytes() {
    TEST_ASSERT_EQUAL(29, (int)TELEMETRY_PACKET_SIZE);
}

void test_crc_roundtrip() {
    TelemetryPacket pkt = {};
    pkt.version      = TELEMETRY_PACKET_VERSION;
    pkt.deviceId     = 0xDEADBEEF;
    pkt.timestampUtc = 1710000000UL;
    pkt.latitude     = 48.8566f;
    pkt.longitude    = 2.3522f;
    pkt.altitudeM    = 42;
    pkt.hdop10       = 12;
    pkt.satellites   = 8;
    pkt.batteryMv    = 3800;
    pkt.batteryPct   = 75;
    pkt.storageKb    = 0;
    pkt.flags        = FLAG_GNSS_FIX_VALID;
    pkt.crc          = telemetryCalcCrc(pkt);

    TEST_ASSERT_TRUE(telemetryValidateCrc(pkt));
}

void test_crc_detects_corruption() {
    TelemetryPacket pkt = {};
    pkt.version  = TELEMETRY_PACKET_VERSION;
    pkt.deviceId = 0x12345678;
    pkt.crc      = telemetryCalcCrc(pkt);

    // Flip a byte in the payload.
    pkt.latitude = 99.0f;

    TEST_ASSERT_FALSE(telemetryValidateCrc(pkt));
}

void test_serialize_deserialize_roundtrip() {
    TelemetryPacket orig = {};
    orig.version      = TELEMETRY_PACKET_VERSION;
    orig.deviceId     = 0xCAFEBABE;
    orig.timestampUtc = 1710000000UL;
    orig.latitude     = -33.8688f;
    orig.longitude    = 151.2093f;
    orig.altitudeM    = 10;
    orig.hdop10       = 8;
    orig.satellites   = 12;
    orig.batteryMv    = 4100;
    orig.batteryPct   = 95;
    orig.storageKb    = 2;
    orig.flags        = FLAG_GNSS_FIX_VALID | FLAG_RADIO_SENT;
    orig.crc          = telemetryCalcCrc(orig);

    uint8_t buf[TELEMETRY_PACKET_SIZE];
    telemetrySerialize(orig, buf);

    TelemetryPacket parsed = {};
    bool ok = telemetryDeserialize(buf, sizeof(buf), parsed);

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL(orig.version,      parsed.version);
    TEST_ASSERT_EQUAL(orig.deviceId,     parsed.deviceId);
    TEST_ASSERT_EQUAL(orig.timestampUtc, parsed.timestampUtc);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, orig.latitude,  parsed.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, orig.longitude, parsed.longitude);
    TEST_ASSERT_EQUAL(orig.altitudeM,    parsed.altitudeM);
    TEST_ASSERT_EQUAL(orig.satellites,   parsed.satellites);
    TEST_ASSERT_EQUAL(orig.batteryMv,    parsed.batteryMv);
    TEST_ASSERT_EQUAL(orig.batteryPct,   parsed.batteryPct);
    TEST_ASSERT_EQUAL(orig.flags,        parsed.flags);
}

void test_deserialize_rejects_short_buffer() {
    uint8_t buf[4] = {0};
    TelemetryPacket pkt = {};
    bool ok = telemetryDeserialize(buf, sizeof(buf), pkt);
    TEST_ASSERT_FALSE(ok);
}

// ---- Mock tests ------------------------------------------------------------

void test_mock_gnss_returns_valid_fix() {
    MockGnss gnss;
    TEST_ASSERT_TRUE(gnss.begin());
    TEST_ASSERT_TRUE(gnss.waitForFix(1000));
    IGnss::Fix fix = gnss.getFix();
    TEST_ASSERT_TRUE(fix.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 48.8566f, fix.lat);
}

void test_mock_gnss_no_fix() {
    MockGnss gnss;
    gnss.fixAvailable = false;
    TEST_ASSERT_FALSE(gnss.waitForFix(1000));
}

void test_mock_radio_sends_payload() {
    MockRadio radio;
    TEST_ASSERT_TRUE(radio.begin());

    uint8_t data[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_TRUE(radio.send(data, sizeof(data)));
    TEST_ASSERT_EQUAL(1, radio.sendCallCount);
    TEST_ASSERT_EQUAL(sizeof(data), radio.lastPayloadLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, radio.lastPayload, sizeof(data));
}

void test_mock_radio_simulates_failure() {
    MockRadio radio;
    radio.sendShouldSucceed = false;
    uint8_t data[] = {0xAA};
    TEST_ASSERT_FALSE(radio.send(data, sizeof(data)));
}

void test_mock_storage_write_and_capacity() {
    MockStorage store;
    TEST_ASSERT_TRUE(store.begin());
    TEST_ASSERT_EQUAL(0u, store.usedBytes());
    TEST_ASSERT_EQUAL((uint32_t)MOCK_STORAGE_CAPACITY, store.totalBytes());

    uint8_t payload[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    TEST_ASSERT_TRUE(store.write(payload, sizeof(payload)));
    TEST_ASSERT_EQUAL(10u, store.usedBytes());
}

void test_mock_power_returns_battery_state() {
    MockPower pwr;
    pwr.stubbedMillivolts = 3700;
    pwr.stubbedPercent    = 60;
    TEST_ASSERT_TRUE(pwr.begin());
    TEST_ASSERT_EQUAL(3700u, pwr.batteryMillivolts());
    TEST_ASSERT_EQUAL(60u,   pwr.batteryPercent());
}

void test_mock_power_sleep_records_call() {
    MockPower pwr;
    TEST_ASSERT_FALSE(pwr.sleptCalled);
    pwr.sleep(5000, PowerState::DeepSleep);
    TEST_ASSERT_TRUE(pwr.sleptCalled);
    TEST_ASSERT_EQUAL(5000u, pwr.lastSleepDurationMs);
    TEST_ASSERT_EQUAL((uint8_t)PowerState::DeepSleep, (uint8_t)pwr.lastSleepState);
}

// ---- Test runner -----------------------------------------------------------

int main(int, char**) {
    UNITY_BEGIN();

    RUN_TEST(test_packet_size_is_29_bytes);
    RUN_TEST(test_crc_roundtrip);
    RUN_TEST(test_crc_detects_corruption);
    RUN_TEST(test_serialize_deserialize_roundtrip);
    RUN_TEST(test_deserialize_rejects_short_buffer);

    RUN_TEST(test_mock_gnss_returns_valid_fix);
    RUN_TEST(test_mock_gnss_no_fix);
    RUN_TEST(test_mock_radio_sends_payload);
    RUN_TEST(test_mock_radio_simulates_failure);
    RUN_TEST(test_mock_storage_write_and_capacity);
    RUN_TEST(test_mock_power_returns_battery_state);
    RUN_TEST(test_mock_power_sleep_records_call);

    return UNITY_END();
}
