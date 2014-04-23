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

/* $Id: flash.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "varis.h"
#include "communication.h"
#include "spi.h"
#include "i2c.h"
#include "flash.h"
#include "AT26F004.h"
#include "common/commands.h"
#include "common/api.h"
#include "audio_fifo.h"

/* Declarations */
static void init_programming(uint8_t adi0, uint8_t adi1, uint8_t adi2);
static void programming_sound(void);
static void playInit(uint8_t const nsound);
static void playingSound(void);
static void stopPlaying(void);

uint8_t flash_state;
uint8_t soundNum;
static uint16_t index;
static uint8_t sound_stored = 0;
static uint8_t count, ad0, ad1;
static uint8_t first_block;


/**
 * \ingroup flash
   \brief Read the number of sound in the flash memory.

   This function scan the sound's indexes, and return the number of sound stored
   in the flash memory.
   */

uint8_t readFlashNumber(void)
{
    uint8_t i;
    ad0 = 0x01;
    ad1 = 0x00;
    count = 0;
    while(1)
    {
        if (read_data(0x00, ad1, ad0) == 0xFF)
        {
            count --;
            return count;
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                ad0 ++;
                if (ad0 == 0)
                    ad1++;
            }
        }
        count ++;
    }
    return 0;
}

uint8_t readLastBlock(uint8_t num)
{
    index = (numSound * 3) + 1;
    ad[0] = read_data(0x00, (index>>8), (index & 0xFF));
    index ++;
    ad[1] = read_data(0x00, (index>>8), (index & 0xFF));
    index ++;
    ad[2] = read_data(0x00, (index>>8), (index & 0xFF));
    return (ad[0] << 4) + (ad[1] >> 4);
}
/**
 * \ingroup flash
   \brief Store a sound in the memory.

   This function contain 5 states :

   DETECT_INDEXES : Detect if the memory is empty or not, and determine where the sound must be stored.

   PROG_INIT : Start the programmation cycle with the indexes determinated in the first state.

   PROGRAMMING : Program the memory with the sound byte received by the RF.

   PROG_TOC : If the programming sequence is finished, the indexes are written.

   PROG_END : Restore the context.

   \note Each sound starts at the first byte of a 4kB block. To do this, the
   lower address byte is reset, and the medium address byte is incremented.

*/

void programming(void)
{
    uint8_t static programming_state = DETECT_INDEXES;
    if (programming_state == DETECT_INDEXES)
    {
        /* Disable audio PWM interrupt */
        TIMSK0 = 0x00;

        if (numSound == 0)
        {
            /* The flash memory is empty. The first address is 0x000400 */
            ad[0] = 0x00;
            ad[1] = 0x04;
            ad[2] = 0x00;
            first_block = 0;
        }
        else
        {
            /* One or more sounds are programmed in the flash memory.
             * The next sound must be stored after the others */
            index = (numSound * 3) + 1;
            ad[0] = read_data(0x00, (index>>8), (index & 0xFF));
            index ++;
            ad[1] = read_data(0x00, (index>>8), (index & 0xFF));
            index ++;
            ad[2] = read_data(0x00, (index>>8), (index & 0xFF));

            // Goto the next 4kB block.
            ad[1] += 0x10;
            ad[1] &= ~0x0F;
            if (ad[1] == 0)
                ad[0] ++;
            ad[2] = 0;
            first_block = (ad[0] << 4) + (ad[1] >> 4);
        }
        AudioFifoClear();

        if (ad[0] > 0x07)
        {
            programming_state = PROG_END;
            queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, FLASH_FULL, 0, 0);
        }
        else
            programming_state ++;

        frame_without_sound = 5000;
        queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, IN_PROGRESS, ad[0], ad[1]);
    }

    else if (programming_state == PROG_INIT)
    {
        init_programming(ad[0], ad[1], ad[2]);
        programming_state ++;
        flash_state = 1;
        sound_stored = 0;
    }
    else if (programming_state == PROGRAMMING)
    {
        if (flash_state)
            programming_sound();

        else
        {
            write_disable();
            if (sound_stored)
            {
                last_block = (ad[0] << 4) + (ad[1] >> 4);
                queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, WAITING_FOR_CONFIRMATION, last_block - first_block, 0);
                programming_state ++;
            }
            else
            {
                queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, NO_SOUND, 0, 0);
                programming_state = PROG_END;
            }
        }
    }
    else if (programming_state == WAITING_STATE)
    {
        /*if (write_toc == 1)*/
        /*{*/
            programming_state = PROG_TOC;
        /*}*/
        /*else if (write_toc == 2)*/
        /*{*/
            /*queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, ERASING_LAST_SOUND, 0, 0);*/
            /*if (first_block == 0)*/
            /*{*/
                /*eraseFlag = 1;*/
                /*programming_state = 0;*/
                /*programmingFlash = 0;*/
            /*}*/
            /*else*/
            /*{*/
                /*last_block = (ad[0] << 4) + (ad[1] >> 4);*/
                /*blockErase(first_block, last_block);*/
                /*programming_state = PROG_END;*/
            /*}*/
        /*}*/
        /*write_toc = 0;*/
    }
    else if (programming_state == PROG_TOC)
    {
        queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, WRITE_TOC, 0, 0);
        numSound ++;
        index = (numSound * 3) + 1;
        program_flash(0x00, (index>>8), (index & 0xFF), ad[0]);
        index ++;
        program_flash(0x00, (index>>8), (index & 0xFF), ad[1]);
        index ++;
        program_flash(0x00, (index>>8), (index & 0xFF), ad[2]);
        last_block = (ad[0] << 4) + (ad[1] >> 4);
        programming_state ++;
    }
    else if (programming_state == PROG_END)
    {
        numSound = readFlashNumber();
        programming_state = 0;
        programmingFlash = 0;
        TIMSK0 = 0x01;
        queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, STANDBY, 0, 0);
        queue_rf_cmd_p(SOUND_VAR_CMD, numSound, last_block, 0);
    }
}

