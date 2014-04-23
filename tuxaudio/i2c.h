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

/* $Id: i2c.h 1304 2008-07-02 15:08:50Z jaguarondi $ */

#ifndef _I2C_H_
#define _I2C_H_

#include <stdbool.h>
#include <util/twi.h>

enum i2c_state
{
    I2C_IDLE,
    I2C_BUSY,
    I2C_ACK,
    I2C_NACK,
    I2C_FULL,
    I2C_ARB_LOST,
};

/**
 * I2C Message - used for pure i2c transaction.
 * XXX see what to do with the flags.
 */
struct i2c_msg {
        uint8_t addr; /**> Slave address, the AVR only supports 7-bit address
                        space. */
        uint8_t len; /**> Message length. */
        uint8_t *buf; /**> Pointer to msg data. */
        enum i2c_state state;
};

extern void i2c_init(void);
extern int8_t i2c_send_bytes(struct i2c_msg *msg);
extern uint8_t i2c_read_bytes(struct i2c_msg *msg);
extern uint8_t i2c_get_addr(void);
extern enum i2c_state i2c_get_status(void);
extern void i2c_clr_status(void);

/************************************************************
 * OLD STUFF */

/*! types */
//typedef enum
//{
    //I2C_IDLE = 0, I2C_BUSY = 1,
    //I2C_MASTER_TX = 2, I2C_MASTER_RX = 3,
    //I2C_SLAVE_TX = 4, I2C_SLAVE_RX = 5
//} eI2cStateType;

/* I2C state and address variables */
extern uint8_t i2cDeviceAddrRW;

/* send/transmit buffer (outgoing data) */
extern uint8_t i2cSendData[];
extern uint8_t i2cSendDataIndex;
extern uint8_t i2cSendDataLength;

/* receive buffer (incoming data) */
extern uint8_t i2cReceiveData[];
extern uint8_t i2cReceiveDataIndex;
extern uint8_t i2cReceiveDataLength;

/* Functions */
void i2cInit(void);
void i2c_master_receive_handler(void (*i2cMasterRx_func) (struct i2c_msg *msg));
void
i2cSetSlaveTransmitHandler(uint8_t(*i2cSlaveTx_func)
                           (uint8_t transmitDataLengthMax,
                            uint8_t * transmitData));
void i2cMasterStart(void);

#endif /* _I2C_H_ */
