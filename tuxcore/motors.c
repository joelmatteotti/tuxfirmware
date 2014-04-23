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

/* $Id: motors.c 5306 2009-08-13 04:41:17Z ks156 $ */

/** \file motors.c
    \brief Motor module
    \ingroup movements
*/

//#define __AVR_LIBC_DEPRECATED_ENABLE__


#include <avr/interrupt.h>
#include <avr/io.h>

#include "global.h"
#include "config.h"
#include "motors.h"
#include "common/defines.h"
/**
 * \name Motors PWM
 * @{ */
uint8_t flippers_params_pwm = 5;
uint8_t spin_params_pwm = 5;
/*! @} */

/**
 * \name Braking delays
 * These functions access the motor I/O port.
 *  @{ */
/** Delay during which the motor is inverted when braking the eyes to stop them
 * in the close position. There's no need to brake the motor when opening the
 * eyes with this gearbox design. */
#define EYES_BRAKING_DLY 6
/** Protection timeout for the eyes. */
#define EYES_TIMEOUT 200
/** Protection timeout for the mouth. */
#define MOUTH_TIMEOUT 200
/** Delay during which the motor is inverted when braking the mouth to stop it
 * in the open or close position. */
#define MOUTH_BRAKING_DLY 8
/** Delay during which the motor is inverted when braking the spinning. Braking
 * here is necessary to improve the accuracy of the absolute position. */
#define SPIN_BRAKING_DLY 10
/** Protection timeout for spinning. */
#define SPIN_TIMEOUT 250
/** Delay during which the motor is inverted when braking the flippers to stop
 * them either up or down. If we don't brake here, the flippers end up in a
 * medium position. */
#define FLIPPERS_BRAKING_DLY 4
/** Protection timeout for the flippers. */
#define FLIPPERS_TIMEOUT 250
/*! @} */
/** Init value of the timer used to reset the flippers in the low position. */
#define FLIP_TIMER_INIT 0xFF
/** Minimum difference required between the period the flippers are moving from
 * up to down and from down to up. */
#define FLIPPERS_RESETTIMER_HYST 0x0A

/** Number of periods remaining before stopping the eyes.
 * Normally used when braking, to stop the motor after a short period of
 * inversion. */
static uint8_t eyes_stop_delay;
/** Number of movements remaining before stopping the flippers. */
uint8_t eyes_move_counter;
/** Number of periods remaining before stopping the mouth.
 * Normally used when braking, to stop the motor after a short period of
 * inversion. */
static uint8_t mouth_stop_delay;
/** Number of movements remaining before stopping the mouth. */
uint8_t mouth_move_counter;
/** Number of periods remaining before stopping the flippers.
 * Normally used when braking, to stop the motor after a short period of
 * inversion. */
static uint8_t flippers_stop_delay;
/** State variable indicating the final requested state for the mouth */
static uint8_t mouth_final_state;
/** Number of movements remaining before stopping the flippers. */
uint8_t flippers_move_counter;
/** PWM applied on the flippers motor. */
uint8_t flippers_PWM = 5;
/** Timer used to measure the period the flippers take between the low and high
 * positions. */
uint8_t flippers_timer;
/** Period taken by the previous movement of the flippers. */
uint8_t flippers_previous_timer;

/** Number of periods remaining before stopping the rotation.
 * Normally used when braking, to stop the motor after a short period of
 * inversion. */
static uint8_t spin_stop_delay;
/** Number of movements remaining before stopping the rotation. */
uint8_t spin_move_counter;
/** PWM applied on the spinning motor. */
uint8_t spin_PWM = 5;
/** Direction type for spinning. */
enum spin_direction
{
    LEFT,
    RIGHT,
};
/** Spinning direction */
static uint8_t spin_direction;

/** Flag byte to indicate if the movement is specified by duration or not */
uint8_t duration_movement;
/** Eyes bit in duration_movement */
#define EYES_FLAG _BV(0)
/** Mouth bit in duration_movement */
#define MOUTH_FLAG _BV(1)
/** Flippers bit in duration_movement */
#define FLIPPERS_FLAG _BV(2)
/** Spin bit in duration_movement */
#define SPIN_FLAG _BV(3)

/** PWM mask register applied on the motor port on a regular basis to drive the
 * flippers and spinning. */
uint8_t portB_PWM_mask;
#define flippers_PWMMask portB_PWM_mask
#define spin_PWM_mask portB_PWM_mask

