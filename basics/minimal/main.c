#include <stdint.h>

extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata;
// Function pointer to the address of the stack
extern void _estack(void);

int main(void) {
  return 0;
}

/*__attribute__((naked, noreturn)) specifies to gcc that the standard function's prologue and epilogue should not be created (naked)
 * and that the function does not return (noreturn)
 * we also have the function _reset which will be the reset handler and the second entry in the vector table
*/
__attribute__((noreturn)) void _reset(void) {
  /* The reason we move the data first before calling main is to prevent data from being garbage at the start of the program */

  // memset .bss to zero, and copy .data section to RAM region
  for (uint32_t *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  // Copy data from flash (LMA) to RAM (VMA)
  for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main(); // Call main()
  for (;;) (void) 0; // Infinite loop if main() returns;
}

/* __attribute__((section(".vectors"))) specifies to the compiler to place the vector table at the .vectors section
 * (linker will place the section at the correct address aka beginning of flash memory)
 * void (*const tab[16 + 91])(void) represents an array of const function pointers that take (void) and return void
 * We directly initialize tab[0] and tab[1] to _estack and _reset which is the address of the stack and the reset handler
*/
__attribute__((section(".vectors"))) void (*const tab[16 + 97])(void) = {
  _estack, _reset
};
