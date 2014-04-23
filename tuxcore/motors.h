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

/* $Id: motors.h 837 2008-01-07 16:42:34Z Paul_R $ */

/** \file motors.h
    \brief Motor module interface
    \ingroup movements
*/

/** \defgroup movements Movements

    The motor module contains all the functions necessery to control the
    movements of tux.
*/

/** \defgroup eyes Eyes
    \ingroup movements

     This group contains all the functions to control the eyes.
*/

/** \defgroup mouth Mouth
    \ingroup movements

     This group contains all the functions to control the mouth.
*/

/** \defgroup flippers Flippers
    \ingroup movements

     This group contains all the functions to control the flippers.
*/

/** \defgroup spin Spin
    \ingroup movements

     This group contains all the functions to control the spinning.
*/

#ifndef _MOTORS_H_
#define _MOTORS_H_

#include "hardware.h"

/** \ingroup eyes */
extern uint8_t eyes_move_counter;
/** \ingroup mouth */
extern uint8_t mouth_move_counter;
/** \ingroup flippers */
extern uint8_t flippers_move_counter, flippers_PWM;
/** \ingroup spin */
extern uint8_t spin_move_counter, spin_PWM;

/*
 * Module configuration
 */
void init_movements(void);
void powersave_movements(void);
void initMotors(void);

/*
 * Motors commands parser
 */
extern void motors_run(uint8_t const motor, uint8_t const value, uint8_t const param);
extern void motors_config(uint8_t const motor, uint8_t const pwm);

/*
 * Movements
 */
extern void stop_eyes(void);
extern void blink_eyes(uint8_t const cnt);
extern void close_eyes(void);
extern void open_eyes(void);
extern void stop_mouth(void);
extern void move_mouth(uint8_t const cnt);
extern void open_mouth(void);
extern void close_mouth(void);
extern void stop_flippers(void);
extern void reset_flippers(void);
extern void wave_flippers(uint8_t const cnt, uint8_t const pwm);
extern void lower_flippers(void);
extern void raise_flippers(void);
extern void stop_spinning(void);
extern void spin_left(uint8_t const angle, uint8_t const pwm);
extern void spin_right(uint8_t const angle, uint8_t const pwm);

/*
 * Control
 */
extern void motor_control(void);

#endif /* _MOTORS_H_ */
