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

/* $Id: hardware.h 1112 2008-05-06 09:54:21Z jaguarondi $ */

/** \file hardware.h
    \brief Hardware configuration of the I/O pins
*/

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <avr/io.h>

/** \defgroup hardware Hardware configuration
 *
 * Description of all the hardware connected to the I/O of the CPU.
 */

/** \ingroup hardware */
/*! @{ */

/**
 * \name Head button
 * The head button switch shortcuts to ground when pressed.
 *  @{ */
/** Head button PIN (input port). */
#define SW_HD_PIN PINB
/** Head button PORT. */
#define SW_HD_PT PORTB
/** Head button DDR. */
#define SW_HD_DDR DDRB
/** Head button mask.*/
#define SW_HD_MK _BV(PB5)
/*! @} */

/**
 * \name Position switches
 * All position switches shortcuts to ground when pressed.
 *
 * - The spin switch is presed approximately each quarter of turn.
 * - The flipper switch is pressed when the flippers are up or down. There's
 *   only one switch for both positions so it's tricky to know the absolute
 *   position of the flippers.
 * - The eyes and mouth both have 2 switches to detect when they are open or
 *   closed. The corresponding switch is pressed when a plain position is
 *   reached. In between, both switches are released.
 *  @{ */
/** Spin switch mask. */
#define PSW_SPIN_MK _BV(PD3)
/** Spin switch PIN. */
#define PSW_SPIN_PIN PIND
/** Spin switch PORT. */
#define PSW_SPIN_PT PORTD
/** Spin switch DDR. */
#define PSW_SPIN_DDR DDRD
/** Flippers position switch PIN. */
#define PSW_FLIPPERS_PIN PINC
/** Flippers position switch PORT. */
#define PSW_FLIPPERS_PT PORTC
/** Flippers position switch DDR. */
#define PSW_FLIPPERS_DDR DDRC
/** Flippers position switch mask. */
#define PSW_FLIPPERS_MK _BV(PC1)
/** Mouth position switches PIN. */
#define PSW_MOUTH_PIN PINB
/** Mouth position switches PORT. */
#define PSW_MOUTH_PT PORTB
/** Mouth position switches DDR. */
#define PSW_MOUTH_DDR DDRB
/** Mouth open position switch mask. */
#define PSW_MOUTH_O_MK _BV(PB3)
/** Mouth closed position switch mask. */
#define PSW_MOUTH_C_MK _BV(PB4)
/** Mouth position switches mask. */
#define PSW_MOUTH_MK (PSW_MOUTH_O_MK | PSW_MOUTH_C_MK)
/** Eyes position switches PIN. */
#define PSW_EYES_PIN PIND
/** Eyes position switches PORT. */
#define PSW_EYES_PT PORTD
/** Eyes position switches DDR. */
#define PSW_EYES_DDR DDRD
/** Eyes open position switch mask. */
#define PSW_EYES_O_MK _BV(PD6)
/** Eyes closed position switch mask. */
#define PSW_EYES_C_MK _BV(PD7)
/** Eyes position switches mask. */
#define PSW_EYES_MK (PSW_EYES_O_MK | PSW_EYES_C_MK)
/*! @} */

/**
 * \name IR
 * - The IR led is controlled through a power driver. Setting the output pin
 *   will turn the led on.
 * - The IR receiver is directly connected to the input pin. The signal is
 *   normally high and goes low when an IR signal is received.
 *  @{ */
/** IR led PORT. */
#define IR_LED_PT PORTD
/** IR led DDR. */
#define IR_LED_DDR DDRD
/** IR led mask. */
#define IR_LED_MK _BV(PD5)
/** IR receiver PIN. */
#define IR_REC_PIN PIND
/** IR receiver PORT. */
#define IR_REC_PT PORTD
/** IR receiver DDR. */
#define IR_REC_DDR DDRD
/** IR receiver mask. */
#define IR_REC_MK _BV(PD2)
/*! @} */

/**
 * \name Phototransistor
 * The Phototransistor is connected as an LDR to a 1MOhms resistor. A second
 * pull-up of 10kOhms can be added in parallel to change the load. The extra
 * pull-up is disconnected when the pin is left in high-Z mode, and is added
 * when the pin is set as strong high.
 *  @{ */
/** Phototransistor extra pull-up PORT. */
#define LIGHT_PU_PORT PORTC
/** Phototransistor extra pull-up DDR. */
#define LIGHT_PU_DDR DDRC
/** Phototransistor extra pull-up mask. */
#define LIGHT_PU_MK _BV(PC0)
/** ADMUX settings for light measurement.
 * AREF reference, result right adjusted, MUX to ADC6. */
#define LIGHT_ADMUX 0x06
/*! @} */

/**
 * \name Charger inhibit
 * This pin inhibits the charge when set high. It shouldn't be left in high-z
 * otherwise the charge won't be stable. Keep it low.
 *  @{ */
