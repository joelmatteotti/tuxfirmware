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

/* $Id: communication.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "communication.h"
#include "fifo.h"
#include "i2c.h"
#include "parser.h"
#include "hardware.h"
#include "varis.h"
#include "audio_fifo.h"
#include "micro_fifo.h"

/* I2C write message (out) */
static uint8_t out_buf[CMD_SIZE];
static struct i2c_msg msg_out = {0, 0, out_buf};
/* I2C read message (in) */
static uint8_t in_buf[CMD_SIZE];
static struct i2c_msg msg_in = {0, 0, in_buf};
/* I2C last received command */
static uint8_t *received_cmd = 0;

/** Size of the stack buffer to tuxcore */
#define CORE_OUT_BUF_SIZE    16
/** Size of the stack buffer to tuxrf */
#define RF_BUF_SIZE   32

FIFO_INSTANCE(core_cmdout_buf, CORE_OUT_BUF_SIZE);
/** Stack for commands to be sent to tuxcore */
fifo_t *core_cmdout = FifoPointer(core_cmdout_buf);

FIFO_INSTANCE(rf_cmdout_buf_s, RF_BUF_SIZE);
/** Stack for commands to be sent to rf */
fifo_t *rf_cmdout_buf = FifoPointer(rf_cmdout_buf_s);

FIFO_INSTANCE(rf_cmdin_buf_s, RF_BUF_SIZE);
/** Stack for commands received from the rf */
fifo_t *rf_cmdin_buf = FifoPointer(rf_cmdin_buf_s);


/**
 * \brief Take one command out of the command stack and send it through i2c.
 *
 * \return 0 if nothing has to be sent, -1 if the stack is corrupted, 1 if
 * something has been sent.
 */
static int8_t send_core_cmds(void)
{
    uint8_t i;

    if (FifoLength(core_cmdout))
        /* Send commands received from RF or testers to tuxcore only. */
    {
        for (i = 0; i < CMD_SIZE; i++)
        {
            if (FifoGet(core_cmdout, &out_buf[i]))
                /* Drop the data if the fifo is corrupted (not enough
                 * bytes) XXX add an error feedback on this */
                return -1;
        }
        i2c_send_bytes(&msg_out);
    }
    else
        /* Nothing to do anymore */
        return 0;

    return 1;
}

/**
 * \brief Start an I2C read request to fetch one command from tuxcore.
 * If the rf stack is full, we return immediately.
 */
static void get_core_cmd(void)
{
    /* First check if the stack is not full */
    if (FifoLength(rf_cmdout_buf) > RF_BUF_SIZE - CMD_SIZE)
        return;

    if (i2c_get_status() != I2C_BUSY)
    {
        i2c_read_bytes(&msg_in);
    }
}


/**
 * \brief Add a command on the stack for tuxcore
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_core_cmd(uint8_t *cmd)
{
    uint8_t i;

    if (FifoLength(core_cmdout) > FifoSize(core_cmdout) - CMD_SIZE)
        return 0;

    uint8_t sreg;
    sreg = SREG;
    cli();
    for (i=0; i<CMD_SIZE; i++)
        FifoPut(core_cmdout, cmd[i]);
    SREG = sreg;
    return 1;
}

/**
 * \brief Add a command on the stack for tuxcore
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_core_cmd_p(uint8_t cmd, uint8_t param1, uint8_t param2, \
                     uint8_t param3)
{
    uint8_t c[4] = {cmd , param1, param2, param3};
    return queue_core_cmd(c);
}

/**
 * \brief Add a command on the stack for the rf
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_rf_cmd(uint8_t const *cmd)
{
    uint8_t i;

    /* Drop command if RF is disconnected, except SLEEP_CMD. */
    if (!(RF_ONLINE_PIN & RF_ONLINE_MK) && cmd[0] != SLEEP_CMD)
    {
        return 0;
    }

    if (FifoLength(rf_cmdout_buf) > RF_BUF_SIZE - CMD_SIZE)
        return 0;

    uint8_t sreg;
    sreg = SREG;
    cli();
    for (i = 0; i < CMD_SIZE; i++)
        FifoPut(rf_cmdout_buf, cmd[i]);
    SREG = sreg;
    return 1;
}

/**
 * \brief Add a command on the stack for the rf
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_rf_cmd_p(uint8_t cmd, uint8_t param1, uint8_t param2, \
                     uint8_t param3)
{
    uint8_t c[4] = {cmd , param1, param2, param3};
    return queue_rf_cmd(c);
}

/**
 * \brief Callback function associated with the i2c ISR and which process the
 * received command.
 * \param msg I2C message received
 * \return 0 if a stop should be sent, 1 for a restart.
 */
