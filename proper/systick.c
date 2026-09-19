#include "hal.h"

static volatile uint32_t s_ticks;

void SysTick_Handler(void) {
  s_ticks++;
}

uint32_t millis(void) { return s_ticks; }

bool timer_expired(uint32_t *last, uint32_t period) {
  uint32_t now = millis();
  if (now - *last >= period) {
    *last = now;
    return true;
  }
  return false;
}

