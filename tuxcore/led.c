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

/* $Id: led.c 938 2008-03-17 05:16:51Z jaguarondi $ */

/** \file led.c
    \brief Blue LEDs control module
    \ingroup led
*/
//#define __AVR_LIBC_DEPRECATED_ENABLE__


#include <stdbool.h>
#include <avr/interrupt.h>


#include "led.h"

/** Buffer the LEDs values before being applied on the I/O port. */
uint8_t leds_buffer = 0;
/** Flag set when the LED status changed. */
bool led_f;

/** Left LED status structure. */
led_t left_led =
{
    .command.step=2,
    .command.delay=1,
    .pulse.max_intensity = 0xFF,
};

/** Right LED status structure. */
led_t right_led =
{
    .command.step=2,
    .command.delay=1,
    .pulse.max_intensity = 0xFF,
};

/**
   \brief Turn left LED on.
 */
#define turn_left_led_on() \
    do {leds_buffer |= LED_L_MK; \
        TIMSK1 |= _BV(OCIE1A); \
    } while(0)

/**
   \brief Turn right LED on.
 */
#define turn_right_led_on() \
    do {leds_buffer |= LED_R_MK; \
        TIMSK1 |= _BV(OCIE1B); \
    } while(0)

/**
   \brief Turn left LED off.
 */
#define turn_left_led_off() \
    do {leds_buffer &= ~LED_L_MK; \
        TIMSK1 &= ~_BV(OCIE1A); \
        LED_PT &= ~LED_L_MK; \
    } while(0)

/**
   \brief Turn right LED off.
 */
#define turn_right_led_off() \
    do {leds_buffer &= ~LED_R_MK; \
        TIMSK1 &= ~_BV(OCIE1B); \
        LED_PT &= ~LED_R_MK; \
    } while(0)

/**
 * Timer1 initialization, used for applying PWM on the LEDs in order to change
 * their intensity and have some fading effects.
 * \ingroup led
 *
 * Settings:
 *   - Fast PWM, 8-bit mode.
 *   - CLK/8 = 1MHz
 *
 * Timer is clocked at 1MHz (CLK/8) as this frequency doesn't seem to disturb
 * the IR receiver too much. Slower frequencies does.
 * Output Compare A and B are used for toggling LEDs left and right so they can
 * be controlled separately.
 */
void led_init(void)
{
    TCCR1A = _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS11);
    TIMSK1 = _BV(TOIE1);
}

/**
 * Shutting down the LEDs for power saving.
 * \ingroup led
 */
void led_shutdown(void)
{
    TCCR1B = 0;
    turn_left_led_off();
    turn_right_led_off();
}

/**
   \brief Refresh the LEDs I/O with the buffer value.
   */
static inline void refresh_leds(void)
{
    /* We should avoid glitches on the LEDs otherwise the IR receiver goes
     * crazy. So we can't simply set the output pins to 0 then set them with
     * leds_buffer. Only the pin that should be 0 has to be set to 0. */
    /* Commented until a better solution is found for driving the leds */
    //LED_PT &= ~(leds_buffer ^ LED_MK); // done in the 4ms loop for now
    LED_PT |= leds_buffer;
}

/**
 * Timer1 overflow interrupt, sets the LEDs for the PWM.
 */
//ISR(SIG_OVERFLOW1)
ISR(TIMER1_OVF_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    refresh_leds();
}

/**
 * Timer1 compare A interrupt, clear the left LED for the PWM.
 */
//ISR(SIG_OUTPUT_COMPARE1A)
ISR(TIMER1_COMPA_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    LED_PT &= ~LED_L_MK;
}

/**
 * Timer1 compare B interrupt, clear the right LED for the PWM.
 */
//ISR(SIG_OUTPUT_COMPARE1B)
ISR(TIMER1_COMPB_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    LED_PT &= ~LED_R_MK;
}