/*
 * Charger
 */
/** Charger inhibit PORT. */
#define CHARGER_INH_PT PORTB
/** Charger inhibit DDR. */
#define CHARGER_INH_DDR DDRB
/** Charger inhibit mask. */
#define CHARGER_INH_MK _BV(PB6)
/*! @} */

/**
 * \name LED
 * The leds are directly connected to the output pins through a 100 Ohms
 * resistor.
 *  @{ */
/** Blue eyes leds PIN. */
#define LED_PIN PINC
/** Blue eyes leds PORT. */
#define LED_PT PORTC
/** Blue eyes leds DDR. */
#define LED_DDR DDRC
/** Blue eyes right led mask. */
#define LED_L_MK _BV(PC2)
/** Blue eyes left led mask. */
#define LED_R_MK _BV(PC3)
/** Blue eyes both leds mask. */
#define LED_MK (LED_R_MK | LED_L_MK)
/*! @} */

/**
 * \name External I/O
 * The external I/O is routed to the external connector which is accessible in
 * the battery compartment of tux. There's a 330 Ohms resistor in series to
 * protect the IC.
 *  @{ */
/** External I/O PIN. */
#define EXIO_PIN PINB
/** External I/O PORT. */
#define EXIO_PT PORTB
/** External I/O DDR. */
#define EXIO_DDR DDRB
/** External I/O mask. */
#define EXIO_MK _BV(PB7)
/*! @} */

/**
 * \name Motors
 * The motor drivers use 2 pins to move forward or backward. Setting the first
 * pin to high will move forward, setting the other will turn backward. You
 * can't set both simultaneously as that doesn't make sense and is dangerous
 * for the motor driver. Here we added a protection so that even in this case,
 * the motor driver won't burn and the motor will simply turn in one direction.
 *  @{ */
/** Flippers backward motor PORT. */
#define MOT_FLIPPERS_BW_PT PORTD
/** Flippers backward motor DDR. */
#define MOT_FLIPPERS_BW_DDR DDRD
/** Flippers backward motor mask. */
#define MOT_FLIPPERS_BW_MK _BV(PD4)
/** Flippers forward motor PORT. */
#define MOT_FLIPPERS_FW_PT PORTB
/** Flippers forward motor DDR. */
#define MOT_FLIPPERS_FW_DDR DDRB
/** Flippers forward motor mask. */
#define MOT_FLIPPERS_FW_MK _BV(PB0)
/** Mouth motor PORT. */
#define MOT_MOUTH_PT PORTD
/** Mouth motor DDR. */
#define MOT_MOUTH_DDR DDRD
/** Mouth motor mask. */
#define MOT_MOUTH_MK _BV(PD1)
/** Mouth inverted motor PORT. */
#define MOT_IMOUTH_PT PORTD
/** Mouth inverted motor DDR. */
#define MOT_IMOUTH_DDR DDRD
/** Mouth inverted motor mask. */
#define MOT_IMOUTH_MK _BV(PD0)
/** Eyes motor PORT. */
#define MOT_IEYES_PT PORTD
/** Eyes motor DDR. */
#define MOT_IEYES_DDR DDRD
/** Eyes motor mask. */
#define MOT_IEYES_MK _BV(PD1) /* inverted eyes motor (backward) */
/** Eyes inverted motor PORT. */
#define MOT_EYES_PT PORTD
/** Eyes inverted motor DDR. */
#define MOT_EYES_DDR DDRD
/** Eyes inverted motor mask. */
#define MOT_EYES_MK _BV(PD0) /* eyes motor (forward) */
/** Spinning right motor PORT. */
#define MOT_SPIN_R_PT PORTB
/** Spinning right motor DDR. */
#define MOT_SPIN_R_DDR DDRB
/** Spinning right motor mask. */
#define MOT_SPIN_R_MK _BV(PB2) /* spin motor, turn on the right */
/** Spinning left motor PORT. */
#define MOT_SPIN_L_PT PORTB
/** Spinning left motor DDR. */
#define MOT_SPIN_L_DDR DDRB
/** Spinning left motor mask. */
#define MOT_SPIN_L_MK _BV(PB1) /* spin motor, turn on the left */
/** Spinning motor PORT. */
#define MOT_SPIN_PT PORTB
/** Spinning motor DDR. */
#define MOT_SPIN_DDR DDRB
/** Spinning motor mask. */
#define MOT_SPIN_MK (MOT_SPIN_L_MK | MOT_SPIN_R_MK)
/*! @} */

/**
 * \name Battery
 * The battery voltage can be measured on the pin ADC7 through a resistive
 * divider.
 *  @{ */
/** ADMUX settings for battery measurement.
 * AREF reference, result right adjusted, MUX to ADC7. */
#define BATTERY_ADMUX 0x07
/*! @} */
/*! @} */

#endif /* _HARDWARE_H_ */
