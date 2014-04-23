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

/* $Id: init.c 2250 2008-10-07 14:26:13Z jaguarondi $ */

#include <avr/io.h>
#include "init.h"

void init_avr(void)
{
    volatile uint8_t i;

    // Crystal Oscillator division factor: 1
    /* XXX what's this little dance for? */
/* XXX CLN    CLKPR=0x80;
    CLKPR=0x00; */

    // Input/Output Ports initialization
    // Port B initialization
    //PORTB.0 ->    OUT flash _HOLD
    //PORTB.1 ->    OUT flash CS
    //PORTB.2 ->    OUT RF CS
    //PORTB.3 ->    OUT SPI MOSI
    //PORTB.4 ->    IN  SPI MISO
    //PORTB.5 ->    OUT SPI SCK
    //PORTB.6 ->    IN  RF ONLINE/OFFLINE (PULL UP)
    //PORTB.7 ->    OUT RF RESET
    DDRB = 0xAF;
    PORTB = 0xC7;

    // Port C initialization
    //PORTC.0 ->    PU  left wing push button
    //PORTC.1 ->    PU  right wing push button
    //PORTC.2 ->    PU  power plug insertion switch
    //PORTC.3 ->    PU  head button
    //PORTC.4 ->    I2C SDA
    //PORTC.5 ->    I2C SCL
    //PORTC.6 ->    RESET
    PORTC = 0x0F;
    DDRC = 0x0F;
    for (i = 0; i < 50; i++) ;
    DDRC = 0x00;

    // Port D initialization
    //PORTD.0 ->    OUT IR receiver power
    //PORTD.1 ->    OUT microphone preamplifier power (PU at beginning then
    //                  switched to strong 1)
    //PORTD.2 ->    IN  RF SPI READY
    //PORTD.3 ->    IN  RF SPI START
    //PORTD.4 ->    PU  CHARGER STATUS
    //PORTD.5 ->    OUT PWM audio output from timer compare
    //PORTD.6 ->    OUT VCC power switch
    //PORTD.7 ->    OUT audio amplifier shutdown pin _SHDN
    PORTD = 0xD3;
    DDRD = 0xE1;

    /* Timer 0 used for audio output (PWM).
     *
     * Fast PWM mode
     * TOP= 0CRA
     * OC0B compare pin and interrupt enabled */
    TCCR0A = 0x23;
    TCCR0B = 0x09;              /* no presscaling */
    OCR0A = 250;                /* we need TOP=250 to get 32kHz PWM further divided by 4 by software to get a 8kHz sampling frequency */
    OCR0B = 0x00;
    TIMSK0 = 0x01;

    /* Timer/Counter 2 used as main program tick
     * Clock source: System Clock/1024
     * Mode: Normal (top=FFh)
     * Interrupt on overflow */
    TCCR2A = 0x00;
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
    TIMSK2 = _BV(TOIE2);

    /* ADC used for microphone
     * ADC Clock frequency: 125,000 kHz
     * ADC Voltage Reference: AREF pin */
#ifndef MIC_GAIN
#define MIC_GAIN    0 /* default value if not declared in the makefile */
#endif
#if (MIC_GAIN == 0)
    /* Left adjust the result and select mic input */
    ADMUX = 0x26;
#else
    /* Right adjust the result and select mic input */
    ADMUX = 0x06;
#endif
    /* Enable the ADC, prescaler division factor of 64 so the clock is 125kHz
     * and conversion time is 104us */
    ADCSRA = 0x86;

    /* SPI master
     * SPI Clock Rate: 2 MHz
     * SPI Clock Phase: Cycle Half
     * SPI Clock Polarity: Low
     * SPI Data Order: MSB First */
    SPCR = 0x50;
    SPSR = 0x00;

    /* Read the SPDR to clear the SPI interrupt flag */
    i = SPDR;

    // External Interrupt(s) initialization
    EICRA = (_BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00));        /* Rising edge of INT0 and rising edge of INT1 */
    // EIMSK = (_BV(INT1)); [> INT0, INT1 external interrupt request enable, now moved before the main loop if the RF board is present <]
    EIFR = (_BV(INT1) | _BV(INT0));     /* INT0, INT1 external interrupt flag */
}
