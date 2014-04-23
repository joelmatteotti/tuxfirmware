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

/* $Id: main.c 5749 2009-10-20 07:03:26Z ks156 $ */

/** \file main.c
    \brief Tuxcore main functions
*/

/** \mainpage Tuxcore firmware documentation of the Tux Droid open source robot

\section SVN SVN repository
http://svn.tuxisalive.com/firmware/tuxcore/trunk

\section flowchart Flowchart
\image html main_flowchart.png
*/

//#define __AVR_LIBC_DEPRECATED_ENABLE__


#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include "global.h"
#include "sensors.h"
#include "motors.h"
#include "ir.h"
#include "led.h"
#include "i2c.h"
#include "communication.h"
#include "standalone.h"
#include "parser.h"
#include "config.h"
#include "debug.h"

/*
 * DEBUG: if stack debugging is enabled in debug.h, this macro initializes the
 * ram with a constant value in order to catch stack overflow by examining the
 * stack at a breakpoint.
 */
DBG_STACK_INIT

/**
 * \name Software timers
 * The main tick has a period of 4ms and is driven by a hardware timer
 * interrupt. Counters are used to get 100ms and 1s ticks.
 *  @{ */
/** Flag set each 4ms. */
static bool t4ms_flag;
/** Flag set each 100ms. */
static bool t100ms_flag;
/** Flag set each 1s. */
static bool t1s_flag;
/** f4ms counter used to get the 100ms tick. */
static uint8_t t4ms_cnt;
/** 100ms counter used to get the 1s tick. */
static uint8_t t100ms_cnt;
/*! @} */

static void initIO(void);
static void updateStatus(void);
static void sleep(void);

/*
 * External variables
 * XXX to be removed from here
 */
volatile GSTATUS gStatus;

/**
   \brief Main tick timer intitialization
   \fn main_tick_init

   Main tick period: 4ms
   Prescaler: 256
   The timer clock will be F_CPU/256 = 31.250 kHz
   CTC mode of operation
   Compare value: (F_CPU * 4) / (256UL * 1000)) = 125
*/
/** Compare value of the main tick timer. */
#define MAIN_TICK_COMPARE    125
static void main_tick_init(void)
{
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS22) | _BV(CS21);
    TCNT2 = 0x00;
    OCR2A = MAIN_TICK_COMPARE;
    TIMSK2 = _BV(OCIE2A);
}

/**
   \brief Main tick timer interrupt.
   This interrupt is called each 4ms on the timer2 compare match. 100ms and 1s
   ticks are also computed from software counters. Flags are set on each
   different ticks: 4ms flag, 100ms flag and 1s flag.
 */
//ISR(SIG_OUTPUT_COMPARE2A)
ISR(TIMER2_COMPA_vect) /* 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    t4ms_cnt++;
    t4ms_flag = true;
    if (t4ms_cnt == 25)
    {
        t4ms_cnt = 0;
        t100ms_flag = true;
        if (++t100ms_cnt == 10)
        {
            t100ms_cnt = 0;
            t1s_flag = true;
        }
    }
}

/*
 * Main code
 */
int main(void)
{
    /* Initialization, config should be initialized first */
    config_init();
    init_movements();
    initIR();
    main_tick_init();
    initIO();
    sensors_init();
    led_init();

    /* I2C communication initialization */
    communication_init();

    /* Enable IR receiver */
    irGetRC5();

    /*
     * Main loop
     */
    for (;;)
    {
        if (t4ms_flag)
        {
            t4ms_flag = false;
            motor_control();
            if (sensorsUpdate)
                sensors_control();

            /* Communication */
            //    commandProcessFlag = 1;

            if (ir_delay)
                ir_delay--;

            /* Led blinking. */
            led_control(cond_flags.eyes_closed);
            /* Refresh the leds. When the eyes are closed, the leds are
             * switched off. They'll be refreshed again with the buffered value
             * when the eyes will reopen. */
        }
        if (t100ms_flag)
        {
            t100ms_flag = false;
            updateStatusFlag = 1;
            if (event_timer)
            {
                event_timer--;
                event_manager_flag = 1;
            }
        }
        /*
         * Communication: updating status, receiving and sending commands
         */
        /* We don't send status when entering or leaving sleep mode. */
        if (updateStatusFlag)
        {
            updateStatusFlag = 0;
            if (!cond_flags.sleep)
            {
                updateStatus();
            }
        }

        /* send pending pongs */
        if (pingCnt && cmds_empty())
        {
            pingCnt--;
            queue_cmd_p(PONG_CMD, pingCnt, 0, 0);
        }
        /* Parse and execute received commands. */
        parse_received_cmd();

        standalone_behavior();  /* standalone behavior manager */

        /* Entering sleep mode. */
        /* Wait for standalone actions to be done. */
        /* XXX check event timer is necessary here */
        if (cond_flags.sleep)
        {
            sleep();
            cond_flags.sleep = false;
        }
    }
}

