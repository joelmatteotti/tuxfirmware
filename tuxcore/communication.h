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

/* $Id: communication.h 1133 2008-05-08 09:46:35Z jaguarondi $ */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdbool.h>
#include "common/commands.h"
#include "fifo.h"


void communication_init(void);

int8_t get_cmd(uint8_t *cmd);
int8_t queue_cmd_p(uint8_t command, uint8_t param1, uint8_t param2, \
                     uint8_t param3);
int8_t queue_cmd(uint8_t *status);
bool cmds_sent(void);
bool cmds_empty(void);

#endif /* COMMUNICATION_H */
