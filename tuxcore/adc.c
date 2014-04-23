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

/* $Id: adc.c 1112 2008-05-06 09:54:21Z jaguarondi $ */

/** \file adc.c
    \brief ADC module
    \ingroup adc
*/
//#define __AVR_LIBC_DEPRECATED_ENABLE__

#include <avr/interrupt.h>
#include <avr/io.h>

#include "adc.h"

/* Union between 16 bits variables and the two 8 bits registers they're made
 * of. */
typedef union
{
    uint16_t w;
    uint8_t b[2];
} union16_t;

/** Function pointer to the ADC ISR user function. */
static void (*ADC_ISR_callback)(void);

/**
   \brief Set the user function which handles the ADC ISR
   \param ADC_ISR_user_callback Function pointer to the user callback function.
   \ingroup adc

   This sets the user function which is called when the conversion completes.
 */
void ADC_set_ISR_callback(void (*ADC_ISR_user_callback)(void))
{
    ADC_ISR_callback = ADC_ISR_user_callback;
}

/**
   \brief Initialize and enable the ADC module.
   \ingroup adc
 */
void ADC_init()
{
    /* Enable ADC in single conversion mode. */
    ADCSRA = _BV(ADEN) |
     /* Enable ADC interrupt. */
        _BV(ADIE) |
     /* Set ADC prescaler to 128.
     * Clock = F_CPU / 128 = 62.5kHz
     * Sample rate = clock / 13 cycles = 4.8kHz (around 2ms). */
        _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

/**
   \brief Disable the ADC.
   The ADC should be re-enabled with ADC_init(), which restores the prescaler.
   \ingroup adc
 */
void ADC_disable()
{
    ADCSRA = 0;
}

/**
   \brief Start a conversion on the selected channel.
   \param ad_mux (ADMUX) Reference, format and channel selection.
   \ingroup adc

   ADC Multiplexer Selection Register - ADMUX
     Bit 7:6 - REFS1:0: Reference Selection Bits
       0 0 AREF, Internal Vref turned off
       0 1 AVCC with external capacitor at AREF pin
       1 0 Reserved
       1 1 Internal 1.1V Voltage Reference with external capacitor at AREF pin
     Bit 5 - ADLAR: ADC Left Adjust Result
     Bit 4 - Res: Reserved Bit
     Bits 3:0 - MUX3:0: Analog Channel Selection Bits
       0000 ADC0
       0001 ADC1
       0010 ADC2
       0011 ADC3
       0100 ADC4
       0101 ADC5
       0110 ADC6
       0111 ADC7
       1110 1.1V (VBG)
       1111 0V (GND)
*/
void ADC_start(const uint8_t ad_mux)
{
    ADMUX = ad_mux;
    ADCSRA |= _BV(ADSC);
}

/**
   \brief Read the value of the last conversion.
   \return Conversion result (10 bits).
   \ingroup adc
   You should ensure that the conversion is completed before reading the value.
   */
uint16_t ADC_read()
{
   union16_t adc_value;
    /* ADCH should be read after ADCL otherwise ADCL stays wite protected
     * (c.f. datasheet). */
    adc_value.b[0] = ADCL;
    adc_value.b[1] = ADCH;
    return adc_value.w;
}

/**
   \brief ADC conversion interrupt
   The callback function is called when the interrupt occurs.
 */
//ISR(SIG_ADC)
ISR(ADC_vect) /* Mise à jour: 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    ADC_ISR_callback();
}