/**
   \brief Pulse LEDs
   \ingroup led
   \param leds Which LEDs are affected by the command
   \param cnt Number of pulses. 0 is ignored, 255 (0xFF) means infinite.
   \param pulse_width Pulse width. 0 is ignored.

   The LEDs will be toggled 'cnt' times.  The pulse width can be used to change
   the toggle frequency.
   If 'cnt' or 'pulse_width' are set to 0, the register is not updated so the
   old value is kept.
   'cnt' can be set to 255 to trigger an infinite pulsing effect.
 */
void led_pulse(leds_t leds, uint8_t const cnt, uint8_t const pulse_width)
{
    /* XXX we should really find a way to optimize this loop for the 2 leds.
     * We don't want to add a new function call, but don't want to have the
     * code written twice. Maybe an inline function can do it with compiler
     * optimizations. */
    led_t *led = &left_led;

    while (leds & LED_BOTH)
    {
        if (leds & LED_LEFT)
        {
            leds &= ~LED_LEFT;
            /* Here, we should already have led = &left_led;*/
        }
        else if (leds & LED_RIGHT)
        {
            leds &= ~LED_RIGHT;
            led = &right_led;
        }
        led->status.pulsing = true;
        led->pulse.pulse_width = pulse_width;
        if (cnt)
        {
            led->pulse.count = cnt;
            /* We can't simply set the flag here otherwise we alter the ongoing
             * pulsing. So we need to check for pulsing first. */
            if (!led->var.pulse_tmr)
                led->var.pulse_flag = true;
        }
    }
}

/**
   \brief Simple function to toggle both leds only if they're not pulsing or
   fading already.
   \ingroup led
   \param cnt Number of pulses. 0 is ignored, 255 (0xFF) means infinite.
   \param pulse_width Pulse width. 0 is ignored.
 */
void leds_toggle(uint8_t const cnt, uint8_t const pulse_width)
{
    /* Only toggle from the standalone if no computer commands are already
     * driving the LEDs. */
    if (!(left_led.status.fading || left_led.status.pulsing \
          || right_led.status.fading || right_led.status.pulsing))
    {
        led_set_fade_speed(LED_BOTH, 1, 10);
        led_pulse(LED_BOTH, cnt, pulse_width);
    }
}

/**
   \brief Set the intensity boundaries for the pulse command.
   \ingroup led
   \param leds Which LEDs are affected by the command
   \param max Maximum intensity
   \param min Minimum intensity
*/
void led_pulse_range(leds_t leds, uint8_t const max, uint8_t const min)
{
    led_t *led = &left_led;

    while (leds & LED_BOTH)
    {
        if (leds & LED_LEFT)
        {
            leds &= ~LED_LEFT;
            /* Here, we should already have led = &left_led;*/
        }
        else if (leds & LED_RIGHT)
        {
            leds &= ~LED_RIGHT;
            led = &right_led;
        }
        led->pulse.max_intensity = max;
        led->pulse.min_intensity = min;
    }
}

/**
   \brief Set the delay and step which determine the speed of the fading
   effect.
   \ingroup led
   \param leds Which LEDs are affected by the command
   \param delay Number of 4ms loops before the intensity is changed. See struct
   led_command.
   \param step Intenisty step applied when fading.
 */
void led_set_fade_speed(leds_t leds, uint8_t const delay, uint8_t const step)
{
    led_t *led = &left_led;

    while (leds & LED_BOTH)
    {
        if (leds & LED_LEFT)
        {
            leds &= ~LED_LEFT;
            /* Here, we should already have led = &left_led;*/
        }
        else if (leds & LED_RIGHT)
        {
            leds &= ~LED_RIGHT;
            led = &right_led;
        }
        if (delay)
            led->command.delay = delay;
        if (step)
            led->command.step = step;
    }
}

/**
   \brief Set LEDs intensity with fading effect.
   \ingroup led
   \param leds Set left, right or both leds.
   \param intensity 8 bits PWM value the led should be set to.
 */
void led_set_intensity(leds_t leds, uint8_t const intensity)
{
    led_t *led = &left_led;

    while (leds & LED_BOTH)
    {
        if (leds & LED_LEFT)
        {
            leds &= ~LED_LEFT;
            /* Here, we should already have led = &left_led;*/
        }
        else if (leds & LED_RIGHT)
        {
            leds &= ~LED_RIGHT;
            led = &right_led;
        }
        led->command.setpoint = intensity;
        led->status.fading = true;
        /* Disable pulsing at the same time. */
        led->status.pulsing = false;
        led->var.pulse_tmr = 0;
        led->var.pulse_flag = false;
    }
}