/**
 * \name Module configuration
 * These functions initialize the motors and switches I/O ports for normal and
 * sleep mode.
 *  @{ */
/** \ingroup movements */
/**
   \brief Initialize all I/O for normal movement operation.
   This function sets all motor I/O as output, initializes all I/O of the
   position switches as input with internal pull-up and enables all
   corresponding interrupts.
 */
void init_movements(void)
{
    /* Position switches */
    PSW_FLIPPERS_DDR &= ~PSW_FLIPPERS_MK;
    PSW_FLIPPERS_PT |= PSW_FLIPPERS_MK;
    PSW_EYES_DDR &= ~PSW_EYES_MK;
    PSW_EYES_PT |= PSW_EYES_MK;
    PSW_MOUTH_DDR &= ~PSW_MOUTH_MK;
    PSW_MOUTH_PT |= PSW_MOUTH_MK;
    PSW_SPIN_DDR &= ~PSW_SPIN_MK;
    PSW_SPIN_PT |= PSW_SPIN_MK;

    /* Motor drivers as output */
    if (hwrev == 1)
    {
        MOT_EYES_DDR |= MOT_EYES_MK;
        MOT_MOUTH_DDR |= MOT_MOUTH_MK;
        //MOT_FLIPPERS_FW_DDR |= MOT_FLIPPERS_FW_MK;
        MOT_FLIPPERS_BW_DDR |= MOT_FLIPPERS_BW_MK;
    }

    /* Enable 'Pin change interrupt 0' for the mouth position switches */
    PCMSK0 = PSW_MOUTH_MK;
    PCICR |= _BV(PCIE0);
    /* Enable 'Pin change interrupt 1' for the flippers position switch */
    PCMSK1 = PSW_FLIPPERS_MK;
    PCICR |= _BV(PCIE1);
    /* Enable 'Pin change interrupt 2' for the eyes position switches */
    PCMSK2 = PSW_EYES_C_MK;
    PCICR |= _BV(PCIE2);
    /* Enable 'INT1 interrupt' for the spin position switch in falling edge
     * mode */
    EICRA |= _BV(ISC11);
    EIMSK |= _BV(INT1);
}

/**
   \brief Initialize the position switches for minimal power consumption.
   In normal mode, all pins have internal pull-up and each switch shorts the
   pin low. So in power save mode, we must set all pins as strong low.
 */
void powersave_movements(void)
{
    PSW_FLIPPERS_PT &= ~PSW_FLIPPERS_MK;
    PSW_FLIPPERS_DDR |= PSW_FLIPPERS_MK;
    PSW_EYES_PT &= ~PSW_EYES_MK;
    PSW_EYES_DDR |= PSW_EYES_MK;
    PSW_MOUTH_PT &= ~PSW_MOUTH_MK;
    PSW_MOUTH_DDR |= PSW_MOUTH_MK;
    PSW_SPIN_PT &= ~PSW_SPIN_MK;
    PSW_SPIN_DDR |= PSW_SPIN_MK;
}
/*! @} */

/**
 * \name Motors command parser
 * These functions parse the received command, and call the specific motor
 * function.
 *  @{ */
/** \ingroup movements */
/**
   \brief Parse the MOTORS_SET_CMD to command a motor with a specific number of movements.
   \param motor The motor to command
   \param value The number of movements or/ the timeout valueto execute
   \param param bit 0 : 0 for count, 1 for timeout
 */
void motors_run(uint8_t motor, uint8_t const value, uint8_t const param)
{
    uint8_t cnt;

    if (motor == MOT_EYES)
    {
        if (param & 0x01)
        {
            eyes_stop_delay = value;
            duration_movement |= EYES_FLAG;
            cnt = 0;
        }
        else
            cnt = value;
        blink_eyes(cnt);
    }
    else if (motor == MOT_MOUTH)
    {
        if (param & 0x01)
        {
            mouth_stop_delay = value;
            duration_movement |= MOUTH_FLAG;
            cnt = 0;
        }
        else
            cnt = value;
        move_mouth(cnt);
    }
    else if (motor == MOT_FLIPPERS)
    {
        if (param & 0x01)
        {
            flippers_stop_delay = value;
            duration_movement |= FLIPPERS_FLAG;
            cnt = 0;
        }
        else
            cnt = value;
        wave_flippers(cnt, flippers_params_pwm);
    }
    else if (motor == (MOT_SPIN_L) || motor == (MOT_SPIN_R))
    {
        if (param & 0x01)
        {
            spin_stop_delay = value;
            duration_movement |= SPIN_FLAG;
            cnt = 0;
        }
        else
            cnt = value;
        if (motor == MOT_SPIN_L)
            spin_left(value, spin_params_pwm);
        else
            spin_right(value, spin_params_pwm);
    }
}

