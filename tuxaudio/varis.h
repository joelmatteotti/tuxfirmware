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

/* $Id: varis.h 2994 2008-12-03 13:20:41Z ks156 $ */

#ifndef VARIS_H
#define VARIS_H

#include "fifo.h"

// PWM Variable
extern unsigned char sampling_pwm;

extern uint8_t sleep_f;

// SPI Variable
extern volatile unsigned char spi_enable;
typedef enum
{ HEADERS, GET_SOUND_FIFO, PUT_COMMAND, DUMMY } STM_SPI_MASTER_SLAVE_TYPE;
extern volatile STM_SPI_MASTER_SLAVE_TYPE spi_slave;
typedef enum
{ HEADERM, PUT_SOUND_FIFO, READ_COMMAND } STM_SPI_SLAVE_MASTER_TYPE;
extern volatile STM_SPI_SLAVE_MASTER_TYPE spi_master;
extern volatile unsigned char spi_headerb;
extern volatile unsigned char spi_master_config;
extern volatile unsigned char spi_count;
extern volatile unsigned char spi_lenght_data;
extern volatile unsigned char spi_ready;
extern volatile uint8_t rf_txe;
extern uint8_t spi_commandTX[5];
extern unsigned char spi_commandRX[5];
extern unsigned char commandRX;
extern uint8_t rf_data_sent_ack;

#define RF_DATA_SENT_FREE       0x00
#define RF_DATA_SENT_BUSY       0x01
#define RF_DATA_SENT_ACKED      0x02
#define RF_DATA_SENT_NACKED     0x03
#define RF_DATA_SENT_DROPPED    0x80


// Flash programming

extern uint8_t eraseFlag;
extern volatile unsigned char programmingFlash;
extern volatile uint8_t numSound;

// Flash Variables
extern volatile unsigned char flashPlay;
extern volatile unsigned char ad[6];
extern volatile unsigned char audioLevel;
extern uint8_t soundToPlay;

extern unsigned char audio_level, battery_level;

extern volatile unsigned char testAudio;

extern volatile unsigned char spiCommandFlasg;

extern uint16_t frame_without_sound;
extern uint8_t sound_played;
extern uint8_t last_block;

// General flags
extern uint8_t write_toc;
#endif