void i2c_master_receive_service(struct i2c_msg *msg)
{
    if (msg->len != CMD_SIZE)
        /* Error here. */
        return;
    if (*(msg->buf) == 0)
    {
        /* Got nothing so stop reading. */
        return;
    }
    if (msg->addr == TUXCORE_ADDR)
        /* From tuxcore */
    {
        received_cmd = msg->buf;
    }
}

/*
 * Get a set of status commands from the rf_cmdout_buf buffer. The command
 * length will always be CMD_SIZE. Returns '1' if there's nothing to
 * get, '0' otherwise.
 */
uint8_t popStatus(uint8_t *command)
{
    uint8_t i;

    if (!FifoLength(rf_cmdout_buf))
        return 1;               /* nothing to do */

    cli();                      /* XXX try to disable I2C interrupts instead */
    for (i = 0; i < CMD_SIZE; i++)
        if (FifoGet(rf_cmdout_buf, &command[i]))
        {
            sei();
            return 1; /* fifo corrupted so drop data XXX add some debug
                         feedback on this instead of dropping data */
        }
    sei();
    return 0;
}

/**
 * \brief Initialize the I2C communication and the in and out buffers.
 */
void communication_init(void)
{
    i2c_init();
    msg_out.addr = TUXCORE_ADDR;
    msg_out.len = CMD_SIZE;
    msg_in.addr = TUXCORE_ADDR;
    msg_in.len = CMD_SIZE;
    i2c_master_receive_handler(i2c_master_receive_service);
}

/*
 * SPI communication
 */

static uint8_t frame_in_idx, frame_out_idx;
static uint8_t spi_in[52], spi_out[52], spi_idx;
static bool rf_spi_request;
static bool rf_cmdout_sent;

/*
 * Initialize (clear) the communication buffers
 */
void initCommunicationBuffers(void)
{
    FifoClear(core_cmdout);
    FifoClear(rf_cmdout_buf);
    /* Necessary to clear the outgoing command, especially after sleep. */
    spi_out[SPI_DATA_OFFSET] = 0;
}

/* INT1 (PD3) Interrupt on TXE signal. */
//ISR(SIG_INTERRUPT1)
ISR(INT1_vect) /* Mise à jour 03/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    rf_spi_request = true;
    rf_txe = true;
}

void start_rf_spi(void)
{
    //PORTB |= 0x80; // XXX DEBUG
    //if (spi_idx) // XXX debug
        //PORTB |= 0x80; // XXX DEBUG
    spi_idx = 0;

    flash_onhold();
    rf_select();
    SPDR = spi_out[spi_idx];
    EIMSK |= _BV(INT0);
    EIFR |= _BV(INT0);
    //PORTB &= ~0x80; // XXX DEBUG
}

/* INT0 (PD0) Interrupt on SPIACK signal. */
//ISR(SIG_INTERRUPT0)
ISR(INT0_vect) /* Mise à jour 03/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    //PORTB |= 0x80; // XXX DEBUG
    /* Check if we got the start of SPI frame (TXE). */
    /* XXX try to remove this */
    if (PORTB & _BV(PB2))
        return;
    /* Check that the previous SPI transmission is done. */
    if ((SPSR & 0x80) == 0)
        return;

    spi_in[spi_idx++] = SPDR;

    if (spi_idx == SPI_SIZE)
    {
        /* Unselect by waiting here otherwise we would have to use the SPI
         * interrupt to do it. */
        rf_unselect();
        EIMSK &= ~_BV(INT0);
        flash_enable();
        rf_cmdout_sent = true;
    }
    else
    {
        SPDR = spi_out[spi_idx];
    }
    //PORTB &= ~0x80; // XXX DEBUG
}

/**
 * \brief Adapt the audio output rate to keep the stack at a mean level.
 */
void adapt_audio_rate(void)
{
    static uint8_t prescaler = 0;
    uint8_t static prev_stack_length = 0;
    uint8_t stack_length = AudioFifoLength();
    bool slope = 0;

    if (++prescaler == 0)
    {
        prescaler = 127;
        if (stack_length >= prev_stack_length)
            slope = 1;
        prev_stack_length = stack_length;

        if (slope && stack_length > 50)
            OCR0A--;
        if (!slope && stack_length < 70)
            OCR0A++;

        /* Limits to avoid oscillations when a lot of frames are
         * dropped and to prevent jumping over 255. */
        if (OCR0A > 254)
            OCR0A = 254;
        if (OCR0A < 240)
            OCR0A = 240;
    }
}

