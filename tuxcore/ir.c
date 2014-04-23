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

/* $Id: ir.c 1112 2008-05-06 09:54:21Z jaguarondi $ */
//#define __AVR_LIBC_DEPRECATED_ENABLE__


#include <avr/interrupt.h>
#include <avr/io.h>

#include "global.h"
#include "hardware.h"
#include "ir.h"

// #define __debug__

uint8_t volatile irStatus;
uint8_t volatile irPulses;
volatile RC5_STRUCT irRC5SendData;
volatile RC5_DATA irRC5ReceivedData;
volatile uint16_t irReceivedCode;

//ISR(SIG_INTERRUPT0)
ISR(INT0_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    if (irStatus & IRSTATUS_END)
    {
        irPulses = SILENCE_PULSES;      /* If in end mode, reset timer to wait for silence before receiving next data */
    }
    else if (irStatus & IRSTATUS_MEASURE)       /* End of measurement */
    {
#ifdef __debug__
        {
            for (_i = 0; _i < 80; _i++)
                PORTC |= 0x04;
            PORTC &= ~0x04;
        }
#endif
        uint8_t period;

        TCCR0B &= ~_BV(CS00);   /* Stop timer */
        period = 0xFF - irPulses;
        irRC5SendData.pulse = (period << 1) - (period >> 1);
        irStatus &= ~IRSTATUS_MEASURE;
        irStatus |= IRSTATUS_RECEIVING; /* go to Receiving state */
        irRC5SendData.bit = RC5_BIT_NUMBER - 2; /* The 2 start bits are already passed */
        irReceivedCode = 0x0000;        /* Clear the buffer to receive code */
    }
    else if (irStatus & IRSTATUS_RECEIVING)     /* Each bit at half time, start timer for a 3/4 bit length to check the value during the first 1/4 of the bit */
    {
#ifdef __debug__
        {
            for (_i = 0; _i < 80; _i++)
                PORTC |= 0x04;
            PORTC |= 0x04;
            PORTC &= ~0x04;
        }
#endif
        irPulses = irRC5SendData.pulse;
        TCCR0B |= _BV(CS00);    /* Start timer, only necessary the first time but well, it's here */
        irStatus &= ~IRSTATUS_REC_WAIT;
        disableIrExtint();
    }
    else                        /* Receiving first start bit */
    {
#ifdef __debug__
        {
            for (_i = 0; _i < 80; _i++)
                PORTC |= 0x04;
            PORTC |= 0x04;
            PORTC &= ~0x04;
        }
#endif
        irPulses = 0xFF;
        TCCR0B |= _BV(CS00);    /* Start timer */
        irStatus |= IRSTATUS_MEASURE;   /* go to Measure state */
    }
}

/*
 * IR TIMER
 */
