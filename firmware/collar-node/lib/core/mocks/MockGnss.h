// MockGnss.h - In-memory GNSS mock for unit tests.
#pragma once

#include "IGnss.h"

class MockGnss : public IGnss {
public:
    // Configurable canned fix returned by getFix().
    Fix cannedFix = {
        48.8566f,       // lat  – Paris, for deterministic tests
        2.3522f,        // lon
        42.0f,          // altMeters
        1.2f,           // hdop
        8,              // satellites
        1710000000UL,   // timestampUtc (2024-03-09T16:00:00Z)
        true            // valid
    };

    bool fixAvailable = true;   // Set false to simulate timeout

    bool begin()  override { return true; }
    bool waitForFix(uint32_t) override { return fixAvailable; }
    Fix  getFix()             override { return cannedFix; }
    void end()    override {}
};
