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

/* $Id: main.c 2994 2008-12-03 13:20:41Z ks156 $ */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "common/defines.h" /* Besoin d'être ici pour le define de F_CPU nécessaire à util/delay.h [03/12/2013 - Joël Matteotti <sf user: joelmatteotti>] */
#include <util/delay.h>
#include <avr/sleep.h>

#include "hardware.h"

#include "common/api.h"
#include "init.h"
#include "varis.h"
#include "spi.h"
#include "fifo.h"
#include "i2c.h"
#include "communication.h"
#include "parser.h"
#include "flash.h"
#include "config.h"
#include "audio_fifo.h"
#include "micro_fifo.h"

/*
 * Debug and test flags
 */
#define DBG_STACK 0

/*
 * Stack Overflow detection
 *
 * Fill the ram with a value (0x5F) before the first initialization in order to
 * detect any stack overflow just by looking to the memory at any breakpoint
 */
#if (DBG_STACK)
extern uint16_t __heap_start;
void init_ram(void) __attribute__ ((naked)) __attribute__ ((section(".init1")));
void init_ram(void)
{
    uint8_t *ptr;

    for (ptr = (uint8_t *) &__heap_start; ptr < (uint8_t *) 0x800500; ptr++)
        *ptr = 0x5F;
}

int8_t check_stack(void)
{
    uint8_t *ptr, cnt=0;

    for (ptr = (uint8_t *) &__heap_start; ptr < (uint8_t *) 0x800500; ptr++)
    {
        if (*ptr == 0x5F)
            cnt++;
        else
            cnt = 0;
        if (cnt == 10)
            return 1;
    }
    return 0;
}
#else
#define check_stack() 1
#endif

/* When sensor values should be sent to the computer */
static bool send_sensors_flag;

//ISR(SIG_PIN_CHANGE1)
ISR(PCINT1_vect) /* Mise à jour 03/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    /* Nothing to do here, it's just an interrupt set on the head button to
     * wake-up from sleep */
    /* This ISR should be declared as any interrupt which doesn't have an
     * interrupt vector initialized will loop infinitely. */
}

void sleep(void)
{
    uint8_t PRR_bak;

    cli();
    sleep_f = false;
    rf_txe = false;

    /* Stop flash, I2C and SPI */
    enter_deep_sleep();
    TWCR = _BV(TWINT);
    SPCR = 0x00;

    /* Configure I/O */
    PORTB = 0x80;
    /* XXX strangely we have to let the flippers as PU otherwise tuxaudio
     * doesn't wake up when they both are drawned to 0 (when pushing them
     * both). To be checked further. To force the problem, set DDRC |= 0x03 and
     * PORTC = 0x08 and the problem always happen. */
    PORTC = 0x0B;
    /* XXX IR receiver cut off, we have to choose if we want to keep it on or
     * off */
    PORTD = 0;

    /* Stop timers */
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TIFR0 = 0x07;
    TCCR2B = 0;
    TIMSK2 = 0;
    TIFR2 = 0x07;

    /* Stop the ADC */
    ADCSRA = _BV(ADIF);

    /* Stop external interrupts and clear flags */
    EIMSK = 0;
    EIFR = (_BV(INT1) | _BV(INT0));

    /* Reduce power */
    PRR_bak = PRR;
    PRR = _BV(PRTWI) | _BV(PRTIM2) | _BV(PRTIM0) | _BV(PRTIM1) | _BV(PRSPI) |
        _BV(PRUSART0) | _BV(PRADC);

    /* Sleep. */
    /* Set pin change interrupt on head button */
    PCMSK1 = 0x08;
    PCIFR = _BV(PCIE1);
    PCICR = _BV(PCIE1);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    cli();

    /* Re-configure in normal mode. */
    PCMSK1 = 0x00;
    PRR = PRR_bak;

    init_avr();
    /* Power the microphone */
    volatile uint16_t _count;
    for (_count = 0; _count<0x0DFF; _count++)
    {
        volatile uint8_t i;
        for (i=0x0F; i>((_count>>8)+2); i--)
            DDRD &= ~0x03;
        for (; i>0;i--)
            DDRD |= 0x03;
    }

    initCommunicationBuffers();
    communication_init();
    leave_deep_sleep();

    /* Activate the interrupts used by the RF module. */
    EIMSK = (_BV(INT1) | _BV(INT0));

    sei();
}

/**
 * \brief Send sensors status
 *
 * Acquire and send to the behavioural CPU the status of the switches, the
 * audio level and the battery voltage
 */
