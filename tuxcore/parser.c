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

/* $Id: parser.c 2238 2008-10-07 09:06:20Z jaguarondi $ */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "communication.h"
#include "global.h"
#include "sensors.h"
#include "motors.h"
#include "ir.h"
#include "led.h"
#include "version.h"

/**
 * commandParser parse commands received by the twi interface and trigger the
 * associated functions
 */
void parse_cmd(uint8_t cmd[CMD_SIZE])
{
    uint8_t i;

    /* Check new conditions and update status from tuxaudio */
    if (cmd[0] == SEND_AUDIOSENSORS_CMD)
    {
        if ((cmd[1] & STATUS_HEADBTN_MK)
            && !(gStatus.sw & GSTATUS_HEADBTN_MK))
            cond_flags.head = 1;
        if ((cmd[1] & STATUS_LEFTWINGBTN_MK)
            && !(gStatus.sw & GSTATUS_LEFTWINGBTN_MK))
            cond_flags.left_flip = 1;
        if ((cmd[1] & STATUS_RIGHTWINGBTN_MK)
            && !(gStatus.sw & GSTATUS_RIGHTWINGBTN_MK))
            cond_flags.right_flip = 1;
        if ((cmd[1] & STATUS_CHARGER_MK)
            && !(gStatus.sw & GSTATUS_CHARGER_MK))
            cond_flags.charger_start = 1;
        if (!(cmd[1] & STATUS_POWERPLUGSW_MK)
            && (gStatus.sw & GSTATUS_POWERPLUGSW_MK))
            cond_flags.unplug = 1;
        if ((cmd[1] & STATUS_RF_MK) && !(gStatus.sw & GSTATUS_RF_MK))
        {
            cond_flags.rf_conn = 1;
            cond_flags.rf_disconn = 0;
        }
        if (!(cmd[1] & STATUS_RF_MK) && (gStatus.sw & GSTATUS_RF_MK))
        {
            cond_flags.rf_conn = 0;
            cond_flags.rf_disconn = 1;
        }
        gStatus.sw = cmd[1];
        gStatus.audio_play = cmd[2];
        gStatus.audio_status = cmd[3];
        return;
    }
    /* Ping */
    else if (cmd[0] == PING_CMD)
    {
        pingCnt = cmd[1];
        return;
    }
    /* Sound */
    else if (cmd[0] == PLAY_SOUND_CMD)
    {
        /* Forward the cmd to the audio CPU. */
        queue_cmd(cmd);
        return;
    }
    else if (cmd[0] == MUTE_CMD)
    {
        /* Forward the cmd to the audio CPU. */
        queue_cmd(cmd);
        return;
    }
    /* Sleep mode */
    else if (cmd[0] == SLEEP_CMD)
    {
        cond_flags.sleep = true;
        return;
    }
    /* Version */
    else if (cmd[0] == INFO_TUXCORE_CMD)
    {
        uint8_t *p = (uint8_t *) &tag_version;
        uint8_t info[12];

        for (i = 0; i < 12; i++)
            info[i] = pgm_read_byte(p++);
        queue_cmd(&info[0]);
        queue_cmd(&info[4]);
        queue_cmd(&info[8]);
        return;
    }
    /* Reset condition flags */
    else if (cmd[0] == COND_RESET_CMD)
    {
        uint8_t *addr = (uint8_t *) & cond_flags;

        for (i = 0; i < COND_RESET_NBR; i++)
            *addr++ = 0;
        return;
    }
    else if (cmd[0] == LED_FADE_SPEED_CMD)
    {
        led_set_fade_speed(cmd[1], cmd[2], cmd[3]);
    }
    else if (cmd[0] == LED_SET_CMD)
    {
        led_set_intensity(cmd[1], cmd[2]);
    }
    else if (cmd[0] == IR_SEND_RC5_CMD)
    {
        irSendRC5(cmd[1], cmd[2]);
    }
    else if (cmd[0] == MOTORS_CONFIG_CMD)
    {
        motors_config(cmd[1], cmd[2]);
    }
    /* Leds */
    else if (cmd[0] == LED_PULSE_RANGE_CMD)
    {
        led_pulse_range(cmd[1], cmd[2], cmd[3]);
    }
    /* Move */
    else
    {
        if (cmd[0] == MOTORS_SET_CMD)
        {
            motors_run(cmd[1], cmd[2], cmd[3]);
        }
        else if (cmd[0] == LED_PULSE_CMD)
        {
            led_pulse(cmd[1], cmd[2], cmd[3]);
        }

        /* Deprecated functions, though they can be kept for the standalone as
         * they're simpler than the other LED functions. */
        else if (cmd[0] == LED_ON_CMD)
        {
            led_set_intensity(LED_BOTH, 0xFF);
        }
        else if (cmd[0] == LED_OFF_CMD)
        {
            led_set_intensity(LED_BOTH, 0x0);
        }
        else if (cmd[0] == LED_TOGGLE_CMD)
        {
            leds_toggle(cmd[1], cmd[2]);
        }
        /* Moves */
        else if (cmd[0] == BLINK_EYES_CMD)
        {
            blink_eyes(cmd[1]);
        }
        else if (cmd[0] == STOP_EYES_CMD)
        {
            stop_eyes();
        }
        else if (cmd[0] == OPEN_EYES_CMD)
        {
            open_eyes();
        }
        else if (cmd[0] == CLOSE_EYES_CMD)
        {
            close_eyes();
        }
        else if (cmd[0] == MOVE_MOUTH_CMD)
        {
            move_mouth(cmd[1]);
        }
        else if (cmd[0] == OPEN_MOUTH_CMD)
        {
            open_mouth();
        }
        else if (cmd[0] == CLOSE_MOUTH_CMD)
        {
            close_mouth();
        }
        else if (cmd[0] == STOP_MOUTH_CMD)
        {
            stop_mouth();
        }
        else if (cmd[0] == WAVE_WINGS_CMD)
        {
            wave_flippers(cmd[1], cmd[2]);
        }
        else if (cmd[0] == RAISE_WINGS_CMD)
        {
            raise_flippers();
        }
        else if (cmd[0] == LOWER_WINGS_CMD)
        {
            lower_flippers();
        }
        else if (cmd[0] == RESET_WINGS_CMD)
        {
            reset_flippers();
        }
        else if (cmd[0] == STOP_WINGS_CMD)
        {
            stop_flippers();
        }
        else if (cmd[0] == SPIN_LEFT_CMD)
        {
            spin_left(cmd[1], cmd[2]);
        }
        else if (cmd[0] == SPIN_RIGHT_CMD)
        {
            spin_right(cmd[1], cmd[2]);
        }
        else if (cmd[0] == STOP_SPIN_CMD)
        {
            stop_spinning();
        }
        /* Undefined commands */
        else
            return;                 /* simply drop it */

        /* Send an updated status here for functions that need it */
        updateStatusFlag = 1;
    }
}

void parse_received_cmd(void)
{
    uint8_t cmd[CMD_SIZE];
    if (get_cmd(cmd))
        parse_cmd(cmd);
}