/**
   \brief Parse the MOTORS_SET_CMD to command a motor with a specific number of movements.
   \param motor The motor to command
   \param pwm The PWM value

   The PWM can be used only for the spinning and the flippers.
 */
void motors_config(uint8_t const motor, uint8_t const pwm)
{
    if (motor == MOT_FLIPPERS)
    {
        flippers_params_pwm = pwm;
        flippers_PWM = pwm;
    }
    else if (motor == (MOT_SPIN_L) || motor == (MOT_SPIN_R))
    {
        spin_params_pwm = pwm;
        spin_PWM = pwm;
    }
}
/** Counter for flipper interrupt suspend. */
uint8_t static suspend_flippers_delay = 0;

/** Flipper interrupt suspend delay. */
#define SUSPEND_FLIPPERS_DELAY 2

/**
   Suspend the flippers interrupt for debouncing, see the flipper ISR for
   details about the problem.
 */
inline static void suspend_flippers_int(void)
{
    suspend_flippers_delay = SUSPEND_FLIPPERS_DELAY;
    PCICR &= ~_BV(PCIE1);
}

/**
   Resume the flippers interrupt.
 */
inline static void resume_flippers_int(void)
{
    /* Clear pending interrupts before enabling them. */
    PCIFR = _BV(PCIF1);
    PCICR |= _BV(PCIE1);
}

/**
   Resume interrupts when disabled for debouncing.
 */
inline static void manage_interrupts(void)
{
    if (suspend_flippers_delay)
    {
        suspend_flippers_delay--;
        if (!suspend_flippers_delay)
            resume_flippers_int();
    }
}

/**
 * \name Eyes functions
 *  @{ */
/** \brief Low level access to stop the eyes motor. */
static inline void stop_eyes_motor(void) __attribute__ ((always_inline));
void stop_eyes_motor(void)
{
    MOT_EYES_PT &= ~(MOT_EYES_MK | MOT_IEYES_MK);
}

/** \brief Low level access to start the eyes motor. */
static inline void run_eyes_motor(void) __attribute__ ((always_inline));
void run_eyes_motor(void)
{
    MOT_EYES_PT &= ~MOT_IEYES_MK;
    MOT_EYES_PT |= MOT_EYES_MK;
}

/** \brief Low level access to invert the eyes motor. */
static inline void invert_eyes_motor(void) __attribute__ ((always_inline));
void invert_eyes_motor(void)
{
    MOT_EYES_PT &= ~MOT_EYES_MK;
    MOT_EYES_PT |= MOT_IEYES_MK;
}

/**
   \brief Stop the eyes immediately.
   \ingroup eyes

   The eyes motor will stop immediately, even if the eyes are not in the open
   or close position. Use open_eyes or close_eyes instead if you want to stop
   them in a given position.
 */
void stop_eyes(void)
{
    gStatus.mot &= ~GSTATUS_MOT_EYES;
    eyes_move_counter = 0;
    duration_movement &= ~EYES_FLAG;
    stop_eyes_motor();
}

/**
   \brief Blink the eyes
   \ingroup eyes

   \param cnt number of movements before the flippers will stop.
   \note Any mouth command is cancelled.

   The eyes will start blinking until 'cnt' movements are executed. A movement
   is raising or lowering the eyelids. 'cnt' can be up to 256. If 'cnt' is
   null, waving will run indefinitely.
 */
void blink_eyes(uint8_t const cnt)
{
    gStatus.mot |= GSTATUS_MOT_EYES;
    eyes_move_counter = cnt;
    if (!(duration_movement & EYES_FLAG))
        eyes_stop_delay = EYES_TIMEOUT;
    run_eyes_motor();
}

/**
   \brief Open the eyes.
   \ingroup eyes

   Open the eyes if they are closed.

   The eyes can be in three different states and should have a different
   behavior in all states:
     - opened - no movement is required;
     - closed - just one movement is required to open the eyes;
     - unknown - two movements are required as the eyes can only be stopped
                 when they are closing; a spring is used to open them.

   We need to differentiate when the eyes are opened and stopped, and when
   they are opened but blinking. In the first case, we don't need to do
   anything. But in the second, we need to stop the eyes in the open position.
   To stop the eyes in the open state by sending the open_eyes_cmd when they're
   blinking, eyes_move_counter is initalised but the blink_eyes function isn't
   called.
   If the motor is running, it will execute two movements before stopping. If
   the motor isn't switched on, nothing happens but eyes_move_counter is set to
   a non zero value though they are no movements happening.

   \note The computer side (API, daemon) must be aware that the
   eyes_move_counter can be non zero with the motor switched off.
 */
