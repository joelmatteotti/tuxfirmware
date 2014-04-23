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

/* $Id: config.c 676 2007-11-09 15:39:21Z jaguarondi $ */

#include <avr/eeprom.h>
#include "common/config.h"

/* Configuration registers */
tuxaudio_config_t tuxaudio_config;

/* Default configurations */
tuxaudio_config_t tuxaudio_config_default EEMEM = TUXAUDIO_CONFIG;

/* Initialize the configuration registers */
void config_init(void)
{
    eeprom_read_block((void *)&tuxaudio_config,
                      (const void *)&tuxaudio_config_default,
                      sizeof(tuxaudio_config_t));
}

/* Save the configuration registers to the eeprom */
void config_write(void)
{
    eeprom_write_block((const void *)&tuxaudio_config,
                       (void *)&tuxaudio_config_default,
                       sizeof(tuxaudio_config_t));
}
