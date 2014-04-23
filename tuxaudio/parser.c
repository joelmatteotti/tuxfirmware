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

/* $Id: parser.c 2288 2008-10-09 08:16:21Z jaguarondi $ */

#include <stddef.h>
#include <avr/interrupt.h>

#include "parser.h"
#include "communication.h"
#include "hardware.h"
#include "misc.h"
#include "varis.h" /* XXX remove this one */
#include "flash.h" /* XXX remove this one */

/**
 * Parse a cmd received by the computer or by tuxcore and drop it if it
 * shouldn't be forwarded.
 * \return True if the command has been parsed and shouldn't be forwarded,
 * false if it should be forwarded.
 */
bool parse_cmd(uint8_t *cmd)
{
    /* Drop everything when sleep should be entered. */
    if (sleep_f )
    {
        return true;
    }

    /*
     * Commands that shouldn't be forwarded.
     */
    /* Empty commands are dropped */
    if (cmd[0] == NULL_CMD)
    {
    }
    /* Version */
    else if (cmd[0] == INFO_TUXAUDIO_CMD)
    {
        send_info();
    }
    else if (cmd[0] == PLAY_SOUND_CMD)
        /* param: cmd[1] : sound number */
        /* cmd[2] : mic sound intensity  */
    {
        /* Drop the cmd if a sound is already playing */
        if (!(flashPlay || programmingFlash))
        {
            audioLevel = cmd[2];
            soundToPlay = cmd[1];
            flashPlay = 1;
            flash_state = 1;
        }
    }
    else if (cmd[0] == MUTE_CMD)
    {
        if (cmd[1])
            mute_amp();
        else
            unmute_amp();
    }
    else if (cmd[0] == STORE_SOUND_CMD)
    {
        if (flashPlay)
            flashPlay = 0;
        flash_state = 1; /* Erasing flash flag */
        programmingFlash = 1; /* Set the flag to enter programming sequence */
    }
    else if (cmd[0] == ERASE_FLASH_CMD)
    {
        eraseFlag = 1;
    }
    else if (cmd[0] == CONFIRM_STORAGE_CMD)
    {
        if (cmd[1])
            write_toc = 1;
        else
            write_toc = 2;
    }
    else if (cmd[0] == CONNECT_ID_CMD)
    {
        /* Send it back as an ack */
        /* XXX check if the problem is here */
        uint8_t tmp[4];
        popStatus(tmp);
        queue_rf_cmd(cmd);
    }
    else if (cmd[0] == SLEEP_CMD)
    {
        if (cmd[1] == SLEEPTYPE_QUICK)
        {
            sleep_f = true;
            /* We need to be sure there's enough place for the sleep commands
             * and we don't need the other commands/status anymore. */
            initCommunicationBuffers();
            /* Send ack. */
            cmd[2] = 1;
            queue_rf_cmd(cmd);
            /* Then set the RF in sleep right away. */
            cmd[2] = 0;
            queue_rf_cmd(cmd);
        }
    }
    else if (cmd[0] == AUTHOR_CMD || cmd[0] == REVISION_CMD ||
             cmd[0] == VERSION_CMD || cmd[0] == INFO_FUXRF_CMD)
    {
        /* Loop back. */
        queue_rf_cmd(cmd);
    }
    else
    {
        /*
         * Commands that should be forwarded.
         */

        /* Ping */
        if (cmd[0] == PONG_CMD)
        {
            /* Index of the pong that is supposed to be received from tuxcore */
            static uint8_t pong_received;
            /* Counter of the missed pongs */
            static uint8_t pong_missed;
            if (pong_received-- < cmd[1])       /* new ping, reset */
            {
                pong_received = cmd[1];
                pong_missed = 0;
            }
            else /* pongs */
            {
                pong_missed += pong_received - cmd[1];
                pong_received = cmd[1]; /* resync */
            }
            cmd[2] = pong_missed;
        }
        return false;
    }
    return true;
}
