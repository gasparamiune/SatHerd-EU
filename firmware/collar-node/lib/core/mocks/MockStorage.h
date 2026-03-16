// MockStorage.h - In-memory storage mock for unit tests.
#pragma once

#include "IStorage.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static constexpr size_t MOCK_STORAGE_CAPACITY = 4096;

class MockStorage : public IStorage {
public:
    uint8_t  buffer[MOCK_STORAGE_CAPACITY] = {};
    uint32_t cursor = 0;
    bool     writeShouldSucceed = true;

    bool begin() override { return true; }

    bool write(const uint8_t* data, size_t len) override {
        if (!writeShouldSucceed) return false;
        if (cursor + len > MOCK_STORAGE_CAPACITY) return false;
        memcpy(buffer + cursor, data, len);
        cursor += len;
        return true;
    }

    uint32_t usedBytes()  override { return cursor; }
    uint32_t totalBytes() override { return MOCK_STORAGE_CAPACITY; }
    void     end()        override {}
};
