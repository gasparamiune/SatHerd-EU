// MockRadio.h - In-memory radio mock for unit tests.
#pragma once

#include "IRadio.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static constexpr size_t MOCK_RADIO_BUF_SIZE = 256;

class MockRadio : public IRadio {
public:
    uint8_t lastPayload[MOCK_RADIO_BUF_SIZE] = {};
    size_t  lastPayloadLen = 0;
    bool    sendShouldSucceed = true;
    int     sendCallCount = 0;

    bool begin() override { return true; }

    bool send(const uint8_t* data, size_t len) override {
        ++sendCallCount;
        if (sendShouldSucceed && len <= MOCK_RADIO_BUF_SIZE) {
            memcpy(lastPayload, data, len);
            lastPayloadLen = len;
            return true;
        }
        return false;
    }

    void end() override {}
};
