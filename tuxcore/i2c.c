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

/* $Id: i2c.c 1133 2008-05-08 09:46:35Z jaguarondi $ */
//#define __AVR_LIBC_DEPRECATED_ENABLE__


#include <avr/io.h>
#include <avr/interrupt.h>

#include "common/defines.h"
#include "i2c.h"

#define TWI_TWCR (_BV(TWINT) | _BV(TWIE) | _BV(TWEA) | _BV(TWEN))
/* send/transmit buffer (outgoing data) */
uint8_t i2cSendData[CMD_SIZE];
uint8_t i2cSendDataIndex;

/* receive buffer (incoming data) */
uint8_t i2cReceiveData[CMD_SIZE];
uint8_t i2cReceiveDataIndex;

/*
 * I2C constants.
 */
/** Value of the R/W bit when sending the slave address in read mode. */
#define I2C_R_BIT 1
/** Value of the R/W bit when sending the slave address in write mode. */
#define I2C_W_BIT 0

/*
 * DEBUG and TEST flags
 */
/* If set, store in a ring buffer all I2C status that occur. */
#define __i2c_debug__ 0
/* If set, some signals will be output on 3 pins, see below. */

/* DEBUG definitions and variables */
#if (__i2c_debug__)
#define I2C_DBG_STACK_SIZE
/* Debug stack, it's a ring buffer that hold all TWI status codes. */
uint8_t twi_debug_stack[I2C_DBG_STACK_SIZE];
uint8_t twi_debug_stack_idx = 0;
#endif

/* I2C status and address variables */
/** High level status of the I2C module. */
static enum i2c_state i2c_state;
/** Pointer to the currently processed I2C message. */
static struct i2c_msg *mt_msg;
/** SLA+R/W value. */
static uint8_t sla_rw;
/** Index for the message buffer. */
static volatile uint8_t buf_idx;

/* function pointer to i2c receive routine */
/* I2cSlaveReceive is called when this processor is addressed as a slave for
 * writing */
static void (*i2c_slave_receive) (uint8_t receiveDataLength,
                                uint8_t * recieveData);
/* I2cSlaveTransmit is called when this processor is addressed as a slave for
 * reading */
static uint8_t(*i2cSlaveTransmit) (uint8_t transmitDataLengthMax,
                                   uint8_t * transmitData);

//i2c_exit();
//i2c_master_send();
//i2c_master_recv();
//static int xfer_read(struct i2c_adapter *adap, unsigned char *buf, int length);
    //i2c_suspend();
    //i2c_resume();

void i2c_slave_receive_handler(void (*i2cSlaveRx_func)
                          (uint8_t receiveDataLength, uint8_t * recieveData))
{
    i2c_slave_receive = i2cSlaveRx_func;
}

/* Set the user function which handles transmitting (outgoing) data as a slave */
void
i2c_slave_transmit_handler(uint8_t(*i2cSlaveTx_func)
                           (uint8_t transmitDataLengthMax,
                            uint8_t * transmitData))
{
    i2cSlaveTransmit = i2cSlaveTx_func;
}

static inline void twi_reset(void)
{
#if (__i2c_debug__)
    twi_debug_stack_idx %= I2C_DBG_STACK_SIZE;
    twi_debug_stack[twi_debug_stack_idx++] = 0x02;
#endif
    /* In case we lost arbitration, we still want to send the start later on so
     * we should 'OR' here and not simply assign. */
    TWCR |= TWI_TWCR;
}

/** Send a START or repeated START condition on the bus.
 * If a STOP condition just occurred on the bus, a START will be sent.
 * Otherwise a repeated START will be sent. */
static inline void twi_send_start(void)
{
#if (__i2c_debug__)
    cli(); /* we need to protect this from an i2c interrupt that will corrupt
              those values otherwise */
    twi_debug_stack_idx %= I2C_DBG_STACK_SIZE;
    twi_debug_stack[twi_debug_stack_idx++] = 0x03;
    sei();
#endif
    TWCR = _BV(TWSTA) | TWI_TWCR;
}

