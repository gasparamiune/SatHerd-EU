// IGnss.h - Interface contract for GNSS operations used by core logic.
#pragma once

#include <stdint.h>

class IGnss {
public:
    struct Fix {
        float    lat;           // Latitude in decimal degrees (+N, -S)
        float    lon;           // Longitude in decimal degrees (+E, -W)
        float    altMeters;     // Altitude above MSL in metres
        float    hdop;          // Horizontal dilution of precision
        uint8_t  satellites;    // Number of satellites used in fix
        uint32_t timestampUtc;  // UNIX epoch (seconds) from GNSS
        bool     valid;         // true when fix meets minimum quality
    };

    virtual ~IGnss() = default;

    // Power on and initialise the GNSS module.
    // Returns true on success.
    virtual bool begin() = 0;

    // Block until a valid fix is obtained or timeoutMs elapses.
    // Returns true if a valid fix was obtained within the timeout.
    virtual bool waitForFix(uint32_t timeoutMs) = 0;

    // Return the most recently obtained fix.  Valid flag reflects quality.
    virtual Fix getFix() = 0;

    // Power down the GNSS module to save energy.
    virtual void end() = 0;
};
