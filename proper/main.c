#include "hal.h"
#include <stdio.h>

int main(void) {
  systick_init(16000);
  uart_init();
  uint16_t led = PIN('A', 5);
  gpio_set_mode(led, GPIO_MODE_OUTPUT);
  for (;;) {
    gpio_write(led, true);
    printf("LED: 1, tick: %lu\r\n", (unsigned long) millis());
    delay_ms(1000);
    gpio_write(led, false);
    printf("LED: 0, tick: %lu\r\n", (unsigned long) millis());
    delay_ms(1000);
  }
}
