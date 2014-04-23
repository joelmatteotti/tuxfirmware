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

/* $Id: bootloader.c 1127 2008-05-07 10:44:53Z jaguarondi $ */

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <avr/eeprom.h>
#include "hardware.h"
#include "common/defines.h"

typedef union
{
    uint16_t w;
    uint8_t b[2];
} union16_t;

void (*jump_to_application) (void) = (void *)0x0000;
void i2cWaitForComplete(void);

/** \ingroup minimal_bootloader
 *
 * Minimal I2C bootloader
 */
int main(void)
{
    union16_t pageAddress, data;
    uint16_t address;
    uint8_t i, twStatus, eeprom_flg = 0;
    volatile uint16_t _count = 0;

    /* Wait for the head button to stabilize. */
    for (_count = 0; _count < 0xFFFF; _count++) ;
    if (SW_HD_PIN & SW_HD_MK) /* if head is not pushed at startup */
        jump_to_application();

    LED_DDR |= LED_L_MK; /* light on left led */
    LED_PT |= LED_L_MK;

    TWBR = (F_CPU / 100000UL - 16) / 2; /* twi intialisation */
    TWAR = (TUXCORE_BL_ADDR << 1);

    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
    for (;;)
    {
        i2cWaitForComplete(); /* wait the I2C address */
        if ((twStatus = TW_STATUS) != TW_SR_SLA_ACK)
            while (1) ;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        i2cWaitForComplete(); /* wait the high byte of the page address */
        if ((twStatus = TW_STATUS) != TW_SR_DATA_ACK)
            while (1) ;
        pageAddress.b[1] = TWDR;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        i2cWaitForComplete(); /* wait the low byte of the page address */
        if ((twStatus = TW_STATUS) != TW_SR_DATA_ACK)
            while (1) ;
        pageAddress.b[0] = TWDR;
        if (pageAddress.w & 0x003F)
            while (1) ; /* incorrect page address */
        if (pageAddress.w >= 0x8000) /* eeprom programming */
        {
            eeprom_flg = 1; /* don't overwrite the bootloader */
            pageAddress.b[1] &= 0x7F; /* remove the eeprom indication bit */
        }
        else if (pageAddress.w >= 0x1E00)
            while (1) ; /* don't overwrite the bootloader */
        else
            eeprom_flg = 0; /* flash programming */
        address = pageAddress.w;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        /* Fill the complete page with the next data. */
        for (i = 0; i < SPM_PAGESIZE; i++)
        {
            i2cWaitForComplete(); /* wait data */
            if ((twStatus = TW_STATUS) == TW_SR_DATA_ACK)
            {
                if ((i & 0x01) == 0)
                {
                    data.b[0] = TWDR;
                }
                else
                {
                    data.b[1] = TWDR;
                    if (eeprom_flg)
                    {
                        eeprom_write_word((uint16_t *) address, data.w);
                        eeprom_busy_wait();
                    }
                    else
                        boot_page_fill(address, data.w);
                    address += 2;
                }
                TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
            }
            else
                while (1) ; /* error in communication */
        }
        i2cWaitForComplete();
        if ((twStatus = TW_STATUS) != TW_SR_STOP)
            /* If no stop at this exact position, there's something wrong with
             * the number of bytes sent, we cancel. */
            while (1) ;
        /* Clear the interrupt immediately so not to miss the next frame. */
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        if (!eeprom_flg)
        {
            boot_page_erase(pageAddress.w);
            boot_spm_busy_wait();
            boot_page_write(pageAddress.w); /* Store buffer in flash page */
            boot_spm_busy_wait();
        }
    }
}

void i2cWaitForComplete(void)
{
    /* Wait for i2c interface to complete operation. */
    while (!(TWCR & _BV(TWINT))) ;
}
