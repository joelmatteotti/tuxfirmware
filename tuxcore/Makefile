# Makefile - GNU Makefile for TUXCORE
#
# TUXCORE - Firmware for the 'core' CPU of tuxdroid
# Copyright (C) 2007 C2ME S.A. <tuxdroid@c2me.be>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# $Id: Makefile 2466 2008-10-27 03:25:39Z ks156 $

## General Flags
PROJECT = tuxcore
MCU = atmega88
FORMAT = ihex
TARGET = tuxcore.elf
BOOTLOADER = tuxcore_bl.elf
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
CDEFS = -DF_CPU=8000000UL

# Place -I options here
CINCS =

# Compile options common for all C compilation units.
ifdef windir
CDEBUG = -gdwarf-2
else
CDEBUG = -gstabs
endif
CWARN = -Wall -Wstrict-prototypes
CTUNING = -finline-limit=10 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS = $(COMMON) $(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CWARN) \
	 $(CSTANDARD) $(CEXTRA) $(CTUNING)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gstabs

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,--section-start=.version=0x1DF0 -Wl,-Map=tuxcore.map tuxcore.ld

## Linker flags for bootloader
BL_LDFLAGS = $(COMMON)
BL_LDFLAGS +=  -Wl,--section-start=.text=0x1E00 -Wl,-Map=tuxcore_bl.map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0

## All source and header files
CSOURCE = $(wildcard *.c)
HEADERS = $(wildcard *.h)

## Objects that must be built in order to link
OBJECTS = main.o adc.o sensors.o motors.o global.o led.o communication.o \
	  i2c.o fifo.o ir.o parser.o config.o standalone.o

## Build
all: svnrev.h $(TARGET) tuxcore.hex tuxcore.eep tuxcore.lss size
boot: $(BOOTLOADER) tuxcore_bl.hex tuxcore_bl.lss bl_size

## Compile
main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

adc.o: adc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sensors.o: sensors.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

motors.o: motors.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

global.o: global.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

led.o: led.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

communication.o: communication.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

i2c.o: i2c.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

fifo.o: fifo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ir.o: ir.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

bootloader.o: bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

parser.o: parser.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

config.o: config.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

standalone.o: standalone.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

## Generate SVN info
#  We need to change the status each time a file changes, thus so many
#  dependencies
svnrev.h: $(CSOURCE) $(HEADERS)

ifdef windir
	SubWCRev . svnrev.tmpl.h svnrev.h
else
	svnwcrev . svnrev.tmpl.h svnrev.h
endif

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) \
	     -o $(TARGET)

$(BOOTLOADER): bootloader.o
	 $(CC) $(BL_LDFLAGS) bootloader.o -o $(BOOTLOADER)

%.hex: %.elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: %.elf common/config.h
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: %.elf
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
ifdef windir
	@avr-size -C --mcu=${MCU} ${TARGET}
else
	@avr-size ${TARGET}
endif

bl_size: ${BOOTLOADER}
	@echo
ifdef windir
	@avr-size -C --mcu=${MCU} ${BOOTLOADER}
else
	@avr-size ${BOOTLOADER}
endif

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) svnrev.h tuxcore.elf dep tuxcore.hex tuxcore.eep \
	    tuxcore.lss tuxcore.map bootloader.o tuxcore_bl.hex tuxcore_bl.lss \
	    tuxcore_bl.map tuxcore_bl.elf svnrev.h dep


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

## Generate doxygen documentation
.PHONY: doc
doc: svnrev.h
ifdef windir
	# XXX add windows version
else
	@./doc/builddoc.sh
endif

# Programming
prog: $(PROJECT).hex
	tuxup $(PROJECT).hex $(PROJECT).eep

progisp: $(PROJECT).hex $(PROJECT).eep
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -e \
	    -U flash:w:$(PROJECT).hex -U eeprom:w:$(PROJECT).eep

progisp_bl: $(PROJECT).hex $(PROJECT).eep $(PROJECT)_bl.hex
	$(AVRDUDE) -p $(MCU) -c jtag2isp -P usb -B 10 -e \
	    -U flash:w:$(PROJECT)_bl.hex -U eeprom:w:$(PROJECT).eep -D \
	    -U flash:w:$(PROJECT).hex

progstk: $(PROJECT).hex $(PROJECT).eep
	$(AVRDUDE) -p $(MCU) -c stk500v2 -e -U flash:w:$(PROJECT).hex \
	    -U eeprom:w:$(PROJECT).eep

progstk_bl: $(PROJECT).hex $(PROJECT).eep $(PROJECT)_bl.hex
	$(AVRDUDE) -p $(MCU) -c stk500v2 -e -U flash:w:$(PROJECT)_bl.hex \
	    -U eeprom:w:$(PROJECT).eep -D -U flash:w:$(PROJECT).hex