void open_eyes(void)
{
    if ((PSW_EYES_PIN & PSW_EYES_O_MK) && (PSW_EYES_PIN & PSW_EYES_C_MK))
        /* Position unknown. */
        blink_eyes(2);
    else if (!(PSW_EYES_PIN & PSW_EYES_C_MK))
        /* Eyes are closed. */
        blink_eyes(1);
    else
        /* Eyes are opened but possibly blinking. */
        eyes_move_counter = 2;
}

/**
   \brief Close the eyes
   \ingroup eyes

   Close the eyes if they are opened.

   When the close_eyes_cmd is received, if the eyes are closed, nothing
   happens. But, when the eyes are blinking, this command must stop the eyes
   in the closed position. To do this, eyes_move_counter is initialised, but
   the blink_eyes function isn't called. If the command is received when the
   eyes are blinking, two movements are executed before the motor is stopped.
 */
void close_eyes(void)
{
    if (PSW_EYES_PIN & PSW_EYES_C_MK)
        /* Eyes are not closed. */
        blink_eyes(1);
    else
        /* Eyes are closed but possibly blinking. */
        eyes_move_counter = 2;
}

/**
   \brief Eyes position interrupt

   This interrupt stops the eyes when the desired number of movements have been
   executed. In order to stop the eyes quickly and block the motor, we need to
   invert the motor for a small duration.

   When the signal is switching from high to low (switch pushed), the eyes are
   closed. Due to the mechanical properties of the head gearbox, as soon as the
   signal rises from low to high the eyes are considered opened
   \note Only the eyes_closed_switch trig the interruption.

   So we count when the switch is pushed (eyes closed) and when the switch is
   released (eyes opened) to know the number of movements that occurs

   The movement counter eyes_move_counter is decreased and when null, the head
   motor is
   switched backwards and will be stopped by timer0 overflow after the
   EYES_BRAKING_DLY period.
 */
//ISR(SIG_PIN_CHANGE2)
ISR(PCINT2_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    if (!(PSW_EYES_PIN & PSW_EYES_C_MK))
        cond_flags.eyes_closed = 1;
    else
        cond_flags.eyes_closed = 0;
    
    if (!(duration_movement & EYES_FLAG))
        eyes_stop_delay = EYES_TIMEOUT;

    if (eyes_move_counter)
    {
        eyes_move_counter--;
        if (!eyes_move_counter)
        {
            stop_eyes_motor();
            //if (!(PSW_EYES_PIN & PSW_EYES_C_MK))
                invert_eyes_motor();
            eyes_stop_delay = EYES_BRAKING_DLY;
         }
    }
    /* Added some debouncing here for the oscillations, just in case it can
     * help but i never found his one necessary. */
    PCIFR = _BV(PCIF2);
}
/*! @} */

/**
 * \name Mouth functions
 *  @{ */
/** \brief Low level access to stop the mouth motor. */
static inline void stop_mouth_motor(void) __attribute__ ((always_inline));
void stop_mouth_motor(void)
{
    MOT_MOUTH_PT &= ~(MOT_MOUTH_MK | MOT_IMOUTH_MK);
}

/** \brief Low level access to start the mouth motor. */
static inline void run_mouth_motor(void) __attribute__ ((always_inline));
void run_mouth_motor(void)
{
    MOT_MOUTH_PT &= ~MOT_IMOUTH_MK;
    MOT_MOUTH_PT |= MOT_MOUTH_MK;
}

/** \brief Low level access to invert the mouth motor. */
static inline void invert_mouth_motor(void) __attribute__ ((always_inline));
void invert_mouth_motor(void)
{
    MOT_MOUTH_PT &= ~MOT_MOUTH_MK;
    MOT_MOUTH_PT |= MOT_IMOUTH_MK;
}

/**
   \brief Stop the mouth immediately.
   \ingroup mouth

   The mouth motor will stop immediately, even if the mouth is not in the open
   or close position. Use open_mouth or close_mouth instead if you want to stop
   them in a given position.
 */
