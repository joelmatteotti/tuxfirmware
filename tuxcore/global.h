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

/* $Id: global.h 2238 2008-10-07 09:06:20Z jaguarondi $ */

/** \file global.h
    \brief Global ressources
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <avr/io.h>
#include "common/commands.h"
#include "common/defines.h"
#include "common/api.h"

/*
 * Custom types
 */
/** Union between 16 bits variables and the two 8 bits registers they're made
 * of. */
typedef union
{
    uint16_t w;
    uint8_t b[2];
} union16_t;

extern enum gerrors gerror;

/*
 * --------------------------------------------------------
 * STATUS
 * --------------------------------------------------------
 */

/*
 * gStatus.sw: Switch Status
 */

#define GSTATUS_LEFTWINGBTN_MK      STATUS_LEFTWINGBTN_MK
#define GSTATUS_RIGHTWINGBTN_MK     STATUS_RIGHTWINGBTN_MK
#define GSTATUS_POWERPLUGSW_MK      STATUS_POWERPLUGSW_MK
#define GSTATUS_HEADBTN_MK          STATUS_HEADBTN_MK
#define GSTATUS_CHARGER_MK          STATUS_CHARGER_MK
#define GSTATUS_RF_MK               STATUS_RF_MK
#define GSTATUS_VCC_MK              STATUS_VCC_MK
#define GSTATUS_MUTE_MK             STATUS_MUTE_MK

/*
 * gStatus.pos: Position Status (Eyes - Wings - Mouth)
 */

/* gStatus.pos:[1-0] Wing position
 *  xxxxxxx0 === Wings lowered
 *  xxxxxxx1 === Wings raised
 *  xxxxxx0x === Motors stopped in a known position
 *  xxxxxx1x === Motors running or stopped in an unknown position. In this
 *  case, the previous bit means the last state the wings have passed in. */
#define GSTATUS_POS_W0      0x01
#define GSTATUS_POS_W1      0x02
/* gStatus.mic:[3-2] Eyes position
 *  xxxxxx00 === Eyes closed
 *  xxxxxx01 === Eyes opened
 *  xxxxxx10 === Motors running, unknown position
 *  xxxxxx11 === Motors stopped, unknown position */
#define GSTATUS_POS_E0      0x04
#define GSTATUS_POS_E1      0x08
/* gStatus.mic:[5-4] Mouth position
 *  xxxxxx00 === Mouth closed
 *  xxxxxx01 === Mouth opened
 *  xxxxxx10 === Motors running, unknown position
 *  xxxxxx11 === Motors stopped, unknown position */
#define GSTATUS_POS_M0      0x10
#define GSTATUS_POS_M1      0x20
//#define GSTATUS_SW__6      0x40
//#define GSTATUS_SW__7      0x80

/*
 * gStatus.bat: Orientation Status
 */

#define GSTATUS_ORI_ORI     0x03        /* Orientation mask */
#define GSTATUS_ORI_CNT     0xFC        /* Turn count mask */

/*
 * gStatus.mot: Motors Status
 */

/* gStatus.mot:[0:1] Spin motor
 *  xxxxxx00 === Spin off
 *  xxxxxx01 === Spin right on 
 *  xxxxxx10 === Spin left on
 *  xxxxxx11 === undef / impossible */
#define GSTATUS_MOT_SPINL      0x01
#define GSTATUS_MOT_SPINR      0x02
#define GSTATUS_MOT_SPIN_MK    0x03
/* gStatus.mot:[2] Eyes/Mouth motor
 *  xxxxx0xx === motor off
 *  xxxxx1xx === motor on */
#define GSTATUS_MOT_EYES      0x04
#define GSTATUS_MOT_MOUTH     0x08
/* gStatus.mot:[3] Wings motor
 *  xxxx0xxx === Wings off
 *  xxxx1xxx === Wings on */
#define GSTATUS_MOT_WINGS     0x10

/*
 * gStatus.ir: IR Status
 */

/* bits 5-0:  - VTCCCCCC (C: RC5 6 bits command) */
#define GSTATUS_IR_VALID        _BV(7)  /* V: set when RC5 data has been \
                                           received */
#define GSTATUS_IR_TOGGLE       _BV(6)  /* T: toggle bit of the RC5 code */
#define GSTATUS_IR_COMMAND      0x3F    /* T: toggle bit of the RC5 code */

/*
 * --------------------------------------------------------
 * COMMANDS
 * --------------------------------------------------------
 */

/*
 * gCommands.mot: Motors commands
 * XXX improve display of bits
 */

#define COM_EYES_MK         0x01        /* Eyes motor command mask */
#define COM_MOUTH_MK        0x02        /* Mouth motor command mask */

#define COM_WINGS_MK        0x20        /* Wings motor */

#define COM_SPIN_STP_MK     0x80        /* Spin motor stop at bat position */
//#define COM_SPIN_R_MK       0x40 [> Spin Right <]
//#define COM_SPIN_L_MK       0x80 [> Spin Left <]

/*
 * --------------------------------------------------------
 * MISC
 * --------------------------------------------------------
 */

typedef struct
{
    uint8_t sw;                 /* Switches */
    uint8_t mic;                /* Microphone level */
    uint8_t bat;                /* Battery level */
    uint8_t lightL;             /* Light level low byte */
    uint8_t lightH;             /* Light level high byte */
    uint8_t lightM;             /* Light mode */
    uint8_t batteryL;           /* Battery level low byte */
    uint8_t batteryH;           /* Battery level high byte */
    uint8_t batteryS;           /* Battery level status */
    uint8_t ir;                 /* IR RC5 code received from tux's remote */
    uint8_t pos;                /* Poitionning */
    uint8_t mot;		/* Motors states */
    uint8_t audio_play;
    uint8_t audio_status;
}
GSTATUS;

extern volatile GSTATUS gStatus;

/*
 * intflags
 * Flags that are set inside interrupts, and reset outside when processed
 */

uint8_t light_f;                /* adc interrupt for light measurement */
uint8_t batt_f;                 /* adc interrupt for battery measurement */
uint8_t ir_f;                   /* ir code received */

/*
 * Standalone behavior
 */

extern uint8_t remote_mode; /* normal mode */

/*
 * General global registers
 */
extern uint8_t updateStatusFlag, commandProcessFlag, pingCnt;
extern uint8_t ir_delay, ir_flg, ir_oldvalue, alt_mode, ir_send_flg, tux_ir_id,
    last_tux_seen;

/*
 * Condition flags
 *
 * TODO merge flags by categories in bitfields or bytes and masks
 */

/* Number of flags that should be reset, change this according to the table
 * below.  */
#define COND_RESET_NBR 9
struct condition_table
{
    /* flags reset by the COND_RESET_CMD */
    uint8_t startup;            /* set startup condition at initialization */
    uint8_t head;
    uint8_t left_flip;
    uint8_t right_flip;
    uint8_t charger_start;
    uint8_t unplug;
    uint8_t tux_recog_cnt;
    uint8_t tux_recog;
    uint8_t sleep;
    /* flags not reset by the COND_RESET_CMD */
    uint8_t eyes_closed;
    uint8_t rf_conn;
    uint8_t rf_disconn;
};

extern struct condition_table cond_flags;

/*
 * Version number
 */

extern const author_t author;
extern const revision_t svn_revision;
extern const version_t tag_version;

#endif /* _GLOBAL_H_ */
