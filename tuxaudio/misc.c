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

/* $Id: misc.c 1130 2008-05-07 12:42:28Z jaguarondi $ */

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "misc.h"
#include "communication.h"
#include "version.h"
#include "varis.h"

/*
 * Version number
 */
#define CPU_NUMBER      TUXAUDIO_CPU_NUM /* audio CPU */
const author_t author __attribute__ ((section("version.3"))) =
{AUTHOR_CMD, AUTHOR_ID, VARIATION};
const revision_t svn_revision __attribute__ ((section("version.2"))) =
{REVISION_CMD, SVN_REV, RELEASE_TYPE};
const version_t tag_version __attribute__ ((section("version.1"))) =
{VERSION_CMD, CPU_VER_JOIN(CPU_NUMBER, VER_MAJOR), VER_MINOR, VER_UPDATE};

/**
 * Send version information.
 */
void send_info(void)
{
    uint8_t i, buf[12];
    uint8_t *data = ((uint8_t *) & tag_version);

    /* Retrieve version information */
    for (i = 0; i < 12; i++)
        buf[i] = pgm_read_byte(data++);
    queue_rf_cmd((uint8_t const *) buf);
    queue_rf_cmd((uint8_t const *) buf+4);
    queue_rf_cmd((uint8_t const *) buf+8);
    /* Send extra information */
    queue_rf_cmd_p(SOUND_VAR_CMD, numSound, last_block, 0);
}
