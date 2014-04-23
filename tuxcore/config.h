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

/* $Id: config.h 2019 2008-09-25 11:47:56Z jaguarondi $ */

#ifndef CONFIG_H
#define CONFIG_H

#include "common/config.h"

extern tuxcore_config_t tux_config;

/* Startup event */
extern uint8_t startup_e[];

/* Head button event */
extern uint8_t head_e[];

/* Left flipper button event */
extern uint8_t left_flip_e[];

/* Right flipper event */
extern uint8_t right_flip_e[];

/* Start charging event */
extern uint8_t charger_start_e[];

/* Unplug event */
extern uint8_t unplug_e[];

/* Entering sleep event */
extern uint8_t sleep_enter_e[];
/* Exiting sleep event */
extern uint8_t sleep_exit_e[];

/* RF connection event */
extern uint8_t rf_conn_e[];

/* RF disconnection event */
extern uint8_t rf_disconn_e[];

/* Tux greeting event */
extern uint8_t tux_gr_e[];

/* Tux greeting reply event */
extern uint8_t tux_gr_repl_e[];

/* Tux greeting second reply event */
extern uint8_t tux_gr_repl2_e[];

/* Hardware revision */
extern uint8_t hwrev;

void config_init(void);

#endif /* CONFIG_H */
