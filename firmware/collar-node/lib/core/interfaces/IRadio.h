// IRadio.h - Interface contract for radio transport used by core logic.
#pragma once

#include <stdint.h>
#include <stddef.h>

class IRadio {
public:
    virtual ~IRadio() = default;

    // Power on and initialise the radio transceiver.
    // Returns true on success.
    virtual bool begin() = 0;

    // Transmit len bytes from data.
    // Returns true if the packet was accepted for transmission.
    virtual bool send(const uint8_t* data, size_t len) = 0;

    // Power down the radio transceiver.
    virtual void end() = 0;
};