/**
   \brief Handles intensity fading.
   \param led Pointer to the led structure

   Set the new intensity and set the fading flag to false when fading is
   completed.
 */
static void fading(led_t *led)
{
    if (!led->var.speed_cnt)
    {
        /* LED status is changing. */
        led_f = true;
        if (led->command.setpoint > led->status.intensity)
        {
            if (led->status.intensity + led->command.step
                >= led->command.setpoint)
            {
                led->status.intensity = led->command.setpoint;
                led->status.fading = false;
                return;
            }
            else
                led->status.intensity += led->command.step;
        }
        else
            /* led->command.setpoint < led->status.intensity here */
        {
            if (led->status.intensity - led->command.step
                <= led->command.setpoint)
            {
                led->status.intensity = led->command.setpoint;
                led->status.fading = false;
                return;
            }
            else
                led->status.intensity -= led->command.step;
        }
        led->var.speed_cnt = led->command.delay;
    }
    /* we decrease in all cases, i.e. if delay == 1, cnt will be set to 0 when
     * leaving the function and so will only loop one time */
    led->var.speed_cnt--;
}

/**
   \brief Handles the pulsing effect.
   \param led Pointer to the led structure

   Set the new fading settings if more pulsing has to be done.
 */
static void pulsing(led_t *led)
{
    /* Infinite pulsing or next pulse. */
    if ((led->pulse.count == 0xFF) || --led->pulse.count)
        led->var.pulse_tmr = led->pulse.pulse_width;
    else
        led->status.pulsing = false;
    if (led->status.intensity <= led->pulse.min_intensity)
        led->command.setpoint = led->pulse.max_intensity;
    else
        (led->command.setpoint = led->pulse.min_intensity);
    led->status.fading = true;
    /* Add the intermediate delay between 2 fades. */
    led->var.speed_cnt = led->command.delay;
}

/**
   \brief Check when fading or pulsing need to be done.
   \param led Pointer to the led structure

   This function should be called at a regular interval, which is the time
   basis for the delays.
 */
static inline void control_effects(led_t *led)
{
    /* Do we need fading? */
    if (led->status.fading)
        fading(led);

    /* Check the pulse width. */
    if (led->var.pulse_tmr)
    {
        led->var.pulse_tmr--;
        if (!led->var.pulse_tmr)
            led->var.pulse_flag = true;
    }
    if (!led->status.fading)
        /* When fading is done, check if we need to pulse again. */
        if (led->var.pulse_flag)
        {
            pulsing(led);
            led->var.pulse_flag = false;
        }
}

/**
   \brief Periodic routine that controls the LEDs.
   \ingroup led
   \param mask If set, masking is applied on the LEDs so they don't light. Used
   when the eyes are closed, it's neater.

   This function should be called regularly as it controls the pulsing. The
   period this function is called will be the unit period of the led pulsing.
 */
void led_control(bool mask)
{
    static bool previous_mask = false;

    if (previous_mask != mask)
    {
        previous_mask = mask;
        /* LED status is changing. */
        led_f = true;
    }

    control_effects(&left_led);
    control_effects(&right_led);

    /* This circumvent the bug we have when the OCR values are too low (<5 it
     * seems), interrupt priority of the comparisons is higher than the
     * overflow so when 2 interrupts occurs nearly simultaneously, the compare
     * is done before the overflow which is not what we want. The result is
     * that the led is ON instead of OFF with low values. XXX should find a way
     * to fix that. */
    if (left_led.status.intensity <= 5 || mask)
        turn_left_led_off();
    else
        turn_left_led_on();
    if (right_led.status.intensity <= 5 || mask)
        turn_right_led_off();
    else
        turn_right_led_on();

    /* Update the PWM registers. */
    OCR1AL = left_led.status.intensity;
    OCR1BL = right_led.status.intensity;
}