static inline void twi_send_stop(void)
{
#if (__i2c_debug__)
    twi_debug_stack_idx %= I2C_DBG_STACK_SIZE;
    twi_debug_stack[twi_debug_stack_idx++] = 0x04;
#endif
    TWCR = _BV(TWSTO) | TWI_TWCR;
}

static inline void twi_send_data(uint8_t data)
{
#if (__i2c_debug__)
    twi_debug_stack_idx %= I2C_DBG_STACK_SIZE;
    twi_debug_stack[twi_debug_stack_idx++] = 0x05;
#endif
    TWDR = data;
    /* Just clear the interrupt flag. */
    TWCR = TWI_TWCR;
}

static inline uint8_t twi_get_data(void)
{
    return TWDR;
}

static inline void twi_return_ack(void)
{
    TWCR = TWI_TWCR | _BV(TWEA);
}

static inline void twi_return_nack(void)
{
    TWCR = TWI_TWCR & ~_BV(TWEA);
}

static inline void twi_transmit_byte(void)
{
    TWCR = TWI_TWCR | _BV(TWEA);
}

static inline uint8_t twi_get_status(void)
{
    return TWSR;
}

static inline uint8_t twi_poll_status(void)
{
    while (!(TWCR & _BV(TWINT)));
    return TWSR;
}


/*
 * I2C prototypes.
 */

/**
 * This function initializes the TWI interface for I2C communication and sets
 * the I2C status.
 */
void i2c_init(void)
{
    i2c_state = I2C_IDLE;
    /* Set pull-up resistors on TWI bus pins */
    PORTC |= _BV(PC5);
    PORTC |= _BV(PC4);
    /* Set local device address (used in slave mode only) */
    TWAR = (I2C_SLA_ADDRESS << 1);
    /* Set the bus clock speed. */
    TWSR = 0x60;
    TWBR = 0x20;
    /* Set the TWI module in the requested mode. */
    TWCR = TWI_TWCR;
}

int8_t i2c_send_bytes(struct i2c_msg *msg)
{
    if (i2c_state == I2C_BUSY)
        return -1;

    mt_msg = msg;
    i2c_state = I2C_BUSY;
    sla_rw = (msg->addr << 1) + I2C_W_BIT;
    buf_idx = 0;
    twi_send_start();
    return 0;
}

uint8_t i2c_read_bytes(struct i2c_msg *msg)
{
    if (i2c_state == I2C_BUSY)
        return -1;

    mt_msg = msg;
    i2c_state = I2C_BUSY;
    sla_rw = (msg->addr << 1) + I2C_R_BIT;
    buf_idx = 0;
    twi_send_start();
    return 0;
}

/*
 * Start a Master transmission
 */
void i2cMasterStart(void)
{
    volatile uint8_t i;
    for (i=0; i<0xFF; i++);
    /* note: when a stop is immediately followed by a start, the slave detects
     * the stop, need to process the received data and exit the ISR. If it
     * doesn't have enough time to do this before a new start is set, it will
     * still be in ISR when the start occurs and won't detect it because the
     * TWI hardware is said to be partly disabled when TWINT is set.
     * Two things to do here, exit the ISR as soon as possible in the slave,
     * and add some delay between a stop and a start in the master. Use a
     * repeated start condition in such a case. */
        twi_send_start();
}

enum i2c_state i2c_get_status(void)
{
    return i2c_state;
}

