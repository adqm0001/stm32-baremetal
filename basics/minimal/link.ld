/* Tells the linker the value of the entry point attribute (aid for the debugger) */
ENTRY(_reset);

MEMORY {
  /* r = read, w = write, x = execute */
  flash(rx)  : ORIGIN = 0x08000000, LENGTH = 512k /* rx (read and execute), read-only */
  sram(rwx) : ORIGIN = 0x20000000, LENGTH = 128k  /* remaining 64k in a separate address space, rwx (read, write, execute) read and write */
}

/* The reason the stack points to the end of ram is since the stack grows downward (heap grows up and starts after the data sections) */
_estack = ORIGIN(sram) + LENGTH(sram); /* stack points to the end of SRAM */

SECTIONS {
  /* In linker scripts * is a wildcard/placeholder, for example *(.vectors) means: for all object files, take the section named .vectors */
  /* > flash tells the linker where to place the collected sections in the final firmware */
  /* Keep tells the compiler to not discard the section even if there is no references to the tab or .vectors section */
  .vectors : {KEEP (*(.vectors)) } > flash
  /* In this case *(.text*) signifies to collect all object files with a prefix match and not exact match of .text- */
  .text    : { *(.text*) }         > flash
  /* Read only data subsections */
  .rodata  : { *(.rodata*) }       > flash

  /* data section (initialized global variables) */
  .data : {
    /* . is the current location counter; _sdata records the start address of .data in RAM (VMA = Virtual Memory Area) */
    _sdata = .;
    /* collect .first_data sections if any */
    *(.first_data)
    /* collect all normal intialized data: .data and .data.* sections, sorted alphabetically */
    *(.data SORT(.data*))
    /* _edata records the end address of .data in RAM (VMA) */
    _edata = .; 
    /* VMA (run address) is RAM, LMA (load address) is flash */
  } > sram AT > flash
  /* _sidata = load address (LMA) of .data, where initial values are stored in flash */
  _sidata = LOADADDR(.data);

  /* bss section (uninitialized global/static variables) */
  .bss : {
    /* _sbss = start of .bss in RAM */
    _sbss = .;
    /* collect .bss and .bss.* sections and COMMON (uninitialized globals) */
    *(.bss SORT(.bss.*) COMMON)
    /* _ebss = end of .bss in RAM */
    _ebss = .;
    /* .bss only lives in RAM; no initial values in flash, zeroed at startup */
  } > sram

  /* Align the location counter to 8 bytes, so heap can start aligned */
  . = ALIGN(8);
  /* _end marks the end of the used RAM (basically move address foward until the address is divisible by 8), often used as start of heap */
  _end = .;
}