void stop_mouth(void)
{
    gStatus.mot &= ~GSTATUS_MOT_MOUTH;
    mouth_move_counter = 0;
    duration_movement &= ~MOUTH_FLAG;
    stop_mouth_motor();
}

/**
   \brief Move the mouth.
   \ingroup mouth
   \note Any eye command is cancelled.
   \param cnt number of movements before the mouth will stop.

   The mouth will start blinking until 'cnt' movements are executed. A movement
   is raising or lowering the eyelids. 'cnt' can be up to 256. If 'cnt' is
   null, waving will run indefinitely.
 */
void move_mouth(uint8_t const cnt)
{
    gStatus.mot |= GSTATUS_MOT_MOUTH;
    mouth_move_counter = cnt;
    if (!(duration_movement & MOUTH_FLAG))
        mouth_stop_delay = MOUTH_TIMEOUT;
    run_mouth_motor();
}

/**
   \brief Open the mouth.
   \ingroup mouth

   This function open the mouth if it's not already open.
   The command is sent only if the mouth is not open or if the motor is
   running.
   We don't know the absolute mouth position. So, a command is sent
   with 2 movements, and a flag is set to specify the final position. When the
   final position is reached, the mouth_move_counter is reinitialized, and the
   motor will stop.
 */
void open_mouth(void)
{
    if (PSW_MOUTH_PIN & PSW_MOUTH_O_MK || MOT_MOUTH_PT & MOT_MOUTH_MK)
    {
        move_mouth(2);
        mouth_final_state = MOUTH_OPEN;
    }
}

/**
   \brief Close the mouth.
   \ingroup mouth

   This function close the mouth if it's not already closed.
   The command is sent only if the mouth is not closed or if the motor is
   running.
   We don't know the absolute mouth position. So, a command is sent
   with 2 movements, and a flag is set to specify the final position. When the
   final position is reached, the mouth_move_counter is reinitialized, and the
   motor will stop.
 */
void close_mouth(void)
{
    if (PSW_MOUTH_PIN & PSW_MOUTH_C_MK || MOT_MOUTH_PT & MOT_MOUTH_MK)
    {
        move_mouth(2);
        mouth_final_state = MOUTH_CLOSED;
    }
}

/**
   \brief Mouth position interrupt.

   This interrupt stops the mouth when the desired number of movements have
   been executed. In order to stop the mouth quickly and block the motor, we
   need to invert the motor for a small duration.

   There are 2 switches to detect the mouth poistion: mouth open and mouth
   closed. When any of the switches is pushed, a movement is counted.
   So we only count when the switch is pushed but not when it is released. As
   the interrupt triggers on a signal change, we need to suppress the release
   interrupt.
   The movement counter mouth_move_counter is decreased and when null, the head
   motor is switched backwards and will be stopped by timer0 overflow after the
   MOUTH_BRAKING_DLY period.
 */
//ISR(SIG_PIN_CHANGE0)
ISR(PCINT0_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    /* We only count when the switch is pushed, not released. */
    if (~PSW_MOUTH_PIN & PSW_MOUTH_MK)
    {
        if (!(duration_movement & MOUTH_FLAG))
            mouth_stop_delay = MOUTH_TIMEOUT;
        
        if (mouth_move_counter)
        {
            mouth_move_counter--;
            if (mouth_final_state != MOUTH_UNKNOWN)
            {
                if ((mouth_final_state == MOUTH_OPEN) && !(PSW_MOUTH_PIN & PSW_MOUTH_O_MK))
                    mouth_move_counter = 0;
                else if ((mouth_final_state == MOUTH_CLOSED) && !(PSW_MOUTH_PIN & PSW_MOUTH_C_MK))
                    mouth_move_counter = 0;
            }
            if (!mouth_move_counter)
            {
                mouth_final_state = MOUTH_UNKNOWN;
                stop_mouth_motor();
                invert_mouth_motor();
                mouth_stop_delay = MOUTH_BRAKING_DLY;
            }
        }
    }
    /* We need some debouncing here for the oscillations. */
    PCIFR = _BV(PCIF0);
}
/*! @} */

/**
 * \name Flippers functions
 *  @{ */
/** \brief Low level access to stop the flippers motor. */
static inline void stop_flippers_motor(void) __attribute__ ((always_inline));
void stop_flippers_motor(void)
{
    flippers_PWMMask &= ~MOT_FLIPPERS_FW_MK;
    MOT_FLIPPERS_BW_PT &= ~MOT_FLIPPERS_BW_MK;
    MOT_FLIPPERS_FW_PT &= ~MOT_FLIPPERS_FW_MK;
}

