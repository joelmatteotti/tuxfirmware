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

/* $Id: global.c 1110 2008-05-06 09:38:42Z jaguarondi $ */

#include <inttypes.h>
#include <avr/io.h>

#include "global.h"
#include "version.h"

/*
 * Standalone behavior
 */
/* remote mode to control tux from the remote control, XXX to be replaced with
 * a better approach from the standalone behavior */
uint8_t remote_mode = 0;

/*
 * General global registers
 */
uint8_t updateStatusFlag, commandProcessFlag, pingCnt;
uint8_t ir_delay, ir_flg, ir_oldvalue, alt_mode, ir_send_flg, tux_ir_id,
    last_tux_seen = 0xFF;

/** Global error variable used for debugging purposes. */
enum gerrors gerror;

/*
 * Condition flags
 *
 * Initialization
 */

struct condition_table cond_flags = { .startup=1 };      /* Set startup flag */

/*
 * Version number
 */

#define CPU_NUMBER      TUXCORE_CPU_NUM /* tuxcore CPU */
const author_t author __attribute__ ((section("version.3"))) =
{AUTHOR_CMD, AUTHOR_ID, VARIATION};
const revision_t svn_revision __attribute__ ((section("version.2"))) =
{REVISION_CMD, SVN_REV, RELEASE_TYPE};
const version_t tag_version __attribute__ ((section("version.1"))) =
{VERSION_CMD, CPU_VER_JOIN(CPU_NUMBER, VER_MAJOR), VER_MINOR, VER_UPDATE};
