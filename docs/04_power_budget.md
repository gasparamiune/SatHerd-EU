# Power Budget

## Target Platform

- MCU: ESP32 (dual-core Xtensa LX6, 240 MHz)
- Battery: 3.7 V Li-Po, 3000 mAh nominal
- Sleep interval: 15 minutes per cycle

## Current Draw Estimates (per subsystem)

| Phase | Subsystem | Current | Duration | Energy (mWh) |
|-------|-----------|---------|----------|--------------|
| Boot + init | ESP32 active | ~160 mA | 0.5 s | 0.087 |
| GNSS acquisition | u-blox M8 + ESP32 | ~80 mA | 60 s (avg) | 1.600 |
| Packet build + store | ESP32 + SPI flash | ~60 mA | 1 s | 0.017 |
| LoRa transmit | SX1276 TX | ~120 mA | 0.5 s | 0.017 |
| Deep sleep | RTC only | ~0.01 mA | 838 s | 0.035 |
| **Cycle total** | | | **900 s** | **~1.76 mWh** |

> All figures are estimates based on datasheet typical values at 3.7 V.
> Actual consumption depends on GNSS cold/warm start, RF path loss, and PCB design.

## Battery Life Estimate

```
Battery capacity:      3000 mAh × 3.7 V = 11100 mWh
Usable capacity (80%): 8880 mWh
Energy per cycle:      ~1.76 mWh
Cycles per charge:     8880 / 1.76 ≈ 5045 cycles
Cycle period:          15 min
Estimated runtime:     5045 × 15 min ≈ 52 days
```

## Optimisation Opportunities

| Opportunity | Potential saving |
|-------------|----------------|
| Disable Wi-Fi / Bluetooth in firmware | −10 mA active |
| Use GNSS power-save / backup mode (warm start) | Reduce fix time to ~5 s → save ~1.3 mWh/cycle |
| Lower LoRa spreading factor when near gateway | Shorter TX → save ~0.01 mWh/cycle |
| Increase sleep interval for stationary animals | Linear improvement |
| Dynamic sleep interval (motion-triggered) | Up to 10× improvement when animal is stationary |

## Critical Battery Thresholds

| Threshold | Action |
|-----------|--------|
| < 3600 mV (~20 %) | Log warning in telemetry flags |
| < 3400 mV (~10 %) | Future: skip radio TX to preserve data logging |
| < 3200 mV (~5 %)  | Future: enter ultra-low-power hibernation |
