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

/* $Id: sensors.h 585 2007-10-08 13:47:19Z Paul_R $ */

/** \file sensors.h
    \brief Sensors module interface
    \ingroup sensors
*/

/** \defgroup sensors Sensors

    The sensor module handles light and battery level measurement. As both
    measurements are done with the same ADC (and same interrupt), it was
    difficult to separate them as different modules.
*/

#ifndef _SENSORS_H_
#define _SENSORS_H_

#define LIGHT_FLAG 1
#define BATTERY_FLAG 2
#define STATUS_SENT 1
#define ADC_READY 2

uint8_t extern sensorsStatus;
uint8_t extern sensorsUpdate;

void sensors_init(void);
void sensors_disable(void);
void sensors_control(void);

#endif /* _SENSORS_H_ */
