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

/* $Id: audio_fifo.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <inttypes.h>
#include "audio_fifo.h"

uint8_t AudioInIdx = 0;
uint8_t AudioOutIdx = 0;
uint8_t AudioFifoSize = 128;
uint8_t AudioBuffer[128];

/* Empty the buffer by clearing the indexes. */
void AudioFifoClear(void)
{
    AudioInIdx = 0;
    AudioOutIdx = 0;
}

/*  Add one data byte to the fifo buffer.
 *  param data : Data byte to add to the queue.
 *  return : Return A_FIFO_OK if the data has been added, A_FIFO_FULL if the buffer
 *  was full and the data couldn't be added.
 */
int8_t AudioFifoPut(uint8_t const data)
{
    if (AudioFifoLength() == AudioFifoSize)
        return A_FIFO_FULL;
    AudioBuffer[AudioInIdx++ & (AudioFifoSize-1)] = data;
    return A_FIFO_OK;
}

/*  Pop the oldest byte from the buffer.
 *  param data : pointer for storing the data read from the queue
 *  return : A_FIFO_OK if a value has been popped out at the pointer address. If
 *  the fifo is empty, A_FIFO_EMPTY is returned and the pointed data is left
 *  unchanged.
 */
int8_t AudioFifoGet(uint8_t *data)
{
    if (AudioOutIdx == AudioInIdx)
        return A_FIFO_EMPTY;
    *data = AudioBuffer[AudioOutIdx++ & (AudioFifoSize - 1)];
    return A_FIFO_OK;
}
