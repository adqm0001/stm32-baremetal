#include "hal.h"

extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata;
extern void _estack(void);

__attribute__((noreturn)) void _reset(void) {
  for (uint32_t *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;
  main();
  for (;;) (void) 0;
}

__attribute__((section(".vectors"))) void (*const tab[16 + 97])(void) = {
    _estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler
};