/**
 * \ingroup flash
 * \brief Active the deep power-down mode
 *
 * This mode active the deep-power mode. The consumption will pass to 8mA from
 * 20uA.
 */
void enter_deep_sleep(void)
{
    flash_enable();
    flash_select();
    spiSend(DEEP_POWER_MODE);
    flash_unselect();
}

/**
 * \ingroup flash
 * \brief Leave the deep power_down mode
 *
 * This function will resume the flash memory from the deep power-down mode.
 */
void leave_deep_sleep(void)
{
    flash_enable();
    flash_select();
    spiSend(RESUME_DEEP_MODE);
    flash_unselect();
}

/**
 * \ingroup flash
   \brief Erase the flash memory.

    The first step is to send the command to erase the flash.
    After, the status is polled while the erase process isn't finished.
    When the BUSY flag is null, the erase sequence is stopped.
*/
void erase(void)
{
    uint8_t static enter = 1;
    if (enter)
    {
        erase_flash();
        enter = 0;
    }
    else if (!(read_status() & BUSY))
    {
        //queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, STANDBY, 0, 0);
        enter = 1;
        eraseFlag = 0;

        /* Wite the first index. */
        program_flash(0x00, 0x00, 0x00, 0xFE);
        program_flash(0x00, 0x00, 0x01, 0x00);
        program_flash(0x00, 0x00, 0x02, 0x04);
        program_flash(0x00, 0x00, 0x03, 0x00);
        numSound = 0;
        last_block = 0;

        queue_rf_cmd_p(STATUS_FLASH_PROG_CMD, STANDBY, 0, 0);
        queue_rf_cmd_p(SOUND_VAR_CMD, numSound, last_block, 0);
        /* Re-enable audio PWM interrupt */
        TIMSK0 = 0x01;
    }
}

/**
 * \ingroup flash
   \brief This function is used to play a sound from the flash memory.

   The first step (playInit) is to initialize the flash memory with the selected sound to play.
   Many tests are made to ensure that the sound to play exist, the indexes are correct, etc.

   The second step (playingSound) is to fill the fifo with the sound's bytes, and to verify the addresses.
   */


void playSound(void)
{
    if (flash_state)
        playInit(soundToPlay);
    else
        playingSound();
}

/* Static functions */
/**
 * \ingroup flash
   \brief This function is used to init the memory to play a sound.
   \param nsound Track number to be played.

    To prevent bugs, some verifications are made :
    - check if the sound to play exist
    - check if the sound to play is not null
    - check if the indexes are correct. (the address exist, and the start and end indexes are not the same.

    If these conditions are respected, the memory is initialised with the first sound's byte address.
    The next index is stored to identify the end of the sound track.
*/

static void playInit(uint8_t const nsound)
{
    uint8_t i;

    if (numSound == 0x00)  /* if unprogrammed we have 0xFF stored in flash */
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }
    if (!nsound || (nsound > numSound))    /* check the limits */
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }

    count = 1;
    ad1 = 0x00;
    ad0 = 0x01;
    while (count != nsound)     // Compute address
    {
        for (i = 0; i < 3; i++)
        {
            ad0++;
            if (ad0 == 0)
                ad1++;
        }
        count++;
    }
    flash_select();             // Chip Select
    spiSend(READ_ARRAY_LOW_F);  // Send Read Page Command
    spiSend(0x00);              // Send Address
    spiSend(ad1);
    spiSend(ad0);

    for (i = 0; i < 6; i++)
    {
        ad[i] = spiSend(NOP);  // Read start and stop sound address
    }
    if (nsound > 1)
    {
        ad[1] += 0x10;
        ad[1] &= ~0x0F;
        if (ad[1] == 0)
            ad[0] ++;
        ad[2] = 0;
    }

    flash_unselect();              // Chip Deselect

    /* Check addresses */
    if (ad[0] > TOP_A2)
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }                  /* don't read outside the flash */
    if (ad[3] > TOP_A2)
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }                  /* don't read outside the flash */
    if ((ad[0] == 0) && (ad[1] < 0x04))
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }                  /* minimum index not respected */
    if ((ad[4] == 0) && (ad[5] < 0x04))
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }    /* minimum index not respected */
    if (ad[3] < ad[0])
    {
        flashPlay = 0;
        soundToPlay = 0;
        return;
    }    /* check that the stop index is greater than the start index */
    else if (ad[3] == ad[0])
    {
        if (ad[4] < ad[1])
        {
            flashPlay = 0;
            soundToPlay = 0;
            return;
        }
        else if (ad[4] == ad[1])
        {
            if (ad[5] <= ad[2])
            {
                flashPlay = 0;
                soundToPlay = 0;
                return;
            }
        }
    }
    AudioFifoClear();
    flash_select();             // Chip Select

    spiSend(0x03);              // Send Read Page Command
    spiSend(ad[0]);             // Send Address
    spiSend(ad[1]);
    spiSend(ad[2]);

    OCR0A = 250;                // Normal operation for PWM if fifo adaptative is on
    flash_state = 0;
    queue_rf_cmd_p(STATUS_AUDIO_CMD, numSound, 0, 0);
}