void sendSensors(void)
{
    uint8_t command[4];
    static uint16_t disconnected_timeout = 0x1000;

    command[0] = SEND_AUDIOSENSORS_CMD;
    /* Get switches, charger status , RF status and mute status */
    command[1] = (PINC & 0x0F) | (PIND & 0xD0);
    if (RF_ONLINE_PIN & RF_ONLINE_MK)
    {
        command[1] |= 0x20;
        disconnected_timeout = 0x1000;
    }
    else
    {
        /* RF disconnected, check timeout before sleep */
        if (!--disconnected_timeout)
        {
            uint8_t cmd[4] = {SLEEP_CMD, SLEEPTYPE_QUICK, 0, 0};
            parse_cmd(cmd);
            disconnected_timeout = 0x1000;
        }
    }
    /* Some bits should be inverted for '1' to represent the action (pressed,
     * plugged) */
    command[1] ^= 0x9B;
    command[2] = soundToPlay;
    if (soundToPlay || sound_played)
        command[3] = 1;
    else
        command[3] = 0;
    queue_core_cmd(command);
}

int main(void)
{
    init_avr();

    /* Reset the RF to eliminate timing uncertainties */
    PORTB &= ~0x80;
    /* Power the microphone */
    volatile uint16_t _count;
    for (_count = 0; _count<0x0DFF; _count++)
    {
        volatile uint8_t i;
        for (i=0x0F; i>((_count>>8)+2); i--)
            DDRD &= ~0x03;
        for (; i>0;i--)
            DDRD |= 0x03;
    }
    /* Start the RF */
    PORTB |= 0x80;

    /* Other initializations */
    AudioFifoClear();
    MicroFifoClear();
    /* Load configuration defaults from EEPROM */
    config_init();
    numSound = readFlashNumber();
    last_block = readLastBlock(numSound);
    communication_init();

    /* Wait for the RF board to signal itself with a stong low on PB6 */
    //while (RF_ONLINE_PIN & RF_ONLINE_MK);

    /* Activate the interrupts used by the RF module. */
    EIFR = (_BV(INT1) | _BV(INT0));
    EIMSK = (_BV(INT1) | _BV(INT0));
    sei();

    while (1)
    {
        sei(); /* XXX can we remove this? */

        /* Check for stack overflow. */
        if (!check_stack())
            while(1)
            {
                PORTB |= 0x80;
                PORTB &= ~0x80;
            }

        if (!rf_txe)
        {
            if (programmingFlash)
            {
                programming();
            }

            if (flashPlay)
                playSound();

            if (eraseFlag)
                erase();
        }

        /* Send commands to I2C, otherwise get new status from tuxcore */
        communication_task();

        /* Sleep mode */
        /* Finish playing the sound to close the SPI cleanly and wait for
         * the current I2C transaction to be done. */
        if (sleep_f == true && !flashPlay && cmds_sent())
        {
            /* Set rf in sleep too. */
            queue_core_cmd_p(SLEEP_CMD, SLEEPTYPE_QUICK, 0, 0);
            while (!cmds_sent())
                communication_task();
            sleep();
        }

        if (send_sensors_flag && !sleep_f)
        {
            send_sensors_flag = false;
            sendSensors();
            /* XXX debug of the audio stack */
            //queue_rf_cmd_p(0xFE, FifoLength(PWMFifo), OCR0A, 0);
        }
    }
}

/* Timer 2 interrupt, used as main tick for sending sensor values.
 * clk = 8MHz/1024,
 * ISR each 256*1024/8e6 = 33ms */