/* TWI interrupt service routine */
//ISR(SIG_TWI)
ISR(TWI_vect) /* Mise à jour 02/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
#if (__i2c_debug__)
    twi_debug_stack_idx %= I2C_DBG_STACK_SIZE;
    twi_debug_stack[twi_debug_stack_idx++] = TW_STATUS;
#endif

    switch(TW_STATUS)
    {
        /*
         * Slave mode
         */
        case TW_SR_SLA_ACK: /* 0x60: own SLA+W has been received, ACK has been returned */
        case TW_SR_ARB_LOST_SLA_ACK: /* 0x68: own SLA+W has been received, ACK has been returned */
        case TW_SR_GCALL_ACK: /* 0x70:     GCA+W has been received, ACK has been returned */
        case TW_SR_ARB_LOST_GCALL_ACK: /* 0x78:     GCA+W has been received, ACK has been returned */
            i2c_state = I2C_BUSY;
            i2cReceiveDataIndex = 0;
            twi_return_ack();
            break;
        case TW_SR_DATA_ACK: /* 0x80: data byte has been received, ACK has been returned */
        case TW_SR_GCALL_DATA_ACK: /* 0x90: data byte has been received, ACK has been returned */
            i2cReceiveData[i2cReceiveDataIndex++] = TWDR;
            if (i2cReceiveDataIndex < 4)
                twi_return_ack();
            else
                twi_return_nack();
            break;
        case TW_SR_DATA_NACK: /* 0x88: data byte has been received, NACK has been returned */
        case TW_SR_GCALL_DATA_NACK: /* 0x98: data byte has been received, NACK has been returned */
            twi_reset(); /* reset TWI in slave mode */
            break;
        case TW_SR_STOP: /* 0xA0: STOP or REPEATED START has been received while addressed as slave */
            if (i2cReceiveDataIndex == 4)
                i2c_slave_receive(i2cReceiveDataIndex, i2cReceiveData);
            i2c_state = I2C_IDLE;
            twi_reset(); /* reset TWI in slave mode */
            break;

        case TW_ST_SLA_ACK: /* 0xA8 */
            /* Own SLA+R has been received, ACK has been returned. */
        case TW_ST_ARB_LOST_SLA_ACK: /* 0xB0 */
            /* Arbitration lost in SLA+RW as Master; own SLA+R has been
             * received, ACK has been returned. */
            i2c_state = I2C_BUSY;
            if (i2cSlaveTransmit)
                if (i2cSlaveTransmit(4, i2cSendData))
                    i2cSendDataIndex = 0;
        case TW_ST_DATA_ACK: /* 0xB8 */
            /* Data byte in TWDR has been transmitted, ACK has been received.
             */
            if (i2cSendDataIndex < 4)
                TWDR = i2cSendData[i2cSendDataIndex++];
            else
                TWDR = 0; /* XXX Should be all '1' here normally */
            /* XXX Should use the last byte transmission, check the I2C specs
             * first. */
            twi_transmit_byte();
            break;
        case TW_ST_DATA_NACK: /* 0xC0 */
            /* Data byte in TWDR has been transmitted, NOT ACK has been
             * received. */
        case TW_ST_LAST_DATA: /* 0xC8 */
            /* Last data byte in TWDR has been transmitted (TWEA = "0"), ACK
             * has been received. */
            /* In both cases, we switch to the not adressed Slave mode; own SLA
             * will be recognized. */
            i2c_state = I2C_IDLE;
            TWDR = 0;
            twi_return_ack();
            break;

        /*
         * Miscellaneous States
         */
        case TW_NO_INFO: /* OxF8 */
            /* No relevant information is available because the TWINT Flag is
             * not set. This occurs between other states, and when the TWI is
             * not involved in a serial transfer. */
            /* We can't have this value here as this condition only happens
             * when the TWI interrupt flag is not set, which we can't get in
             * this interrupt. So treat this as an error. */
            /* XXX should log the error */
            break;
        case TW_BUS_ERROR: /* 0x00 */
            /* Bus error due to a START or STOP condition that occuerd at an
             * illegal position in the format frame. */
            /* To recover from a bus error, the TWSTO Flag must set and TWINT
             * must be cleared by writing a logic one to it. This causes the
             * TWI to enter the not addressed Slave mode and to clear the TWSTO
             * Flag (no other bits in TWCR are affected).
             * Only the internal hardware is affected, no STOP condition is
             * sent on the bus. In all cases, the bus is released and TWSTO is
             * cleared. */
            twi_send_stop();
            /* XXX should log the error */
            break;
        default:
            break;
    }
}
