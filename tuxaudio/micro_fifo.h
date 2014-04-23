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

/* $Id: micro_fifo.h 2994 2008-12-03 13:20:41Z ks156 $ */

#ifndef _MICRO_FIFO_H_
#define _MICRO_FIFO_H_

#define MicroFifoLength() (uint8_t)(MicroInIdx - MicroOutIdx)

enum MICRO_FIFO_STATUS {M_FIFO_OK = 0, M_FIFO_FULL, M_FIFO_EMPTY};

extern void MicroFifoClear(void);
int8_t MicroFifoPut(uint8_t const data);
int8_t MicroFifoGet(uint8_t *data);

extern uint8_t MicroInIdx;
extern uint8_t MicroOutIdx;
extern uint8_t MicroFifoSize;
extern uint8_t MicroBuffer[];


static inline void MicroFifoPut_inl(uint8_t const data)
{
    if (MicroFifoLength() == MicroFifoSize)
	return;
    MicroBuffer[MicroInIdx++ & (MicroFifoSize - 1)] = data;
}

static inline void MicroFifoGet_inl(uint8_t *data)
{
    if (MicroOutIdx == MicroInIdx)
	return;
    *data = MicroBuffer[MicroOutIdx++ & (MicroFifoSize -1)];
}

#endif /* _MICRO_FIFO_H_ */