//ISR(SIG_OVERFLOW2)
ISR(TIMER2_OVF_vect) /* Mise à jour 03/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    send_sensors_flag = true;
}

//static inline void audio_sampling(void) __attribute__ ( ( always_inline ) );
//void audio_sampling(void) __attribute__ ( ( signal ) );
//void audio_sampling(void)
//ISR(audio_sampling)
//{
    //ADCSRA |= 0x40;             // ADC started at  XXX a laisser la ??

//#ifndef MIC_GAIN
//#define MIC_GAIN    0 [> default value if not declared in the makefile <]
//#endif
//#if (MIC_GAIN == 6)
    //[> MEDIUM GAIN MODE <]
    //uint8_t tmp;

    //[> Audio in: microphone <]
    //tmp = ADCL;
    //tmp = tmp >> 1;
    //if (!(ADCH & 0x01)) [> AND the 9th bit then add 0x80 is equivalent to AND the complement of the 9th bit <]
        //tmp |= 0x80;
    //FifoPut_inl(ADCFifo, tmp);
    //[> HIGH GAIN MODE <]
//#elif (MIC_GAIN == 12)
    //uint8_t tmp;

    //tmp = ADCL + 0x80;
    //FifoPut_inl(ADCFifo, tmp);
    //asm volatile ("lds __tmp_reg__, %0"::"M" (_SFR_MEM_ADDR(ADCH)));    [> ADCH needs to be read for the next acquisition <]
//#else [> (MIC_GAIN == 0) or anything else <]
    //[> LOW GAIN MODE <]
    //[>if (ADCH == 0)      [> XXX RF looses connection if too much '0' are sent, but the noise should normally be enough to avoid that <]<]
    //[>FifoPut(ADCFifo, 0x01);<]
    //[>else<]
    //FifoPut_inl(ADCFifo, ADCH);
//#endif

    //FifoGet_inl(PWMFifo, (uint8_t *)&OCR0B); [> set the sample value to timer pulse width <]
    //[>if (!FifoGet(PWMFifo, (uint8_t *)&OCR0B))  [> set the sample value to timer pulse width <]<]
    //[>{<]
    //[>Fifoinert = 0;<]
    //[>if (tuxaudio_config.automute)   [> XXX mute functions should not be called here each time a sample is placed, this is silly <]<]
    //[>unmute_amp();<]
    //[>}<]
    //[>else<]
    //[>{<]
    //[>Fifoinert++;<]
    //[>if (Fifoinert >= 30)<]
    //[>{<]
    //[>if (tuxaudio_config.automute)<]
    //[>mute_amp();<]
    //[>Fifoinert = 30;<]
    //[>//OCR0A = 250; // Normal operation for ADC sampling if FIFO Adaptative is on<]
    //[>}<]
    //[>}<]
    //[> XXX we should move this timer away from here <]
    /* send status to the behavioural CPU, 8KHz divided by 256 lead to a
     * status sent each 32ms */
    //if (--sendSensorsCmpt == 0)
        //send_sensors_flag = 1;
    ////asm volatile (
        ////"pop r25 \n"
        ////"pop r23 \n"
        ////"pop r22 \n"
        ////::);
//}

ISR(__vector_audio_sampling)
{
    //FifoPut(ADCFifo, ADCH);

    /* Speaker at 16kHz */
    AudioFifoGet_inl((uint8_t *)&OCR0B);

    if (sampling_pwm & 0x02)
    {
        /* Microphone at 8kHz */
        MicroFifoPut_inl(ADCH);
        ADCSRA |= 0x40;
    }

    // XXX DEBUG: used to generate a saw waveform
    //static uint8_t cnt = 0;
    //static uint8_t dir = 1;
//#define STEP 8
    //OCR0B = cnt;
    //if (dir)
        //cnt += STEP;
    //else
        //cnt -= STEP;
    //if ((cnt >= (0xFF - STEP)) || (cnt == 0))
        //dir = !dir;
}

/*
 * Timer 0 overflow interrupt service routine (PWM)
 * 32KHz timer so the ISR is called each 31.25us
 */
#if 1
//ISR(SIG_OVERFLOW0)
ISR(TIMER0_OVF_vect) /* Mise à jour 03/12/2013 - Joël Matteotti <sf user: joelmatteotti> */
{
    /* timer 0 is clocked at 32KHz so we divide it by 4 to get 8KHz */
    if (++sampling_pwm & 0x01)
    {
        asm("rcall __vector_audio_sampling"::);
        cli();
    }
}
#endif

#if 0
ISR(SIG_OVERFLOW0) __attribute__((naked));
ISR(SIG_OVERFLOW0)
{
    asm volatile (
        "in	r0, 0x3f" "\n\t"
        "push	r28" "\n\t"
        "lds	r28, 0x0100" "\n\t"
        "subi	r28, 0xFF" "\n\t"
        "sts	0x0100, r28" "\n\t"
        "sbrs	r28, 0" "\n\t"
        "rjmp	v_out" "\n\t"
        "rcall __vector_audio_sampling" "\n\t"
        "cli" "\n\t"
        "v_out:"
        "pop	r28" "\n\t"
        "out	0x3f, r0" "\n\t"
        ::);
    asm ("reti" "\n\t"::);
}
#endif
//ISR(SIG_OVERFLOW0) __attribute__((naked));
//ISR(SIG_OVERFLOW0)
//{
    ////asm volatile (
        ////"in __tmp_reg__,__SREG__" "\n\t"
        ////"push r24" "\n\t"
        ////"lds r24, sampling_pwm" "\n\t"
        ////"subi r24, 0x01" "\n\t"
        ////"andi r24, 0x03" "\n\t"
        ////"sts sampling_pwm, r24" "\n\t"
        ////"brne 1f" "\n\t"
        ////"rcall audio_sampling" "\n\t"
        ////"1: cli" "\n\t"
        ////"1: pop r24" "\n\t"
        ////"out  __SREG__, __tmp_reg__" "\n\t"
        ////"reti" "\n\t"
        ////::);
//}
