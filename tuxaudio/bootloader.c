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

/* $Id: bootloader.c 1118 2008-05-06 15:00:57Z jaguarondi $ */

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <avr/eeprom.h>
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
    volatile uint8_t _count = 0;

    PORTC = 0x08;               /* head button on PC3 */
    DDRC = 0x08;                /* force the head button up for a short period to quickly charge the capacitance */
    for (_count = 0; _count < 50; _count++) ;
    DDRC = 0x00;                /* then set as pull-up */
    if ((PINC & 0x08))          /* and check it */
        jump_to_application();

    TWBR = (F_CPU / 100000UL - 16) / 2; /* twi intialisation */
    TWAR = (TUXAUDIO_BL_ADDR << 1);

    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
    for (;;)
    {
        i2cWaitForComplete();   /* wait the I2C address */
        if ((twStatus = TW_STATUS) != TW_SR_SLA_ACK)
            while (1) ;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        i2cWaitForComplete();   /* wait the high byte of the page address */
        if ((twStatus = TW_STATUS) != TW_SR_DATA_ACK)
            while (1) ;
        pageAddress.b[1] = TWDR;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        i2cWaitForComplete();   /* wait the low byte of the page address */
        if ((twStatus = TW_STATUS) != TW_SR_DATA_ACK)
            while (1) ;
        pageAddress.b[0] = TWDR;
        if (pageAddress.w & 0x003F)
            while (1) ;         /* incorrect page address */
        if (pageAddress.w >= 0x8000)    /* eeprom programming */
        {
            eeprom_flg = 1;     /* don't overwrite the bootloader */
            pageAddress.b[1] &= 0x7F;   /* remove the eeprom indication bit */
        }
        else if (pageAddress.w >= 0x1E00)
            while (1) ;         /* don't overwrite the bootloader */
        else
            eeprom_flg = 0;     /* flash programming */
        address = pageAddress.w;
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
        for (i = 0; i < SPM_PAGESIZE; i++)      /* fill the complete page with the next data */
        {
            i2cWaitForComplete();       /* wait data */
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
                while (1) ;     /* error in communication */
        }
        i2cWaitForComplete();
        if ((twStatus = TW_STATUS) != TW_SR_STOP)
            while (1) ;         /* if no stop at this exact position, there's something wrong with the number of bytes sent, we cancel */
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);      /* clear the interrupt immediately so not to miss the next frame */
        if (!eeprom_flg)
        {
            boot_page_erase(pageAddress.w);
            boot_spm_busy_wait();
            boot_page_write(pageAddress.w);     /* Store buffer in flash page */
            boot_spm_busy_wait();
        }
    }
}

void i2cWaitForComplete(void)
{
    while (!(TWCR & _BV(TWINT))) ;      /* wait for i2c interface to complete operation */
}
