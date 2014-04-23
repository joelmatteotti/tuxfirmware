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

/* $Id: audio_fifo.h 2994 2008-12-03 13:20:41Z ks156 $ */

#ifndef _AUDIO_FIFO_H_
#define _AUDIO_FIFO_H_

#define AudioFifoLength() (uint8_t)(AudioInIdx - AudioOutIdx)

enum AUDIO_FIFO_STATUS {A_FIFO_OK = 0, A_FIFO_FULL, A_FIFO_EMPTY};

extern void AudioFifoClear(void);
int8_t AudioFifoPut(uint8_t const data);
int8_t AudioFifoGet(uint8_t *data);

extern uint8_t AudioInIdx;
extern uint8_t AudioOutIdx;
extern uint8_t AudioFifoSize;
extern uint8_t AudioBuffer[];


static inline void AudioFifoPut_inl(uint8_t const data)
{
    if (AudioFifoLength() == AudioFifoSize)
	return;
    AudioBuffer[AudioInIdx++ & (AudioFifoSize - 1)] = data;
}

static inline void AudioFifoGet_inl(uint8_t *data)
{
    if (AudioOutIdx == AudioInIdx)
	return;
    *data = AudioBuffer[AudioOutIdx++ & (AudioFifoSize -1)];
}

#endif /* _AUDIO_FIFO_H_ */