/** \brief Low level access to start the flippers motor. */
static inline void run_flippers_motor(void) __attribute__ ((always_inline));
void run_flippers_motor(void)
{
    MOT_FLIPPERS_BW_PT &= ~MOT_FLIPPERS_BW_MK;
    MOT_FLIPPERS_FW_PT |= MOT_FLIPPERS_FW_MK;
}

/** \brief Low level access to invert the flippers motor. */
static inline void invert_flippers_motor(void) __attribute__ ((always_inline));
void invert_flippers_motor(void)
{
    MOT_FLIPPERS_FW_PT &= ~MOT_FLIPPERS_FW_MK;
    MOT_FLIPPERS_BW_PT |= MOT_FLIPPERS_BW_MK;
}

/**
   \brief Stop the flippers immediately.
   \ingroup flippers

   The flippers motor will stop immediately, even if the flippers are not in
   the up or down position. Use raise_flippers or lower_flippers instead if you
   want to stop them in a given position.
 */
void stop_flippers(void)
{
    gStatus.mot &= ~GSTATUS_MOT_WINGS;
    flippers_move_counter = 0;
    duration_movement &= ~FLIPPERS_FLAG;
    stop_flippers_motor();
}

/**
   \brief Start waving the flippers up and down.
   \ingroup flippers
   \param cnt number of movements before the flippers will stop.
   \param pwm pwm value between 1 (slow) and 5 (fast).

   The flippers will start waving until 'cnt' movements have been executed. A
   movement is raising or lowering the flippers. 'cnt' can be up to 256. If
   'cnt' is null, waving will run indefinitely. pwm can be used to change the
   speed of the motor although it's more directed to the power delivered to the
   motor. It's possible that the smallest PWM values won't deliver enough power
   to even start the motor.
 */
void wave_flippers(uint8_t const cnt, uint8_t const pwm)
{
    gStatus.mot |= GSTATUS_MOT_WINGS;
    flippers_move_counter = cnt;
    flippers_PWM = pwm;
    if (!(duration_movement & FLIPPERS_FLAG))
        flippers_stop_delay = FLIPPERS_TIMEOUT;
    MOT_FLIPPERS_BW_PT &= ~MOT_FLIPPERS_BW_MK;
    flippers_PWMMask |= MOT_FLIPPERS_FW_MK;
    PORTB |= flippers_PWMMask;
}

/**
   \brief Reset flippers.
   \ingroup flippers

   This function resets the flippers position. flippers_timer is used to
   determine the flippers positions. Times needed to raise or lower the
   flippers are not the same. If 2 movements are executed (up - down - up),
   flippers_timer allows to determine the shorter time, and get if the flippers
   are up or low at the end.
   */
void reset_flippers(void)
{
    flippers_timer = FLIP_TIMER_INIT;
    /* The first movement is to be sure the timer doesn't start counting from
     * any unknown postion and that the motors are in regime. */
    wave_flippers(2, 5);
}

/**
   \brief Raise flippers.
   \ingroup flippers

   The condition to raise the flippers is that they aren't already in the upper
   position. If this condition is respected, a single movement is executed by
   the wave_flippers function.

   If the flippers were in the higher position and have been lowered manually,
   the flippers can't be raised directly. They must pass by the low position
   (lower_flippers function) before being able to be raised.

   \note If the flippers are in a medium position, it's possible that they'll
   first move low before raising. Implementing conditions to be able to raise
   the flippers immeditely is not trivial and increases the code size too much
   compared to the benefit. This would require changing the motor direction
   when two consecutive commands are identical. Furthermore the absolute
   position when the motor is switched off is important. So, the braking delay
   couldn't be the same in both directions.
   */
void raise_flippers(void)
{
    if (gStatus.pos == 0)
        wave_flippers(1,5);
    else
        flippers_move_counter = 2;
}

/**
   \brief Lower flippers.
   \ingroup flippers

   The condition to lower the flippers is that they aren't already in the lower
   position. If this condition is respected, a single movement is executed by
   the wave_flippers function.

   \note Has the same limitations as raise_flippers.
 */
void lower_flippers(void)
{
    if (gStatus.pos == 1)
        wave_flippers(1,5);
    else
        flippers_move_counter = 2;
}

