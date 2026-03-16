// IStorage.h - Interface contract for telemetry storage operations.
#pragma once

#include <stdint.h>
#include <stddef.h>

class IStorage {
public:
    virtual ~IStorage() = default;

    // Mount / initialise the storage medium.
    // Returns true on success.
    virtual bool begin() = 0;

    // Append len bytes from data to persistent storage.
    // Returns true if all bytes were written successfully.
    virtual bool write(const uint8_t* data, size_t len) = 0;

    // Return the number of bytes currently used.
    virtual uint32_t usedBytes() = 0;

    // Return the total capacity in bytes.
    virtual uint32_t totalBytes() = 0;

    // Flush pending writes and unmount the storage medium.
    virtual void end() = 0;
};
