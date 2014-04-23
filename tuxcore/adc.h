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

/* $Id: adc.h 584 2007-10-08 11:48:05Z jaguarondi $ */

/** \file adc.h
    \brief ADC module interface
    \ingroup adc
*/

/** \defgroup adc ADC
    The ADC module is the driver for the internal ADC of the AVR.
*/

#ifndef _ADC_H_
#define _ADC_H_

/*
 * Interface
 */
void ADC_set_ISR_callback(void (*ADC_ISR_user_callback)(void));
void ADC_init(void);
void ADC_disable(void);
void ADC_start(const uint8_t ad_mux);
uint16_t ADC_read(void);

#endif /* _ADC_H_ */
