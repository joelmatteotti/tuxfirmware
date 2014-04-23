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

/* $Id: debug.h 1112 2008-05-06 09:54:21Z jaguarondi $ */

/**
 * \defgroup debug Debugging aid
 *
 * \section stack Checking for stack overflow
 *
 * Setting DBG_STACK to 1 will enable the init_ram function which initializes
 * the ram with a constant value in the .init1 section.  During initialization,
 * the beginning of the ram is replaced with the variables, and the end of the
 * ram is overwritten when the stack grows. Using an ICE (In-Circuit Debugger),
 * you can pause the running process at any time and check that the middle part
 * of the ram still has some cells with the initial constant value. Otherwise
 * you have a stack overflow.
 *
 * Usage:
 * - set DBG_STACK to 1 in debug.h
 * - insert the DBG_STACK_INIT macro in your main code:
 * \code
 * // If stack debugging is enabled, this macro initializes the ram with a
 * // constant value in order to catch stack overflow by examining the stack at a
 * // breakpoint.
 * DBG_STACK_INIT \endcode
 *
 * \todo TODO Could we get a function that checks the free memory by looking at
 * that middle part of the ram and sends that information back to the computer?
 * It might help knowing when we're getting close to a stack overflow.
 */

/**
 * \file debug.h
 * \ingroup debug
 * \brief Debugging aid
 */

/** \name Configuration
 * \ingroup debug */
/*! @{ */

/** Enables stack overflow detection */
#define DBG_STACK 0

/*! @} */

/**
 * \var DBG_STACK_INIT
 * \brief RAM initialization with a constant value
 * \ingroup debug
 *
 * Fill the ram with a constant value at the first .init section, before GCC
 * initializes the global variables.
 *
 * \todo TODO Explain where the offset comes from, change the code to have a define
 * of the size instead of hard coded values for the ram boundaries
 */
/* __DOXYGEN__ is set when generating the doxygen documentation, this helps
 * choosing what Doxygen should parse. */
#if (DBG_STACK || __DOXYGEN__)
#define RAM_INIT_VALUE 0x5F
#define DBG_STACK_INIT \
void init_ram(void) __attribute__((naked)) __attribute__((section(".init1"))); \
void init_ram(void) \
{ \
    uint8_t *ptr; \
 \
    for (ptr = (uint8_t *) 0x0100; ptr < (uint8_t *) 0x0300; ptr++) \
        *ptr = RAM_INIT_VALUE; \
}
#else
#define DBG_STACK_INIT
#endif