bool cmds_sent(void)
{
    return (!FifoLength(core_cmdout) && (i2c_get_status() != I2C_BUSY) &&
            !FifoLength(rf_cmdout_buf) && rf_cmdout_sent);
}

/**
 * \brief Service routine that handles the regular communication tasks like
 * sending and fetching commands.
 */
void communication_task(void)
{
    if (rf_spi_request)
    {
        rf_spi_request = false;
        start_rf_spi();
    }

    /* Fill and process RF data. */
    if (spi_idx >= SPI_SIZE)
    {
        uint8_t config_in;
        static uint8_t config_out;
        uint8_t i;

        spi_idx = 0;
        config_in = spi_in[SPI_CONFIG_OFFSET];

        /* Incoming data */
        if (frame_in_idx != spi_in[SPI_IDX_OFFSET])
        {
            //PORTB |= 0x80; // XXX DEBUG
            frame_in_idx = spi_in[SPI_IDX_OFFSET];
            if ((!(config_in & CFG_DATA_MK)) != (!(config_out & CFG_ACK_MK)))
            {
                /* Parse the command and forward to tuxcore if it isn't
                 * dropped. */
                uint8_t *cmd = &spi_in[SPI_DATA_OFFSET];
                if (!parse_cmd(cmd))
                    queue_core_cmd(cmd);
                /* Ack the data by toggling the bit */
                config_out ^= CFG_ACK_MK;
            }
        }
        //else
            //PORTB |= 0x80; // XXX DEBUG
        if (config_in & CFG_AUDIO_MK && !flashPlay)
        {
            adapt_audio_rate();

            for (i=0; i<AUDIO_SPK_SIZE; i++)
            {
                AudioFifoPut_inl(spi_in[i+SPI_AUDIO_OFFSET]);
            }
        }
        else
        {
            if (frame_without_sound)
                frame_without_sound --;
        }

        /* DEBUG VERSION
         * The first part checks a saw wave, the second part can check for
         * stack overflow. */
        //{
            //uint8_t static tmp2;
            //adapt_audio_rate();

            //for (i=0; i<AUDIO_SPK_SIZE; i++)
            //{
                //uint8_t tmp1;
                //tmp1 = spi_in[i+SPI_AUDIO_OFFSET];
                //FifoPut(PWMFifo, tmp1);
                //if (tmp1 != (uint8_t)(tmp2 + 1))
                //{
                    //PORTB ^= 0x80; // XXX DEBUG
                    //queue_rf_cmd_p(0xFE, tmp1, tmp2, frame_in_idx);
                //}
                //tmp2 = tmp1;
                //XXX DEBUG: used to show when the stack overflows.
                //if (FifoPut(PWMFifo, spi_in[i+SPI_AUDIO_OFFSET]) != FIFO_OK)
                //PORTB |= 0x80; // XXX DEBUG
                //else
                //PORTB &= ~0x80; // XXX DEBUG
            //}
        //}
        /*else*/
        /*PORTB ^= 0x80; // XXX DEBUG*/

        /* Outgoing data, add commands and/or audio. */
        spi_out[SPI_IDX_OFFSET] = frame_out_idx++;
        if ((!(config_out & CFG_DATA_MK)) == (!(config_in & CFG_ACK_MK)) &&
            FifoLength(rf_cmdout_buf) >= CMD_SIZE)
        {
            config_out ^= CFG_DATA_MK;
            rf_cmdout_sent = false;
            for (i=0; i<CMD_SIZE; i++)
            {
                FifoGet(rf_cmdout_buf, &spi_out[i+SPI_DATA_OFFSET]);
            }
        }
        if (MicroFifoLength() >= AUDIO_MIC_SIZE)
        {
            config_out |= CFG_AUDIO_MK;
            for (i=0; i<AUDIO_MIC_SIZE; i++)
            {
                MicroFifoGet_inl(&spi_out[i+SPI_AUDIO_OFFSET]);
            }
        }
        else
        {
            config_out &= ~CFG_AUDIO_MK;
        }
        spi_out[SPI_CONFIG_OFFSET] = config_out;
        rf_txe = false;
    }

    /* If busy, pass. */
    if (i2c_get_status() == I2C_BUSY)
        return;

    /* If nacked, restart. */
    if (msg_out.state == I2C_NACK)
    {
        i2c_send_bytes(&msg_out);
        return;
    }

    /* Parse the received command and forward if it isn't dropped. */
    if (received_cmd)
    {
        if (!parse_cmd(received_cmd))
            queue_rf_cmd(received_cmd);
        received_cmd = NULL;
    }

    /* Send otherwise get commands. */
    if (!send_core_cmds() && !sleep_f)
    {
        get_core_cmd();
    }
}