/* Static functions */
/**
 * \ingroup flash
   \brief Read the flash memory and fill the fifo.

    This function reads bytes into the flash memory and fill the fifo.
    When the last byte is read, the sound play's sequence is stopped.
 */

static void playingSound(void)
{
    uint8_t sound;
    while (!rf_txe && AudioFifoLength() < (AudioFifoSize-1))
    {
        sound = spiSend(0x00);  // Wait response
        sound = sound >> audioLevel;
        /* Save it twice to have 16kHz */
        AudioFifoPut_inl(sound);
        AudioFifoPut_inl(sound);

        ad[2]++;        // Increment address for next play
        if (ad[2] == 0)
        {
            ad[1]++;
            if (ad[1] == 0)
            {
                ad[0]++;
                if (ad[0] == 0x08)      // Address overflow
                {
                    stopPlaying();
                    break;
                }
            }
        }
        if (ad[0] == ad[3])     // Test end of sound
            if (ad[1] == ad[4])
                if (ad[2] == ad[5])
                {
                    stopPlaying();
                    break;
                }
    }
}

/* Static functions */
/**
 * \ingroup flash
   \brief Stop the play sequence.

    */
static void stopPlaying(void)
{
    soundToPlay = 0;
    flashPlay = 0;
    queue_rf_cmd_p(STATUS_AUDIO_CMD, 0, 0, 0);
    PORTB |= 0x01;              // Set the HOLD signal
    PORTB |= 0x02;              // Chip Deselect
}

/**
 * \ingroup flash
   \brief Init the programming sequence

   To perform a sequential programming, the first step is to send the correct OP
   code, and the three address bytes where the first data byte must be stored.
   The second step is to send the OP code and a data to write.

   This function perform the first step.
    */
static void init_programming(uint8_t adi0, uint8_t adi1, uint8_t adi2)
{
    uint8_t data;
    write_enable();
    flash_select();
    spiSend(SEQU_PROGRAM);
    spiSend(adi0);
    spiSend(adi1);
    spiSend(adi2);
    if (AudioFifoGet(&data) == A_FIFO_OK)
        spiSend(data);
    else
        spiSend(0x80);

    flash_unselect();              // Chip Deselect
}


/**
 * \ingroup flash
   \brief Program the sound's data into the flash memory

    This function is executed while the SPI start command is not present.  Each
    cycle, one byte is popped from the PWM fifo and stored in the sound flash.

    The frame_without_sound variable is decremented each time the RF receive a
    frame without sound.  When this variable is null, the sound process is
    stopped.

    If a programming sequence starts, but no sound is received, the cycle is
    stopped.

    If the address is equal to 0x07FFFF (the last memory address), the programming
    cycle is stopped.

    The sound_stored flag is set when at least one byte is stored in the memory.
    Else, this variable is null.
 */

static void programming_sound(void)
{
    while (!rf_txe)
    {
        if (AudioFifoLength())
        {
            uint8_t data;
            sound_stored = 1;
            frame_without_sound = STOP_FRAME_NUMBER;

            flash_select();
            spiSend(SEQU_PROGRAM);
            AudioFifoGet_inl(&data);
            spiSend(data);
            /* Drop one byte out of 2 to store in 8kHz */
            AudioFifoGet_inl(&data);
            flash_unselect();

            ad[2] ++;
            if (ad[2] == 0x00)
            {
                ad[1]++;
                if (ad[1] == 0x00)
                    ad[0]++;
            }
            if (ad[0] == 0x07 && ad[1] == 0xFF && ad[2] == 0xFF)
            {
                flash_state = 0;
                break;
            }
            while (read_status() & BUSY) ;
        }
        else
        {
            break;
        }

    }
    /* Check for the last sound byte */
    if (!(frame_without_sound))
    {
        flash_state = 0;
    }
}