/*
 * I/O initialisation
 *
 */
static void initIO(void)
{
    /* Set charger inhibit line as output */
    CHARGER_INH_DDR |= CHARGER_INH_MK;

    /* Set output for Led's */
    LED_DDR |= LED_MK;

    /* set external I/O as pull-up */
    EXIO_PT |= EXIO_MK;

    PCIFR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2); /* clear pending interrupts */
    sei();                      /* enable global interrupts */
}

/*
 * Close IO pins to minimize power consumption.
 *
 * CHARGER_INH and IR stay configured, they're just turned off.
 */
static void closeIO(void)
{
    PCICR = 0;
    EIMSK = 0;
    ADCSRA = 0;
    led_shutdown();
    turnIrOff();
}

static void updateStatus(void)
{
    queue_cmd_p(STATUS_SENSORS1_CMD, gStatus.sw, gStatus.audio_play,
               gStatus.audio_status);
    queue_cmd_p(STATUS_PORTS_CMD, PINB, PINC, PIND);
    queue_cmd_p(STATUS_POSITION1_CMD, eyes_move_counter, mouth_move_counter,
               flippers_move_counter);
    queue_cmd_p(STATUS_POSITION2_CMD, spin_move_counter, gStatus.pos, 
                gStatus.mot);
    if (led_f)
    {
        led_f = false;
        queue_cmd_p(STATUS_LED_CMD, left_led.status.intensity,
                   right_led.status.intensity,
                   left_led.status.fading |
                   (left_led.status.pulsing << 1) |
                   (right_led.status.fading << 2) |
                   (right_led.status.pulsing << 3) |
                   /* Also add the mask. */
                   (cond_flags.eyes_closed << 4));
    }
    if (sensorsStatus & LIGHT_FLAG)     /* send light measurement */
    {
        sensorsStatus &= ~LIGHT_FLAG;
        queue_cmd_p(STATUS_LIGHT_CMD, gStatus.lightH, gStatus.lightL, gStatus.lightM);
    }
    if (sensorsStatus & BATTERY_FLAG)   /* send battery measurement */
    {
        sensorsStatus &= ~BATTERY_FLAG;
        queue_cmd_p(STATUS_BATTERY_CMD, gStatus.batteryH, gStatus.batteryL, gStatus.batteryS);
    }
    if (ir_f)                   /* send received ir signals */
    {
        ir_f--;
        queue_cmd_p(STATUS_IR_CMD, gStatus.ir, ir_f, gStatus.ir);
    }
    if (gerror)
        queue_cmd_p(GERROR_CMD, TUXCORE_CPU_NUM, gerror, 0);
    sensorsUpdate |= STATUS_SENT;
}

/*
 * Sleep function
 *
 * Configures all blocks and pins to minimize power,
 * sleep until a wake-up condition is met,
 * restore the configuration to continue normal operation.
 */

static void sleep(void)
{
    uint8_t PRR_bak;
    uint8_t status_bak;
    /* Set power savings configuration. */

    cli();
    closeIO();
    sensors_disable();
    powersave_movements();
    stop_spinning();
    stop_mouth();
    stop_flippers();
    TWCR = _BV(TWINT);
    PRR_bak = PRR;
    PRR = _BV(PRTWI) | _BV(PRTIM2) | _BV(PRTIM0) | _BV(PRTIM1) | _BV(PRSPI) |
        _BV(PRUSART0) | _BV(PRADC);

    /* Sleep. */
    /* Set pin change interrupt on head button */
    PCMSK0 = SW_HD_MK;
    PCIFR = _BV(PCIE0);
    PCICR |= _BV(PCIE0);

    status_bak = gStatus.pos;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();

    /* Re-configure in normal mode. */
    PRR = PRR_bak;
    init_movements();
    initIR();
    main_tick_init();
    initIO();
    sensors_init();
    led_init();
    communication_init();
    irGetRC5();
    gStatus.pos = status_bak;

    /* Wait 200ms for the pull-up to rise before next standalone behavior
     * otherwise position switches signals are wrong */
    event_timer = 2;
}
