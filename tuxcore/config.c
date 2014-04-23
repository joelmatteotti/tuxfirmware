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

/* $Id: config.c 2238 2008-10-07 09:06:20Z jaguarondi $ */

#include <avr/eeprom.h>
#include "common/config.h"

/* Startup event */
uint8_t startup_e[LONG_EVENT] EEMEM = STARTUP_E_SEQ;

/* Head button event */
uint8_t head_e[SHORT_EVENT] EEMEM = HEAD_E_SEQ;

/* Left flipper button event */
uint8_t left_flip_e[SHORT_EVENT] EEMEM = LEFT_FLIP_E_SEQ;

/* Right flipper event */
uint8_t right_flip_e[SHORT_EVENT] EEMEM = RIGHT_FLIP_E_SEQ;

/* Start charging event */
uint8_t charger_start_e[SHORT_EVENT] EEMEM = CHARGER_START_E_SEQ;

/* Unplug event */
uint8_t unplug_e[SHORT_EVENT] EEMEM = UNPLUG_E_SEQ;

/* RF connection event */
uint8_t rf_conn_e[SHORT_EVENT] EEMEM = RF_CONN_E_SEQ;

/* RF disconnection event */
uint8_t rf_disconn_e[SHORT_EVENT] EEMEM = RF_DISCONN_E_SEQ;

/* Configuration registers */
tuxcore_config_t tux_config;

/* Default configurations */
tuxcore_config_t tux_config_default EEMEM = TUXCORE_CONFIG;

/* Hardware revision */
uint8_t hwrev;

/* Initialize the configuration registers */
void config_init(void)
{
    eeprom_read_block((void *)&tux_config, (const void *)&tux_config_default,
                      sizeof(tuxcore_config_t));

    /* Hardware revision */
    hwrev = eeprom_read_byte((uint8_t *)0x8101FF);
}

/* Save the configuration registers to the eeprom */
void config_write(void)
{
    eeprom_write_block((const void *)&tux_config, (void *)&tux_config_default,
                       sizeof(tuxcore_config_t));
}