/**
   \brief Flippers position interrupt.

   This interrupt stops the flippers when the desired number of movements have
   been executed. In order to stop the flippers quickly and block the motor, we
   need to invert the motor for a small duration.

   When the signal is switching from high to low (switch pushed), the number of
   movements is decreased and when null, the flippers motor is switched
   backwards and will be stopped by timer0 overflow after the
   FLIPPERS_BRAKING_DLY period. So we only count when the switch is pushed but
   not when it is released. As the interrupt triggers on a signal change, we
   need to suppress the release interrupt.

   When adding LED PWM, we noticed that the flipper position interrupt was
   triggered by the LED signal (PC2) while the switch signal (PC1) was around
   VDD/2. The capacitor that filters the switch signal (PC1) also decreases the
   rising time so the uncertainty period between which the input pin (PC1)
   could change from low to high is quite long. During this time, it seems
   changes of PC2 can affect PC1 directly, probably through crosstalks, and
   this triggers this interrupt. In order to filter out this glitches, we
   disabled the interrupts for a couple milliseconds.
 */
//ISR(SIG_PIN_CHANGE1)
ISR(PCINT1_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    suspend_flippers_int();
    /* We only count when the switch is pushed, not released. */
    if (~PSW_FLIPPERS_PIN & PSW_FLIPPERS_MK)
    {
        /* This test prevents the toggle when the flippers are pushed. The
         * flippers motor must be switched on (flippers_PWM <> 0) to toggle the
         * position */
        gStatus.pos ^= GSTATUS_POS_W0;  /* toggle wings position */

        if (flippers_timer) /* if we need to reposition the flippers */
        {
            if (flippers_move_counter == 1)
            {
                if ((flippers_previous_timer) && (flippers_timer <
                        (flippers_previous_timer - FLIPPERS_RESETTIMER_HYST)))
                {
                    /* flippers are down */
                    /* Position reached so init gStatus.pos bit to lower
                     * position */
                    gStatus.pos &= ~GSTATUS_POS_W0;
                    flippers_timer = 0;
                    flippers_previous_timer = 0;
                }
                else
                {
                    /* Unknow position, execute another movement to determine
                     * it. */
                    flippers_stop_delay = FLIPPERS_TIMEOUT;
                    flippers_previous_timer = flippers_timer;
                    flippers_timer = FLIP_TIMER_INIT;
                    return;
                }
            }
            else
                flippers_timer = FLIP_TIMER_INIT;
        }
        
        if (!(duration_movement & FLIPPERS_FLAG))
            flippers_stop_delay = FLIPPERS_TIMEOUT;

        if (flippers_move_counter)
        {
            flippers_move_counter--;
            if (!flippers_move_counter)
            {
                stop_flippers_motor();
                invert_flippers_motor();
                flippers_stop_delay = FLIPPERS_BRAKING_DLY;
            }
        }
    }
}
/*! @} */

/**
 * \name Spinning functions
 *  @{ */
/** \brief Low level access to stop the spinning motor. */
static inline void stop_spin_motor(void) __attribute__ ((always_inline));
void stop_spin_motor(void)
{
    spin_PWM_mask &= ~MOT_SPIN_MK;
    MOT_SPIN_PT &= ~MOT_SPIN_MK;
}
/**
   \brief Stop spinning immediately.
   \ingroup spin
 */
void stop_spinning(void)
{
    gStatus.mot &= ~GSTATUS_MOT_SPIN_MK;
    spin_move_counter = 0;
    duration_movement &= ~SPIN_FLAG;
    stop_spin_motor();
}

/**
   \brief Spin left for the \c angle amount.
   \param angle Angle to turn, in 90° unit.
   \param pwm PWM value assigned to the spinning.
   \ingroup spin
 */
void spin_left(uint8_t const angle, uint8_t const pwm)
{
    gStatus.mot |= GSTATUS_MOT_SPINL;
    spin_move_counter = angle;
    /* If the rotation direction is changing and we are not stopped exactly on
     * the switch (position switch not pressed), we need to increment the angle
     * value to prevent counting the first switch detection that will happen as
     * soon as the rotation starts. */
    if ((spin_direction == RIGHT) && (PSW_SPIN_PIN & PSW_SPIN_MK))
        if (spin_move_counter)
            spin_move_counter++;
    spin_direction = LEFT;
    if (!(duration_movement & SPIN_FLAG))
        spin_stop_delay = SPIN_TIMEOUT;
    spin_PWM = pwm;
    spin_PWM_mask &= ~MOT_SPIN_R_MK;
    spin_PWM_mask |= MOT_SPIN_L_MK;
    PORTB |= portB_PWM_mask;
}

