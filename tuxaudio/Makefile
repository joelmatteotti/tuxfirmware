# Makefile for the 'audio' CPU of Tux Droid

## Configuration Flags
MIC_GAIN = 0 # values can be 0, 6 or 12 (which is the gain in dB)

## General Flags
PROJECT = tuxaudio
MCU = atmega88
FORMAT = ihex
TARGET = tuxaudio.elf
BOOTLOADER = tuxaudio_bl.elf
OPT = s

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude

# Name of this Makefile (used for "make depend").
MAKEFILE = Makefile

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

# Place -D or -U options here
CDEFS = -DF_CPU=8000000UL -DMIC_GAIN=$(MIC_GAIN)

# Place -I options here
CINCS =

# Compile options common for all C compilation units.
CDEBUG = -g
CWARN = -Wall -Wstrict-prototypes
CTUNING = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS = $(COMMON) $(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CWARN) $(CSTANDARD) $(CEXTRA)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gstabs

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,--section-start=.version=0x1DF0 -Wl,-Map=tuxaudio.map tuxaudio.ld

## Linker flags for bootloader
BL_LDFLAGS = $(COMMON)
BL_LDFLAGS +=  -Wl,--section-start=.text=0x1E00  -Wl,-Map=tuxaudio_bl.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0


## Objects that must be built in order to link
OBJECTS = init.o main.o varis.o fifo.o spi.o AT26F004.o flash.o communication.o parser.o misc.o i2c.o config.o audio_fifo.o micro_fifo.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: svnrev.h $(TARGET) tuxaudio.hex tuxaudio.eep tuxaudio.lss size
boot: $(BOOTLOADER) tuxaudio_bl.hex tuxaudio_bl.lss bl_size

## Compile
init.o: init.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

varis.o: varis.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

fifo.o: fifo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

spi.o: spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

AT26F004.o: AT26F004.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

flash.o: flash.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

communication.o: communication.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

parser.o: parser.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

misc.o: misc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

i2c.o: i2c.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

config.o: config.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

bootloader.o: bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

audio_fifo.o: audio_fifo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

micro_fifo.o: micro_fifo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

$(BOOTLOADER): bootloader.o
	 $(CC) $(BL_LDFLAGS) bootloader.o -o $(BOOTLOADER)

%.hex: %.elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: %.elf
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: %.elf
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

bl_size: ${BOOTLOADER}
	@echo
	@avr-size ${BOOTLOADER}

## Generate SVN info
#  We need to change the status each time a file changes, thus so many
#  dependencies
svnrev.h: $(CSOURCE) $(HEADERS)

ifdef windir
	SubWCRev . svnrev.tmpl.h svnrev.h
else
	svnwcrev . svnrev.tmpl.h svnrev.h
endif

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) svnrev.h tuxaudio.elf dep tuxaudio.hex tuxaudio.eep tuxaudio.lss tuxaudio.map tuxaudio_bl.o tuxaudio_bl.hex tuxaudio_bl.lss tuxaudio_bl.map tuxaudio_bl.elf


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

## Generate doxygen documentation
.PHONY: doc
doc: svnrev.h
	@./doc/builddoc.sh


# Programming
prog: $(PROJECT).hex
	tuxup $(PROJECT).hex $(PROJECT).eep

progisp: $(PROJECT).hex $(PROJECT).eep
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -e -U flash:w:$(PROJECT).hex -U eeprom:w:$(PROJECT).eep

dwen:
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -U hfuse:w:0x9D:m

nodwen:
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -U hfuse:w:0xDD:m

debug:
	avarice -P $(MCU) -2 -w -j usb -p -f $(PROJECT).elf :4242

progisp_bl: $(PROJECT).hex $(PROJECT).eep $(PROJECT)_bl.hex
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -e -U flash:w:$(PROJECT)_bl.hex -U eeprom:w:$(PROJECT).eep -D -U flash:w:$(PROJECT).hex

progstk: $(PROJECT).hex $(PROJECT).eep
	$(AVRDUDE) -p $(MCU) -c stk500v2 -e -U flash:w:$(PROJECT).hex -U eeprom:w:$(PROJECT).eep

progstk_bl: $(PROJECT).hex $(PROJECT).eep $(PROJECT)_bl.hex
	$(AVRDUDE) -p $(MCU) -c stk500v2  -e -U flash:w:$(PROJECT)_bl.hex -U eeprom:w:$(PROJECT).eep -D -U flash:w:$(PROJECT).hex
