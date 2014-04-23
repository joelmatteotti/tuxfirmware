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

/* $Id: ir.h 479 2007-09-05 09:45:34Z jaguarondi $ */

#ifndef IR_H
#define IR_H

/*
 * irStatus
 */
extern uint8_t volatile irStatus;

#define IRSTATUS_MODE           _BV(0)  /* Cleared in Send mode, set in Receiver mode */
#define IRSTATUS_MODE_SEND      0x00
#define IRSTATUS_MODE_GET       0x01
#define IRSTATUS_EMIT           _BV(1)  /* Turn the IR LED modulation on */
#define IRSTATUS_PHASE          _BV(2)  /* Send second phase of bit */
#define IRSTATUS_END            _BV(3)  /* End of IR transmission */
#define IRSTATUS_MEASURE        _BV(4)  /* Measure state */
#define IRSTATUS_RECEIVING      _BV(5)  /* Receiving state, we change irReceivedCode in this stage only */
#define IRSTATUS_REC_WAIT       _BV(6)  /* Wait next bit interrupt at the receiver */

extern uint8_t volatile irPulses;       /* Counter for IR transmission */

#define SILENCE_PULSES          0x80

#define RC5_BIT_NUMBER          14

typedef volatile struct
{
    uint16_t code;
    uint8_t repeat:7;
    uint8_t biPhase:1;
    uint8_t bit;
    uint8_t pulse;
}
RC5_STRUCT;

/*!
 *  \ingroup ir
 *  \brief RC5 IR structure
 *
 * -# irAddress[5:0] : [T,Address(5 bits)]
 *     - T: toggle bit which toggles each time you press the same button
 *     - Address: 5 bits RC5 address
 * -# irCommand[5:0] : Command(6 bits)
 *     - Command: 6 bits RC5 command
 */
typedef struct
{
    uint8_t irCommand;          /* IR COMMAND */
    uint8_t irAddress;          /* IR RC5 ADDRESS */
}
RC5_DATA;

extern volatile RC5_STRUCT irRC5SendData;
extern volatile RC5_DATA irRC5ReceivedData;
extern volatile uint16_t irReceivedCode;

#define enableIrLed()           (TCCR0A |= _BV(COM0B1))
#define disableIrLed()          (TCCR0A &= ~_BV(COM0B1))
#define enableIrExtint()        {EICRA |= _BV(ISC00); EIFR |= _BV(INT0); EIMSK |= _BV(INT0);}
#define disableIrExtint()       EIMSK &= ~_BV(INT0)

void initIR(void);

/*!
 *  \brief Turn IR led on
 *  \ingroup ir
 */
void turnIrOn(void);

/*!
 *  \brief Turn IR led Off
 *  \ingroup ir
 */
void turnIrOff(void);

/*!
 *  \brief Send RC5 ir code
 *  \ingroup ir
 *  \param address - xxTAAAAA (T: toggle bit, A: RC5 5 bits address)
 *  \param command - xxCCCCCC (C: RC5 6 bits command)
 *  \note Tux can't send and receive IR at the same time because the code sent is also detected by the receiver and also because they share the same timer. So irSendRC5 will disable irGetRC5 when sending data.
 *
 *  Send an ir code following the Philips RC-5 Protocol. The RC-5 code from Philips is possibly the most used protocol by hobbyists, probably because of the wide availability of cheap remote controls.
 *  For more information on a couple of protocols, visit http://www.sbprojects.com/knowledge/ir/rc5.htm or google for it :-)
 */
void irSendRC5(uint8_t address, uint8_t command);

/*!
 *  \ingroup ir
 *  \brief Enables the IR receiver in RC5 format
 *
 *  Enables the IR receiver to capture RC5 ir frames. They'll be available through global status in 2 bytes:
 * -# irAddress[5:0] : [T,Address(5 bits)]
 *     - T: toggle bit which toggles each time you press the same button
 *     - Address: 5 bits RC5 address
 * -# irCommand[5:0] : Command(6 bits)
 *     - Command: 6 bits RC5 command
 */
void irGetRC5(void);

/*
 * Resumes the IR receiver to the state it was before a call to "disableIR()". The IR receiver will be turned on if you previously called an irGet.*() function.
 */
void resumeIR(void);

void disableIR(void);

/*!
 *  \brief Turn off the IR receiver
 *  \ingroup ir
 */
void stopIRReceiver(void);

#endif