/**
   \brief Spin right for the \c angle amount.
   \param angle Angle to turn, in 90° unit.
   \param pwm PWM value assigned to the spinning.
   \ingroup spin
 */
void spin_right(uint8_t const angle, uint8_t const pwm)
{
    gStatus.mot |= GSTATUS_MOT_SPINR;
    spin_move_counter = angle;
    /* If the rotation direction is changing and we are not stopped exactly on
     * the switch (position switch not pressed), we need to increment the angle
     * value to prevent counting the first switch detection that will happen as
     * soon as the rotation starts. */
    if ((spin_direction == LEFT) && (PSW_SPIN_PIN & PSW_SPIN_MK))
        if (spin_move_counter)
            spin_move_counter++;
    spin_direction = RIGHT;
    if (!(duration_movement & SPIN_FLAG))
        spin_stop_delay = SPIN_TIMEOUT;
    spin_PWM = pwm;
    spin_PWM_mask &= ~MOT_SPIN_L_MK;
    spin_PWM_mask |= MOT_SPIN_R_MK;
    PORTB |= portB_PWM_mask;
}
/**
   \brief Spin position interrupt.

   This interrupt stops the spinning motor when the desired number of movements
   have been executed.

   The switch is pushed each 90 degrees. We set the interrupt in falling edge
   mode so there's no interrupt generated when the switch is released.
 */
//ISR(SIG_INTERRUPT1)
ISR(INT1_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    if (!(duration_movement & SPIN_FLAG))
        spin_stop_delay = SPIN_TIMEOUT;
    if (spin_move_counter)
    {
        spin_move_counter--;

        if (!spin_move_counter)
        {
            spin_PWM_mask &= ~MOT_SPIN_MK;
            stop_spin_motor();
            spin_stop_delay = SPIN_BRAKING_DLY;
            if (spin_direction == LEFT)
                spin_PWM_mask |= MOT_SPIN_R_MK;
            else
                spin_PWM_mask |= MOT_SPIN_L_MK;
        }
    }
}
/*! @} */

/**
   \brief Periodic routine that controls the PWM of the spinning, flippers and
   all motors braking.
   \fn motor_control
   \ingroup movements

   This function should be called from a timer interrupt that will fix the PWM
   resolution.
   The PWM period will be PWM_PERIOD times * the timer period.
   The braking time is dependant on the timer period too.
 */
/** Number of timer periods that constitutes the PWM period. This is also the
 * number of PWM values you can get. */
#define PWM_PERIOD 5
void motor_control(void)
{
    static uint8_t pwm_tim;
    /* Flippers PWM
     * Pulse is stopped here */
    if (pwm_tim == flippers_PWM)
        MOT_FLIPPERS_FW_PT &= ~MOT_FLIPPERS_FW_MK;

    /* Spin PWM
     * Pulse is stopped here */
    if (pwm_tim == spin_PWM)
        MOT_SPIN_PT &= ~MOT_SPIN_MK;

    /* PWM motor management
     * Pulse is set here when pwm_tim is at maximum and is
     * reset when pwm_tim equals the PWM value set by the user
     */
    if (pwm_tim++ == PWM_PERIOD)
    {
        pwm_tim = 0;
        PORTB |= portB_PWM_mask; /* spin and flippers */
    }

    /* Flippers timer to stop the flippers in any position */
    if (flippers_timer)
    {
        flippers_timer--;
        if (!flippers_timer)
        {
            stop_flippers_motor();
        }
    }

    /* Flippers motor timeout */
    if (flippers_stop_delay)
    {
        flippers_stop_delay--;
        if (!flippers_stop_delay)
        {
            stop_flippers();
            flippers_PWM = 0;
        }
    }

    /* eyes motor timeout */
    if (eyes_stop_delay)
    {
        eyes_stop_delay--;
        if (!eyes_stop_delay)
        {
            stop_eyes();
        }
    }

    /* mouth motor timeout */
    if (mouth_stop_delay)
    {
        mouth_stop_delay--;
        if (!mouth_stop_delay)
        {
            stop_mouth();
        }
    }

    /* spin motor timeout */
    if (spin_stop_delay)
    {
        spin_stop_delay--;
        if (!spin_stop_delay)
        {
            stop_spinning();
        }
    }

    /* Handle interrupt suspend. */
    manage_interrupts();
}
