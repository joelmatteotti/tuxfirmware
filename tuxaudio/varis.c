/*
 * TUXAUDIO - Firmware for the 'audio' CPU of tuxdroid
 * Copyright (C) 2007 C2ME S.A. <tuxdroid@c2me.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: varis.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <avr/io.h>
#include "fifo.h"

// PWM Variable
unsigned char sampling_pwm = 0x04;

/* Set when sleep should be entered. */
uint8_t sleep_f = 0;

// SPI Variable
volatile unsigned char spi_enable = 1;
typedef enum
{ HEADERS, GET_SOUND_FIFO, PUT_COMMAND, DUMMY } STM_SPI_MASTER_SLAVE_TYPE;
volatile STM_SPI_MASTER_SLAVE_TYPE spi_slave;
typedef enum
{ HEADERM, PUT_SOUND_FIFO, READ_COMMAND } STM_SPI_SLAVE_MASTER_TYPE;
volatile STM_SPI_SLAVE_MASTER_TYPE spi_master;
volatile unsigned char spi_headerb;
volatile unsigned char spi_master_config;
volatile unsigned char spi_count;
volatile unsigned char spi_lenght_data;
volatile unsigned char spi_ready = 0;
volatile uint8_t rf_txe = 0;
volatile uint8_t spi_commandTX[5];
unsigned char spi_commandRX[5];
unsigned char commandRX = 0;
uint8_t rf_data_sent_ack;


// Flash programming
uint8_t eraseFlag = 0;
volatile unsigned char programmingFlash = 0;
volatile uint8_t numSound;

// Flash Variables
volatile unsigned char flashPlay = 0;
volatile unsigned char ad[6];
volatile unsigned char audioLevel;
uint8_t soundToPlay;

unsigned char audio_level, battery_level;

volatile unsigned char spiCommandFlasg = 0;

uint16_t frame_without_sound = 0;
uint8_t sound_played = 0;
uint8_t last_block = 0;

// General flags
uint8_t write_toc = 0;
