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

/* $Id: standalone.c 2238 2008-10-07 09:06:20Z jaguarondi $ */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "standalone.h"
#include "communication.h"
#include "parser.h"
#include "global.h"
#include "sensors.h"
#include "motors.h"
#include "ir.h"
#include "led.h"
#include "version.h"
#include "common/remote.h"
#include "config.h"

/*
 * Event manager
 */

uint8_t event_timer, event_manager_flag;
struct
{
    uint8_t time;
    uint8_t command[CMD_SIZE];
    uint8_t *addr;
    uint8_t nbr;
} action;

void loadAction(void)
{
    uint8_t i;

    action.time = eeprom_read_byte(action.addr++);
    for (i = 0; i < CMD_SIZE; i++)
        action.command[i] = eeprom_read_byte(action.addr++);
}

void actionManager(void)
{
    while ((event_timer <= action.time) && (action.time != END_OF_ACTIONS))
    {
        /* Execute the pre-loaded action */
        uint8_t i = 0;
        while (i < CMD_SIZE)
        {
            parse_cmd(&action.command[i]);
            i += 1 + (action.command[i] >> 6);
        }
        /* then load the next action for execution at action.time. */
        loadAction();
    }
    event_manager_flag = 0;
}

void launchActions(const uint8_t * addr)
{
    event_timer = eeprom_read_byte(addr);
    action.addr = (uint8_t *) addr;
    loadAction();
    event_manager_flag = 1;
}

void eventTriggering(void)
{
    /* Startup */
    if (cond_flags.startup)
    {
        cond_flags.startup = 0;
        launchActions((const uint8_t *)&startup_e);
        tux_ir_id = gStatus.lightL; /* XXX remove this when fixing the greeting
                                       function */
    }

    /* Head button */
    else if (cond_flags.head)
    {
        cond_flags.head = 0;
        launchActions((const uint8_t *)&head_e);
    }

    /* Left flipper button */
    else if (cond_flags.left_flip)
    {
        cond_flags.left_flip = 0;
        launchActions((const uint8_t *)&left_flip_e);
    }

    /* Right flipper button */
    else if (cond_flags.right_flip)
    {
        cond_flags.right_flip = 0;
        launchActions((const uint8_t *)&right_flip_e);
    }

    /* Start charging */
    else if (cond_flags.charger_start)
    {
        cond_flags.charger_start = 0;
        launchActions((const uint8_t *)&charger_start_e);
    }

    /* Unplug condition */
    else if (cond_flags.unplug)
    {
        cond_flags.unplug = 0;
        launchActions((const uint8_t *)&unplug_e);
    }

    /* RF connection */
    else if (cond_flags.rf_conn)
    {
        cond_flags.rf_conn = 0;
        launchActions((const uint8_t *)&rf_conn_e);
    }

    /* RF disconnection */
    else if (cond_flags.rf_disconn)
    {
        cond_flags.rf_disconn = 0;
        launchActions((const uint8_t *)&rf_disconn_e);
    }

    else if (ir_send_flg)
    {
        if (cond_flags.tux_recog_cnt)
        {
            if (cond_flags.tux_recog == 0xF1)
                irSendRC5(0x08, tux_ir_id);
            else
                irSendRC5(0x0C, tux_ir_id);
            cond_flags.tux_recog_cnt--;
        }
        else
            irSendRC5(0x04, tux_ir_id);
        ir_send_flg = 0;
    }
}

void standalone_behavior(void)
{
    static uint8_t mov_nbr = 1;
    uint8_t ir_command, ir_toggle;

    if (event_manager_flag)
        actionManager();
    if (!event_timer)
        eventTriggering();

    /* Disable spinning when plugged */
    if ((gStatus.sw & GSTATUS_POWERPLUGSW_MK))
    {
        stop_spinning();         /* flush the spinning commands */
        spin_move_counter = 0;

        /* XXX check the prod firmware for sleep mode, things have been added
         * here. */
    }

    /* IR signal processing */
    /* put here things that should be done only once when a button is pushed */
    if (gStatus.ir != ir_oldvalue && gStatus.ir & GSTATUS_IR_VALID)
    {
        ir_command = gStatus.ir & GSTATUS_IR_COMMAND;
        ir_toggle = gStatus.ir & GSTATUS_IR_TOGGLE;
        ir_oldvalue = gStatus.ir;

        /* IR feedback signal */
        if (tux_config.ir_feedback)
            leds_toggle(2, 4);

        /* ALT KEYS */
        if (alt_mode)
        {
            /* Entering remote mode */
            if (ir_command == K_STARTVOIP)
            {
                if (!remote_mode)
                    remote_mode = 1;
                else
                    remote_mode = 0;
            }
        }

        /* Alt key pressed
         * Leave this test after the ALT KEYS otherwise alt_mode is reset
         * before the other commands are processed when you push the second key
         * */
        if (ir_command == K_ALT)
            alt_mode = 1;
        else
            alt_mode = 0;

        /* Remote control mode */
        if (remote_mode)
        {
            switch (ir_command)
            {
            case K_0:
            case K_1:
            case K_2:
            case K_3:
            case K_4:
            case K_5:
            case K_6:
            case K_7:
            case K_8:
            case K_9:
                mov_nbr = ir_command;
                break;
            case K_UP:
                blink_eyes(mov_nbr);
                break;
            case K_OK:
                move_mouth(mov_nbr);
                break;
            case K_DOWN:
                wave_flippers(mov_nbr, 5);
                break;
            case K_STANDBY:
                queue_cmd_p(SLEEP_CMD, SLEEPTYPE_QUICK, 0, 0);
                break;
            case K_LEFT:
                spin_left(mov_nbr, spin_PWM);
                break;
            case K_RIGHT:
                spin_right(mov_nbr, spin_PWM);
                break;
            case K_STOP:
                stop_spinning();
                stop_flippers();
                stop_mouth();
                break;
            case K_MUTE:
                tux_config.ir_feedback = !tux_config.ir_feedback;
                break;
            case K_CHANNELPLUS:
                led_pulse(LED_RIGHT, 1, 0);
                break;
            case K_VOLUMEPLUS:
                led_pulse(LED_LEFT, 1, 0);
                break;
            case K_FASTREWIND:
                if (spin_PWM)
                    spin_PWM--;
                break;
            case K_FASTFORWARD:
                if (spin_PWM < 5)
                    spin_PWM++;
                break;
            case K_PREVIOUS:
                if (flippers_PWM)
                    flippers_PWM--;
                break;
            case K_NEXT:
                if (flippers_PWM < 5)
                    flippers_PWM++;
                break;
            }
        }
    }
}
