/*
 * TUXDEFS - Common defines used by the firmware and daemon of tuxdroid
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

/* $Id: config.h 2238 2008-10-07 09:06:20Z jaguarondi $*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <avr/eeprom.h>

#include "commands.h"
#include "api.h"
#include "defines.h"

/*
 * Standalone events
 */

#define TINY_EVENT 6
#define SHORT_EVENT 16
#define LONG_EVENT 31
#define END_OF_ACTIONS  0xFF    /* action time that indicates the end of commands */

/* Startup event */
#define STARTUP_E_SEQ {\
    24, RESET_WINGS_CMD, OPEN_EYES_CMD, LED_ON_CMD, 0, /* set wings low, open the eyes if closed and light them */\
    20, MOVE_MOUTH_CMD, 2, 0, 0, /* move the mouth */\
    18, PLAY_SOUND_CMD, 4, 0, 0, /* play the 'hello' sound */\
    10, LED_OFF_CMD, CLOSE_MOUTH_CMD, 0, 0,\
    0,  COND_RESET_CMD, INFO_TUXCORE_CMD, 0, 0, /* reset the conditional flags */\
    END_OF_ACTIONS\
}

/* Head button event */
#define HEAD_E_SEQ {\
    0, LED_TOGGLE_CMD, 2, 30, 0, /* Pulse LEDs once */\
    END_OF_ACTIONS\
}

/* Left flipper button event */
#define LEFT_FLIP_E_SEQ {\
    0, LED_TOGGLE_CMD, 2, 30, 0, /* Pulse LEDs once */\
    END_OF_ACTIONS\
}

/* Right flipper event */
#define RIGHT_FLIP_E_SEQ {\
    0, LED_TOGGLE_CMD, 2, 30, 0, /* Pulse LEDs once */\
    END_OF_ACTIONS\
}

/* Start charging event */
#define CHARGER_START_E_SEQ {\
    0, MOVE_MOUTH_CMD, 2, 0, 0, /* move the mouth */\
    0, LED_TOGGLE_CMD, 4, 50, 0, /* Pulse LEDs twice */\
    0, PLAY_SOUND_CMD, 3, 0, 0, /* play a sound */\
    END_OF_ACTIONS\
}

/* Unplug event */
#define UNPLUG_E_SEQ {\
    3, PLAY_SOUND_CMD, 2, 0, 0, /* play a sound */\
    0, MOVE_MOUTH_CMD, 2, 0, 0, /* move the mouth */\
    END_OF_ACTIONS\
}

/* RF connection event */
#define RF_CONN_E_SEQ {\
    0, LED_ON_CMD, 0, 0, 0, /* turn on the leds */\
    END_OF_ACTIONS\
}

/* RF disconnection event */
#define RF_DISCONN_E_SEQ {\
    0, LED_OFF_CMD, 0, 0, 0, /* turn off the leds */\
    END_OF_ACTIONS\
}

/* Tux greeting event */
#define TUX_GR_E_SEQ {\
    5, PLAY_SOUND_CMD, 6, 0, 0, /* play a sound */\
    5, MOVE_MOUTH_CMD, 2, 0, 0,\
    END_OF_ACTIONS\
}

/* Tux greeting reply event */
#define TUX_GR_REPL_E_SEQ {\
    10, PLAY_SOUND_CMD, 7, 0, 0, /* play a sound */\
    10, MOVE_MOUTH_CMD, 2, 0, 0,\
    10, WAVE_WINGS_CMD, 2, 5, 0,\
    END_OF_ACTIONS\
}

/* Tux greeting second reply event */
#define TUX_GR_REPL2_E_SEQ {\
    10, PLAY_SOUND_CMD, 8, 0, 0, /* play a sound */\
    10, MOVE_MOUTH_CMD, 2, 0, 0,\
    0, WAVE_WINGS_CMD, 2, 5, 0,\
    END_OF_ACTIONS\
}

/*
 * Configuration settings
 */
typedef struct
{
    uint8_t ir_feedback;        /* flashes the leds when an ir code is received */
    uint8_t led_off_when_closed_eyes;   /* turns off the leds when the eyes are closed */
    uint8_t tux_greeting;       /* greeting when another tux is seen */
}
tuxcore_config_t;

typedef struct
{
    uint8_t automute;           /* mutes the speaker when no sounds are played and unmute it when sound arrives, you can't use the line in in this mode as the insertion of a plug is not detected so the speaker won't be unmuted */
}
tuxaudio_config_t;

/* Default configurations */
#define TUXCORE_CONFIG  { \
    .ir_feedback = 1, \
    .led_off_when_closed_eyes = 1, \
    .tux_greeting = 0}

#define TUXAUDIO_CONFIG { \
    .automute = 0}

#endif /* _CONFIG_H_ */
