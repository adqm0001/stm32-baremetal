#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))

#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

#define GPIO_START 0x40020000UL
#define GPIO_OFFSET 0x400UL

struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
};

#define GPIO(bank) ((struct gpio *) (GPIO_START + GPIO_OFFSET * (bank)))

struct rcc {
  volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
      RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR, AHB2ENR, AHB3ENR,
      RESERVED2, APB1ENR, APB2ENR, RESERVED3[2], AHB1LPENR, AHB2LPENR,
      AHB3LPENR, RESERVED4, APB1LPENR, APB2LPENR, RESERVED5[2], BDCR, CSR,
      RESERVED6[2], SSCGR, PLLI2SCFGR;
};

#define RCC ((struct rcc *) 0x40023800)

// Enum values are per datasheet: 0, 1, 2, 3
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  int n = PINNO(pin);
  RCC->AHB1ENR |= BIT(PINBANK(pin));        // Enable GPIO clock
  gpio->MODER &= ~(3U << (n * 2));          // Clear existing setting
  gpio->MODER |= (mode & 3) << (n * 2);     // Set new mode
}

static inline void gpio_set_af(uint16_t pin, uint8_t func) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  int n = PINNO(pin);
  gpio->AFR[n / 8] &= ~(15U << ((n % 8) * 4));
  gpio->AFR[n / 8] |= (func & 15U) << ((n % 8) * 4);
}

static inline void gpio_write(uint16_t pin, bool val) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  /* BSRR: lower 16 bits set pin high, upper 16 bits set pin low
   * Example we want pin 5 to be ON, we make bit 5 to 1 therefore val = 0
   * If we want pin 5 to be OFF, we make bit 21 to 0 therefore val = 16
  */
  gpio->BSRR = BIT(PINNO(pin)) << (val ? 0 : 16);
}

struct systick {
  volatile uint32_t CTRL, LOAD, VAL, CALIB;
};

#define SYSTICK ((struct systick *) 0xE000E010)
#define MAX_LOAD_VALUE 0xffffff

static inline void systick_init(uint32_t ticks) {
  if ((ticks - 1) > MAX_LOAD_VALUE) return;
  struct systick *systick = SYSTICK;
  systick->LOAD = ticks - 1;
  systick->VAL = 0;
  systick->CTRL = BIT(0) | BIT(1) | BIT(2); // Or 0x7 aka 0b111
}

void SysTick_Handler(void);

uint32_t millis(void);

bool timer_expired(uint32_t *last, uint32_t period);

static inline void delay_ms(uint32_t ms) {
  uint32_t initial_ms = millis();
  while (millis() - initial_ms < ms) {
    __asm__ __volatile__("nop");
  }
}

struct uart {
  volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
};

#define UART2 ((struct uart *) 0x40004400)

static inline void uart_init(void) {
  uint16_t tx = PIN('A', 2);
  uint16_t rx = PIN('A', 3);
  RCC->APB1ENR |= BIT(17); // Enable USART2 clock for UART communication
  gpio_set_mode(tx, GPIO_MODE_AF);
  gpio_set_af(tx, 7); // AF7 for USART2
  gpio_set_mode(rx, GPIO_MODE_AF);
  gpio_set_af(rx, 7);
  UART2->BRR = 16000000 / 115200;
  UART2->CR1 = 0;
  UART2->CR1 |= BIT(13) | BIT(3) | BIT(2); // enable UE, TE, RE
}

static inline int uart_read_ready(struct uart *uart) {
  return uart->SR & BIT(5);
}

static inline uint8_t uart_read_byte(struct uart *uart) {
  return (uint8_t)(uart->DR & 255);
}

static inline void spin(volatile uint32_t count) {
  while (count--) __asm__ __volatile__("nop");
}

static inline void uart_write_byte(struct uart *uart, uint8_t byte) {
  while ((uart->SR & BIT(7)) == 0) spin(1);
  uart->DR = byte;
}

static inline void uart_write_buf(struct uart *uart, const char *buf, uint32_t len) {
  while (len-- > 0) uart_write_byte(uart, *buf++);
}

int main(void);

#endif
