# stm32-baremetal

Bare-metal firmware for an STM32F4 (ARM Cortex-M4) written from scratch in C, with no vendor HAL, CMSIS, or IDE. Everything from the reset vector up is hand-written.

## What's inside

| Path | What it does |
| --- | --- |
| `basics/minimal` | Smallest possible image: a vector table and reset handler, with notes on sections, VMA/LMA and alignment |
| `basics/blinky` | Blinks the on-board LED (PA5) by writing GPIO registers directly |
| `proper/` | A cleaner layout split into a register-level HAL, startup code, syscalls and a SysTick timer |

`proper/` covers:
- **Startup:** `startup.c` defines the vector table and a reset handler that zeroes `.bss`, copies `.data` from flash to SRAM, then calls `main`
- **Linker script:** `link.ld` maps 512 KB flash and 128 KB SRAM and places the vector table first
- **GPIO and RCC:** `hal.h` drives peripherals through memory-mapped register structs
- **UART:** USART2 at 115,200 baud. `syscalls.c` implements `_write` and `_sbrk`, so `printf` from newlib-nano goes out over the serial port
- **SysTick:** a 1 ms tick interrupt powering `millis()`, `delay_ms()` and non-blocking timers

## Build and flash

Requires `arm-none-eabi-gcc` and a flashing tool (e.g. `st-flash`).

```bash
cd proper
make build                                  # produces firmware.elf and firmware.bin
st-flash --reset write firmware.bin 0x8000000
```

Open a serial terminal at 115,200 baud to see the LED state and tick count printed every second.

## Why

To understand what actually happens between power-on and `main()` on a microcontroller, without any framework hiding it.
