CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -Os -ffunction-sections -fdata-sections -I. \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(EXTRA_CFLAGS)
LDFLAGS ?= -Tlink.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map
SOURCES = main.c 

ifeq ($(OS),Windows_NT)
  RM = cmd /C del /Q /F
else
  RM = rm -f
endif

build: firmware.bin

firmware.elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@
# $@ = target name (firmware.elf), so output is firmware.elf

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@
# $< = first prerequisite (firmware.elf), $@ = target (firmware.bin)
# so command becomes: objcopy -O binary firmware.elf firmware.bin

flash: firmware.bin
	st-flash --reset write $< 0x8000000
# $< = first prerequisite (firmware.bin), so flash firmware.bin

clean:
	$(RM) firmware.*
# RM is the remove command: rm -f on Linux, del /Q /F on Windows
# firmware.* deletes all build artifacts like firmware.elf, firmware.bin, etc.
