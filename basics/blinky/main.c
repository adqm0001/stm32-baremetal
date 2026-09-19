#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)
#define GPIO_START 0x40020000UL
#define GPIO_OFFSET 0x400UL

extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata;
extern void _estack(void);

struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
};

#define GPIO(bank) ((struct gpio *) (GPIO_START + GPIO_OFFSET * (bank)))

// Enum values are per datasheet: 0, 1, 2, 3
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

/* By default all peripherals disabled on STM32 to save power, so in order to enable a GPIO peripheral it should be enabled
 * via the RCC (Reset and Clock Control) unit. AHB1ENR (AHB1 peripheral clock enable register) is responsbile to turn GPIO banks on or off
*/
struct rcc {
  volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
      RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR, AHB2ENR, AHB3ENR,
      RESERVED2, APB1ENR, APB2ENR, RESERVED3[2], AHB1LPENR, AHB2LPENR,
      AHB3LPENR, RESERVED4, APB1LPENR, APB2LPENR, RESERVED5[2], BDCR, CSR,
      RESERVED6[2], SSCGR, PLLI2SCFGR;
};
#define RCC ((struct rcc *) 0x40023800)

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  struct gpio *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  RCC->AHB1ENR |= BIT(PINBANK(pin)); // Enable GPIO clock
  gpio->MODER &= ~(3U << (n * 2));         // Clear existing setting
  gpio->MODER |= (mode & 3) << (n * 2);    // Set new mode
}

static inline void gpio_write(uint16_t pin, bool val) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  /* BSRR: lower 16 bits set pin high, upper 16 bits set pin low
   * Example we want pin 5 to be ON, we make bit 5 to 1 therefore val = 0
   * If we want pin 5 to be OFF, we make bit 21 to 0 therefore val = 16
  */
  gpio->BSRR = BIT(PINNO(pin)) << (val ? 0 : 16);
}

static volatile uint32_t s_ticks;

void SysTick_Handler(void){
  s_ticks++;
}

struct systick {
  volatile uint32_t CTRL, LOAD, VAL, CALIB;
};

#define SYSTICK ((struct systick *) 0xE000E010)

#define MAX_LOAD_VALUE 0xffffff 

static inline void systick_init(uint32_t ticks){
  if ((ticks - 1) > MAX_LOAD_VALUE) return;
  struct systick *systick = SYSTICK;
  /* Enables the counter, Enables SysTick exception request and indicates the clock source
   * Bit 0 is the ENABLE bit where when set to 1 enables the counter
   * Bit 1 is the TICKINT bit where when set to 1, counting down to zero asserts the SysTick exception request (interrupt)
   * Bit 2 is the CLKSOURCE bit where when set to 1 indicates that the clock source is the processor clock
   * In our case we want all three of them set to 1 (0x7)
  */
  systick->LOAD = ticks - 1;
  systick->VAL = 0;
  systick->CTRL = BIT(0) | BIT(1) | BIT(2); // Or 0x7 aka 0b111
}

uint32_t millis(void) { return s_ticks; };

bool timer_expired(uint32_t *last, uint32_t period){
  uint32_t now = millis();
  if (now - *last >= period){
    *last = now;
    return true; 
  }
  return false;
}

static inline void delay_ms(uint32_t ms) {
  uint32_t initial_ms = s_ticks;
  while (s_ticks - initial_ms < ms){
    __asm__ __volatile__ ("nop");
  }
}

struct uart {
  volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR; 
};

#define UART2 ((struct uart *) 0x40004400)

void gpio_set_af(uint16_t pin, uint8_t func){
  struct gpio *gpio = GPIO(PINBANK(pin));
  int n = PINNO(pin);
  gpio->AFR[n / 8] &= ~(15U << ((n % 8) * 4)); // Clear bits of AFR
  gpio->AFR[n / 8] |= (func & 15U) << ((n % 8) * 4);
}

static inline void uart_init(){
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

static inline int uart_read_ready(struct uart *uart){
  return uart->SR & BIT(5);
}

static inline uint8_t uart_read_byte(struct uart *uart){
  return (uint8_t) uart->DR & 255;
}

static inline void spin(volatile uint32_t count) {
  while (count --) __asm__ __volatile__("nop");
}

static inline void uart_write_byte(struct uart *uart, uint8_t byte){
  while ((uart->SR & BIT(7)) == 0) spin(1);
  uart->DR = byte;
}

static inline void uart_write_buf(struct uart *uart, const char *buf, uint32_t len){
  while (len-- > 0) uart_write_byte(uart, *buf++);
}

int main(void) {
  systick_init(16000);
  uint16_t led = PIN('A', 5); // Green user led: PA5
  uart_init();
  uart_write_buf(UART2, "hello\r\n", 7);
  gpio_set_mode(led, GPIO_MODE_OUTPUT);
  for (;;) {
    gpio_write(led, true);
    delay_ms(1000);
    gpio_write(led, false);
    delay_ms(1000);
  }
}

__attribute__((noreturn)) void _reset(void) {
    for (uint32_t *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
    for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    main();
    for (;;) (void) 0;
}

__attribute__((section(".vectors"))) void (*const tab[16 + 97])(void) = {
    _estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler
};
