// IPower.h - Interface contract for power state and control operations.
#pragma once

#include <stdint.h>

enum class PowerState : uint8_t {
    Active     = 0,  // Full CPU and peripheral power
    LightSleep = 1,  // CPU halted, RAM retained, fast wake
    DeepSleep  = 2,  // CPU off, only RTC active, slowest wake
};

class IPower {
public:
    virtual ~IPower() = default;

    // Initialise power management hardware (ADC, gauges, regulators).
    // Returns true on success.
    virtual bool begin() = 0;

    // Return the battery terminal voltage in millivolts.
    virtual uint16_t batteryMillivolts() = 0;

    // Return the estimated state-of-charge in percent (0–100).
    virtual uint8_t batteryPercent() = 0;

    // Enter a low-power sleep state for durationMs milliseconds.
    // Deep sleep does not return; the MCU resets on wake.
    virtual void sleep(uint32_t durationMs, PowerState state = PowerState::DeepSleep) = 0;
};
