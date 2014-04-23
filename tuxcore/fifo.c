/*
 * TUXCORE - Firmware for the 'core' CPU of tuxdroid
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

/* $Id: fifo.c 508 2007-09-11 14:07:55Z jaguarondi $ */

/** \file fifo.c
    \brief Circular buffer (FIFO)
*/

#include <inttypes.h>
#include "fifo.h"

/** \weakgroup circular_buffer */
/*! @{ */

/** \brief Empty the buffer by clearing the indexes.
 *  \param p Fifo pointer.
 */
void FifoClear(fifo_t *p)
{
    p->inIdx = 0;
    p->outIdx = 0;
}

/** \brief Return TRUE if the buffer is full.
 *  \param p Fifo pointer.
 *  \return TRUE if the buffer is full, FALSE otherwise.
 */
uint8_t FifoFull(fifo_t const *p)
{
    return (FifoLength(p) == p->size);
}

/** \brief Add one data byte to the fifo buffer.
 *  \param p Fifo pointer.
 *  \param data Data byte to add to the queue.
 *  \return Return FIFO_OK if the data has been added, FIFO_FULL if the buffer
 *  was full and the data couldn't be added.
 */
int8_t FifoPut(fifo_t *p, uint8_t const data)
{
    if (FifoLength(p) == p->size)
        return FIFO_FULL;

    p->buffer[p->inIdx++ & (p->size-1)] = data; /* store data */
    return FIFO_OK;
}

/** \brief Pop the oldest byte from the buffer.
 *  \param p Fifo pointer.
 *  \param data pointer for storing the data read from the queue
 *  \return FIFO_OK if a value has been popped out at the pointer address. If
 *  the fifo is empty, FIFO_EMPTY is returned and the pointed data is left
 *  unchanged.
 */
int8_t FifoGet(fifo_t *p, uint8_t *data)
{
    if (p->outIdx == p->inIdx)
        return FIFO_EMPTY;

    *data = p->buffer[p->outIdx++ & (p->size-1)];
    return FIFO_OK;
}

/*! @} */
