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

/* $Id: micro_fifo.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <inttypes.h>
#include "micro_fifo.h"

uint8_t MicroInIdx = 0;
uint8_t MicroOutIdx = 0;
uint8_t MicroFifoSize = 64;
uint8_t MicroBuffer[64];

/* Empty the buffer by clearing the indexes. */
void MicroFifoClear(void)
{
    MicroInIdx = 0;
    MicroOutIdx = 0;
}

/*  Add one data byte to the fifo buffer.
 *  param data : Data byte to add to the queue.
 *  return : Return M_FIFO_OK if the data has been added, M_FIFO_FULL if the buffer
 *  was full and the data couldn't be added.
 */
int8_t MicroFifoPut(uint8_t const data)
{
    if (MicroFifoLength() == MicroFifoSize)
        return M_FIFO_FULL;
    MicroBuffer[MicroInIdx++ & (MicroFifoSize-1)] = data;
    return M_FIFO_OK;
}

/*  Pop the oldest byte from the buffer.
 *  param data : pointer for storing the data read from the queue
 *  return : M_FIFO_OK if a value has been popped out at the pointer address. If
 *  the fifo is empty, M_FIFO_EMPTY is returned and the pointed data is left
 *  unchanged.
 */
int8_t MicroFifoGet(uint8_t *data)
{
    if (MicroOutIdx == MicroInIdx)
        return M_FIFO_EMPTY;
    *data = MicroBuffer[MicroOutIdx++ & (MicroFifoSize - 1)];
    return M_FIFO_OK;
}
