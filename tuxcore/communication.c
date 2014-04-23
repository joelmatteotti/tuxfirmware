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

/* $Id: communication.c 1133 2008-05-08 09:46:35Z jaguarondi $ */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "common/defines.h"
#include "global.h"
#include "communication.h"
#include "i2c.h"

/** Size of the incoming stack buffer. */
#define CMD_IN_BUF_SIZE 32
/** Size of the outgoing stack buffer. */
#define CMD_OUT_BUF_SIZE 32

FIFO_INSTANCE(cmdin_buf, CMD_IN_BUF_SIZE);
/** Stack for commands received from tuxaudio */
fifo_t *cmdin = FifoPointer(cmdin_buf);

FIFO_INSTANCE(cmdout_s, CMD_OUT_BUF_SIZE);
/** Stack for commands to tuxaudio */
fifo_t *cmdout = FifoPointer(cmdout_s);

/**
 * \brief Called at the end of the I2C slave reception to process received
 * data.
 * \param data_length Length of the data buffer.
 * \param data Buffer of data received.
 */
void i2c_slave_receive_service(uint8_t data_length, uint8_t *data)
{
    uint8_t i;

    /* We should have the right number of bytes. */
    if (data_length != CMD_SIZE)
    {
        gerror = GERROR_INV_RECEIVE_LENGTH;
        return;
    }
    /* And enough place to store it. */
    if (FifoLength(cmdin) > FifoSize(cmdin) - CMD_SIZE)
    {
        gerror = GERROR_CMDINBUF_OVF;
        return;
    }
    for (i=0; i<CMD_SIZE; i++)
        FifoPut(cmdin, data[i]);
}

/**
 * \brief Called when an I2C read transaction occurs. It fetches data to be
 * sent to tuxaudio.
 * \param data_length Length of the data buffer.
 * \param data Buffer of data to be transmitted.
 * \return 0 if nothing has to be sent, 1 otherwise.
 */
uint8_t i2c_slave_transmit_service(uint8_t data_length, uint8_t* data)
{
    uint8_t i;

    if (!FifoLength(cmdout))
    return 0;

    for (i = 0; i < data_length; i++)
    {
        if (FifoGet(cmdout, &data[i]) != FIFO_OK)
        {
            gerror = GERROR_CMDOUTBUF_EMPTY;
            return 0;
        }
    }
    return 1;
}

/**
 * \brief Get a command from the stack of received commands from tuxaudio.
 * \return 0 if the stack is empty, 1 if a command has been received
 * successfully.
 */
int8_t get_cmd(uint8_t *cmd)
{
    uint8_t i;

    if (!FifoLength(cmdin))
        return 0;

    for (i=0; i<CMD_SIZE; i++)
    {
        if (FifoGet(cmdin, &cmd[i]) != FIFO_OK)
        {
            gerror = GERROR_CMDINBUF_EMPTY;
            return 0;
        }
    }
    return 1;
};

/**
 * \brief Add a command on the status stack to be sent to tuxaudio.
 * \param cmd Command array.
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_cmd(uint8_t *cmd)
{
    uint8_t i;

    if (FifoLength(cmdout) > FifoSize(cmdout) - CMD_SIZE)
        return 0;
    cli();
    for (i=0; i<CMD_SIZE; i++)
        if (FifoPut(cmdout, cmd[i]) != FIFO_OK)
        {
            gerror = GERROR_CMDOUTBUF_FULL;
            return 0;
        }
    sei();
    return 1;
}

/**
 * \brief Add a command on the status stack to be sent to tuxaudio.
 * \param cmd Command byte.
 * \param param1 First parameter.
 * \param param2 Second parameter.
 * \param param3 Third parameter.
 * \return 0 if the stack is full, 1 if the command has been added
 * successfully.
 */
int8_t queue_cmd_p(uint8_t cmd, uint8_t param1, uint8_t param2, \
                     uint8_t param3)
{
    uint8_t c[4] = {cmd , param1, param2, param3};
    return queue_cmd(c);
}

/**
 * \brief Check if all commands are completely sent through I2C.
 * \return true if all commands are sent, false otherwise.
 */
bool cmds_sent(void)
{
    return !(FifoLength(cmdout) || (i2c_get_status() == I2C_BUSY));
}

/**
 * \brief Check if the command stack is empty.
 * \return true if stack is empty.
 * It's possible that the stack is empty but the command is in the I2C buffer.
 * This doesn't ensure that the command has been received on the other side.
 */
bool cmds_empty(void)
{
    return !FifoLength(cmdout);
}

/**
 * \brief I2C communication initialization
 *
 * Initialize the I2C interface in slave mode.
 */
void communication_init(void)
{
    i2c_init();

    /* Set receive and transmit handlers. */
    i2c_slave_receive_handler(i2c_slave_receive_service);
    i2c_slave_transmit_handler(i2c_slave_transmit_service);

    FifoClear(cmdout);
}
