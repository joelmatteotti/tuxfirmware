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

/* $Id: led.h 938 2008-03-17 05:16:51Z jaguarondi $ */

/** \file led.h
    \brief Blue LEDs control interface.
    \ingroup led
*/

/** \defgroup led LEDs
    The LED module contains the functions that drive the eye LEDs.
*/

#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>
#include "hardware.h"
#include "common/defines.h"

/**
   \ingroup led
   LED status structure holding all information necessary to handle intensity,
   fading and pulsing effects.
 */
typedef struct led_t
{
    struct led_status
    {
        uint8_t intensity;
        bool fading;
        bool pulsing;
    } status;
    struct led_command
    {
        uint8_t setpoint;
        uint8_t delay;
        uint8_t step;
    } command;
    struct led_pulse
    {
        uint8_t min_intensity;
        uint8_t max_intensity;
        /** Number of times the LEDs should be toggled when pulsing. */
        uint8_t count;
        /** Delay between 2 toggles of the LEDs when pulsing them. */
        uint8_t pulse_width;
    } pulse;
    struct led_var
    {
        uint8_t speed_cnt;
        /** Timer for the delay between 2 toggles of the LEDs. */
        uint8_t pulse_tmr;
        bool pulse_flag;
    } var;
} led_t;

extern led_t left_led;
extern led_t right_led;
extern bool led_f;

void led_init(void);
void led_shutdown(void);
void led_set_fade_speed(leds_t leds, uint8_t const delay, uint8_t const step);
void led_set_intensity(leds_t leds, uint8_t const intensity);
void led_pulse_range(leds_t leds, uint8_t const max, uint8_t const min);
void led_pulse(leds_t led, uint8_t const cnt, uint8_t const pulse_width);
void leds_toggle(uint8_t const cnt, uint8_t const pulse_width);
void led_control(bool mask);

#endif /* _LED_H_ */
