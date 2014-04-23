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

/* $Id: fifo.h 2250 2008-10-07 14:26:13Z jaguarondi $ */

/** \file fifo.h
    \ingroup circular_buffer
    \brief Circular buffer (FIFO)

    \section Internals

    These routines are inspired from an article in Jack Ganssle's Embedded
    Muse: http://www.ganssle.com/tem/tem110.pdf

    The buffer size must be a power of 2 and is limited to 128. This simplyfies
    the wrap-around to a single AND operation which is only applied when
    accessing the buffer but not applied on the in and out indexes. The indexes
    increase from 0 to 255 then wrap to 0. With this method, the buffer is
    empty when the indexes are equal and is full when the difference is equal
    to the buffer size. Thus we can't use a buffer of 256 as it wouldn't be
    possible to differentiate the buffer when it's empty or full.
*/

#ifndef _FIFO_H_
#define _FIFO_H_

/** \defgroup circular_buffer Circular buffer (FIFO)

    This circular buffer module implements a minimal general use fifo.

    The fifo is a structure of a data array pointer, it's size, input and
    output indexes. The fifo buffer itself is hidden from the
    application. There are a couple of functions to put/get data or
    access its properties.

    The buffer size must be a power of 2 and is limited to 128 (2, 4,
    8, 16, 32, 64, or 128).

    Interrupts should be disabled if you access a fifo from both interrupt
    routines and the main code.

    Usage:
    - use the following include:
    \code #include "fifo.h" \endcode
    - instanciate a fifo with a given name and size. The size must be a power
      of 2 and is limited to 128 (2, 4, 8, 16, 32, 64, or 128).
    \code
    FIFO_INSTANCE(fifo_name, FIFO_SIZE);
    \endcode
    - declare a pointer to a fifo type and initialize it with the address of
      the fifo you just created
    \code
    fifo_t *myFifo = FifoPointer(fifo_name);
    \endcode

    You can instanciate multiple fifo with FIFO_INSTANCE and get the pointer to
    them with FifoPointer().
*/

/*
 * Hidden from the interface
 */

/** \brief Fifo structure type which holds the buffer, it's size, input and
 *  output indexes.
 *
 *  This structure is hidden from the application and should not be accessed
 *  directly.
 */
typedef struct fifo_t fifo_t;
struct fifo_t {
  /** buffer array */
     uint8_t *buffer;
  /** size of the buffer */
     uint8_t const size;
  /** input index, points to the next empty cell */
     uint8_t inIdx;
  /** output index, points to the next value to get */
     uint8_t outIdx;
};

/** \addtogroup circular_buffer */
/*! @{ */

/** \brief Status which is returned by some functions. */
enum FIFO_STATUS {FIFO_OK = 0, FIFO_FULL, FIFO_EMPTY};

/** \name Initialization */
/*! @{ */

/*
 * Initialization
 */

/** \brief This macro instanciates a fifo given its name and its size. The fifo
 *  can then be accessed with FifoPointer().
 *
 *  The macro declares the fifo buffer and a the fifo structure which holds the
 *  buffer, the size and the indexes. Using this macro somehow hides the
 *  structure from the application.
 *
 *  The real buffer is named with the fifo name appended with _buf and the fifo
 *  structure is named with the fifo name appended with _struct.
 */
#define FIFO_INSTANCE(fifo_name, fifo_size) \
    uint8_t fifo_name##_buf[fifo_size]; \
    fifo_t fifo_name##_struct = {fifo_name##_buf, sizeof fifo_name##_buf, 0, 0}

/** \brief Return the address of the fifo
 *  \param fifo_name Name of the fifo you defined with FIFO_INSTANCE.
 *  \return pointer to the fifo
 *
 *  As the structure should be hidden from the application, this macro should
 *  be used to return the address of a given fifo.
 */
#define FifoPointer(fifo_name) (&fifo_name##_struct)

/*! @} */

/** \name Interface */
/*! @{ */
/*
 * Interface
 */

/** \brief Return the number size of the fifo buffer.
 *  \param p Fifo pointer.
 *  \return Size of the buffer.
 */
#define FifoSize(p) (p->size)

void FifoClear(fifo_t *p);
uint8_t FifoFull(fifo_t const *p);

/** \brief Return the number of elements in the fifo buffer.
 *  \param p Fifo pointer.
 *  \return Number of elements in the buffer.
 */
#define FifoLength(p) (uint8_t)(p->inIdx - p->outIdx)

int8_t FifoPut(fifo_t *p, uint8_t const data);
int8_t FifoGet(fifo_t *p, uint8_t *data);

static inline void FifoPut_inl(fifo_t *p, uint8_t const data)
{
    if (FifoLength(p) == p->size)
        return;

    p->buffer[p->inIdx++ & (p->size-1)] = data; /* store data */
}

static inline void FifoGet_inl(fifo_t *p, uint8_t *data)
{
    if (p->outIdx == p->inIdx)
        return;

    *data = p->buffer[p->outIdx++ & (p->size-1)];
}

/*! @} */
/*! @} */
#endif /* _FIFO_H_ */
