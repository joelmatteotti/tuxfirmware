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

/* $Id: sensors.c 1112 2008-05-06 09:54:21Z jaguarondi $ */

/** \file sensors.c
    \brief Sensors module
    \ingroup sensors
*/

#include <avr/interrupt.h>

#include "sensors.h"
#include "adc.h"
#include "global.h"
#include "sensors.h"
#include "hardware.h"
#include "adc.h"

void static light_control(uint16_t light_val);
void static battery_control(uint16_t battery_val);
void static read_flag(void);

uint8_t static sensorsControlState;
uint8_t static motorsStatus;
uint8_t sensorsStatus;
uint8_t sensorsUpdate;

/**
   \brief This function manage the acquisition of all sensors.

   Two sensors are treaties for now : A light sensor and the battery.
   This function is based on a state machine.

   To execute this function, the STATUS_SENT flag OR the ADC_READY flag must be set. The STATUS_SEND flag is set when the status are send to the PC. The ADC_READY flag is set when an interrupt on the ADC module occurs.

   When the state machine is in the last state, it will be resetted and wait while the status are sent. If status are sent before the last state, the cycle continue.
 */
void sensors_control(void)
{
    if (sensorsControlState == 0)
    {
        // New cycle started - clear the status_sent flag
        sensorsUpdate &= ~STATUS_SENT;

        ADC_start(LIGHT_ADMUX); // XXX Arg : ADMUX
        sensorsControlState ++;
    }
    else if (sensorsControlState == 1)
    {
        light_control(ADC_read());
        if (gStatus.mot)
            motorsStatus = 1;

        ADC_start(BATTERY_ADMUX); // XXX arg : ADMUX
        sensorsControlState ++;
    }
    else if (sensorsControlState == 2)
    {
        if (gStatus.mot)
            motorsStatus = 1;
        battery_control(ADC_read());
        sensorsControlState = 0;
    }
}

/**
   \brief Initialize the sensors for normal operation.
 */
void sensors_init(void)
{
    ADC_set_ISR_callback(read_flag);
    ADC_init();
}

/**
   \brief Disable all sensors.
   This function should minimize power consumption.
  */
void sensors_disable(void)
{
    ADC_disable();
}

/**
   \brief Callback function of the ADC ISR.

   When an ADC conversion cycle is finished, the ADC_READY flag is set.
   */

void static read_flag(void)
{
    sensorsUpdate |= ADC_READY;
}

/**
   \brief Light control function.
   \param light_val The light level

   This function control the light level.
   The light sensor has 2 different values of pull-up resistors (1M and 10k) to
   increase the range. It's necessary to switch from one to the other resistor
   depending on the light level. This is done here.
  */

void static light_control(uint16_t light_val)
{
    union16_t light_value;
    light_value.w = light_val;

    /* Save light mode before changing it below. */
    gStatus.lightM = LIGHT_PU_PORT & LIGHT_PU_MK;

    /* Check if it's necessary to change the light mode. */
    if ((light_value.w <= 0x0028) && !(LIGHT_PU_PORT & LIGHT_PU_MK))
    {
        LIGHT_PU_PORT |= LIGHT_PU_MK;
        LIGHT_PU_DDR |= LIGHT_PU_MK;
    }
    else if ((light_value.w >= 0x03E8) && (LIGHT_PU_PORT & LIGHT_PU_MK))
    {
        LIGHT_PU_PORT &= ~LIGHT_PU_MK;
        LIGHT_PU_DDR &= ~LIGHT_PU_MK;
    }

    gStatus.lightL = light_value.b[0];
    gStatus.lightH = light_value.b[1];
    /* There's a new light value. */
    sensorsStatus |= LIGHT_FLAG;
    sensorsUpdate &= ~ ADC_READY;
}

/**
   \brief Battery control function.
   \param battery_val Battery level

   The battery level is just stored in gStatus to be sent to the computer.
  */

void static battery_control(uint16_t battery_val)
{
    union16_t battery_value;
    battery_value.w = battery_val;

    gStatus.batteryL = battery_value.b[0];
    gStatus.batteryH = battery_value.b[1];
    if (motorsStatus)
        gStatus.batteryS = 1;
    else
        gStatus.batteryS = 0;
    motorsStatus = 0;
    sensorsStatus |= BATTERY_FLAG;
    sensorsUpdate &= ~ ADC_READY;
}
/*! @} */
