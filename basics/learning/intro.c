#include <stdint.h>

struct gpio {
  // Represents all the GPIO port register, each field is a 32-bit register mapped to hardware addresses
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
};

// Starting address of GPIO peripherals (located in memory mapped i/o) 
#define GPIO_START 0x40020000UL

// GPIO bank offset
#define GPIO_BANK_OFFSET 0x400UL

// Define GPIO bank (a GPIO peripheral)  
#define GPIO(bank) ((struct gpio *) (GPIO_START + GPIO_BANK_OFFSET * (bank)))

/*
 * -- Human readable way of defining pin --
 * Example:
 * bank = B, 'B'-'A' = 1, 00000001 00000000 
 * num = 3, 00000000 00000011
 * bank | num = 00000001 00000011
*/
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))

/*
 * -- Human readable way of getting pin number of a pin--
 * Example:
 * pin = B2, 00000001 00000010
 * 255 = 00000000 11111111
 * pin & 255 = 00000000 00000010 (correctly leaves only the pin number and we can put this in a uint8_t
*/
#define PINNO(pin) (pin & 255)

/*
 * -- Human readable way of getting bank of a pin --
 * Example:
 * pin = B2, 00000001 00000010
 * pin >> 8 = 00000000 00000001 (correctly leaves only the bank number and we can put this in a uint8_t
*/
#define PINBANK(pin) (pin >> 8)

// Enum values are 0, 1, 2, 3 (these values are given to pins to state which mode they should behave in)
enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};

static inline void gpio_set_mode(uint8_t pin, uint8_t mode) {
  // GPIO bank adress using pin 
  struct gpio *gpio = GPIO(PINBANK(pin));

  // GPIO pin number using pin
  uint8_t n = PINNO(pin);

  /*
   * -- Clears exisiting setting on pin --
   * Explanation:
   * pin number (n) = 2
   * 3U = 00000011
   * Since 2 bits per pin we do to find position, n * 2 = 4
   * Shift to correct position (3U << (n * 2)) = 00110000
   * Invert and AND gate to reset and leave other numbers unaffected = 11001111
  */
  gpio->MODER &= ~(3U << (pin * 2));
  /*
   * -- Set new mode --
   * pin number (n) = 2, mode = 1 (output)
   * Since we know the mode is max 2 bits we can zero out all other bits by doing: (mode & 3) = 00000001 & 00000011 = 00000001
   * We now shift the mode into the right pin position, (mode & 3) << (n * 2)
   * Use OR gate with the actual MODER to only affect the pin bits
  */
  gpio->MODER |= (mode & 3) << (n * 2);
  return;
}

int main()
{
  uint16_t pin = PIN('A', 3); // Pin A3
  gpio_set_mode(pin, GPIO_MODE_OUTPUT); // Set to output
  return 0;
}