//ISR(SIG_OUTPUT_COMPARE0A)
ISR(TIMER0_COMPA_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    IR_LED_PT &= ~IR_LED_MK;
    if (irPulses)
        irPulses--;

    /*
     * RECEIVING MODE
     */
    else
    {
        if (irStatus & IRSTATUS_MODE)   /* End of bit in Receive mode */
        {
            if (irStatus & IRSTATUS_REC_WAIT)   /* Timer overflow, the dataflow has been interrupted, start over */
            {
                irStatus |= IRSTATUS_END;
                irStatus &= ~(IRSTATUS_RECEIVING | IRSTATUS_REC_WAIT);
                irPulses = SILENCE_PULSES;      /* If in end mode, reset timer to wait for silence before receiving next data */
            }
            else if (irStatus & IRSTATUS_RECEIVING)     /* We are at 1/4 of the bit length and should read the value */
            {
#ifdef __debug__
                {
                    for (_i = 0; _i < 160; _i++)
                        PORTC |= 0x04;
                    PORTC |= 0x04;
                    PORTC &= ~0x04;
                }
#endif
#ifdef __debug__
                {
                    PORTC |= 0x04;
                    PORTC &= ~0x04;
                }
#endif
#ifdef __debug__
                {
                    PORTC |= 0x04;
                    PORTC &= ~0x04;
                }
#endif
                irRC5SendData.bit--;
                if (IR_REC_PIN & IR_REC_MK)
                    irReceivedCode |= (0x0001 << irRC5SendData.bit);
                if (irRC5SendData.bit == 0)     /* End of reception */
                {
                    /* We'll send the IR code 2 times to the computer. */
                    ir_f = 2;
                    irStatus |= IRSTATUS_END;
                    irStatus &= ~IRSTATUS_RECEIVING;
                    irPulses = SILENCE_PULSES;  /* If in end mode, reset timer to wait for silence before receiving next data */
                    /* XXX IR command and address need to be sent to the computer */
                    /* Update global status */
                    gStatus.ir = ((uint8_t)irReceivedCode & 0x3F);
                    /* Check if the code comes from our remote control. */
                    if ((irReceivedCode & 0x07C0) == 0x0740)
                        gStatus.ir |= GSTATUS_IR_VALID; /* set valid bit */
                    if (irReceivedCode & 0x0800)
                        gStatus.ir |= GSTATUS_IR_TOGGLE;    /* set toggle bit */
                    else
                        gStatus.ir &= ~GSTATUS_IR_TOGGLE;   /* clear toggle bit */

                }
                else
                {
                    irStatus |= IRSTATUS_REC_WAIT;
                    irPulses = SILENCE_PULSES;  /* Set an overflow timer in case next data will never appear */
                }
                enableIrExtint();
            }
            else if (irStatus & IRSTATUS_END)   /* End of transmission */
            {
                irStatus &= ~IRSTATUS_END;      /* Get back to ready state waiting for the start bit */
                TCCR0B &= ~_BV(CS00);   /* Stop timer */
            }
        }

        /*
         * SEND MODE
         */
        else                    /* End of bit in Send mode */
        {
            if (irStatus & IRSTATUS_PHASE)      /* Send second phase of bit */
            {
                irStatus ^= IRSTATUS_EMIT;      /* toggle output */
                irStatus &= ~IRSTATUS_PHASE;
                irPulses = irRC5SendData.pulse;
            }
            else if (irRC5SendData.bit) /* Send next bit */
            {
                irRC5SendData.bit--;
                if (!(irRC5SendData.code & (0x0001 << irRC5SendData.bit)))
                    irStatus |= IRSTATUS_EMIT;
                else
                    irStatus &= ~IRSTATUS_EMIT;
                irStatus |= IRSTATUS_PHASE;
                irPulses = irRC5SendData.pulse;
            }
            else                /* End of transmission */
            {
                irStatus |= IRSTATUS_END;       /* Receiving mode will be resumed in this mode */
                irStatus |= IRSTATUS_MODE;      /* Return to Receive mode */
                irStatus &= ~IRSTATUS_EMIT;     /* Disable LED output */
                irPulses = SILENCE_PULSES;      /* If in end mode, reset timer to wait for silence before receiving next data */
                enableIrExtint();       /* Resume receiving interrupt */
            }
            if (irStatus & IRSTATUS_EMIT)
                enableIrLed();  /* set LED PWM output */
            else
                disableIrLed();
        }
    }
}

void initIR(void)
{
    IR_LED_PT &= ~IR_LED_MK;
    IR_LED_DDR |= IR_LED_MK;
}

void turnIrOn(void)
{
    IR_LED_PT |= IR_LED_MK;
}

void turnIrOff(void)
{
    IR_LED_PT &= ~IR_LED_MK;
}

void configureIrTimer(void)
{
    /*
     * timer 0 intitialisation
     *
     * IR receiver timer
     *
     * Mode: FAST PWM
     * Prescaler: F_CPU/1 : 8MHz
     * Period: 211(0xd3) = 26.275us
     * RC5 bits = 1.778ms is 67.4 cycles
     */

    TCCR0A = _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(WGM02);
    TCNT0 = 0x00;
    OCR0B = 0x08;               /* we use a small duty cycle but the IR driver holds the pulse a bit more so the effective duty cycle is bigger */
    OCR0A = 0xd3;
    TIMSK0 = _BV(OCIE0A);
}

#define PULSES 34
void irSendRC5(uint8_t address, uint8_t command)
{
    disableIR();                /* Will be resumed automatically at the end of transmission in the timer interrupt */
    configureIrTimer();
    irStatus = IRSTATUS_MODE_SEND;      /* IRSTATUS_MODE cleared */
    irRC5SendData.bit = RC5_BIT_NUMBER;
    irPulses = 0;               /* Start interrupt by loading a bit */
    irStatus = 0;               /* reset status to send mode */
    /* code: S1 S2 T Address(5b) Command(6b)
     * S1, S2: start bits (always 1)
     * T: toggle bit, given with address */
    irRC5SendData.code = address | 0xC0;        /* Set the 2 start bits */
    irRC5SendData.code <<= 6;
    irRC5SendData.code |= (command & 0x3F);     /* Mask to have 6 bits only */
    irRC5SendData.pulse = PULSES;
    TCCR0B |= _BV(CS00);        // Start timer
}

void irGetRC5(void)
{
    configureIrTimer();
    irStatus = IRSTATUS_MODE_GET;       /* IRSTATUS_MODE set */
    irStatus |= IRSTATUS_END;   /* Start detection by waiting for a silence wich occurs at the end of a detection */
    irPulses = SILENCE_PULSES;  /* If in end mode, reset timer to wait for silence before receiving next data */
    TCCR0B |= _BV(CS00);        /* Start timer */
    enableIrExtint();
}

void disableIR(void)
{
    TCCR0B &= ~_BV(CS00);       /* Stop timer */
    disableIrExtint();
}

void stopIRReceiver(void)
{
    TCCR0B &= ~_BV(CS00);       /* Stop timer */
    disableIrExtint();
}
