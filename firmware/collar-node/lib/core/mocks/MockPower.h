// MockPower.h - In-memory power mock for unit tests.
#pragma once

#include "IPower.h"
#include <stdint.h>

class MockPower : public IPower {
public:
    uint16_t   stubbedMillivolts = 3800;
    uint8_t    stubbedPercent    = 75;
    bool       sleptCalled       = false;
    uint32_t   lastSleepDurationMs = 0;
    PowerState lastSleepState    = PowerState::DeepSleep;

    bool     begin()             override { return true; }
    uint16_t batteryMillivolts() override { return stubbedMillivolts; }
    uint8_t  batteryPercent()    override { return stubbedPercent; }

    void sleep(uint32_t durationMs, PowerState state = PowerState::DeepSleep) override {
        sleptCalled         = true;
        lastSleepDurationMs = durationMs;
        lastSleepState      = state;
        // In tests we return immediately instead of halting the MCU.
    }
};
