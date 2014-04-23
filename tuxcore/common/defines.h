/*
 * TUXDEFS - Common defines used by the firmware and daemon of tuxdroid
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

/* $Id: defines.h 2253 2008-10-07 15:15:52Z jaguarondi $*/

/** \file defines.h
    \brief Common firmware constants, structures and types.

    \ingroup common
*/

/** \defgroup common Common firmware constants, structures and types.
 */
/*!  @{ */

#ifndef _DEFINES_H_
#define _DEFINES_H_

#ifndef _BV
#define _BV(x) (1<<x)
#endif

#include <stdint.h>
/**
 * \name Identifiers and addresses.
 *  @{ */
/**
 * CPU identifiers.
 */
enum CPU_IDENTIFIERS
{
    /** 'tuxcore' is the core CPU of tuxdroid, it manages the sensors and
     * actuators. */
    TUXCORE_CPU_NUM = 0,
    /** 'tuxaudio' is the CPU of tuxdroid that handles audio (microphone,
     * speaker and the audio flash) and communications between the RF and
     * tuxcore. */
    TUXAUDIO_CPU_NUM = 1,
    /** 'tuxrf' is the RF CPU of tuxdroid. */
    TUXRF_CPU_NUM = 2,
    /** 'fuxrf' is the RF CPU inside the dongle. */
    FUXRF_CPU_NUM = 3,
    /** 'fuxusb' is the USB CPU inside the dongle. */
    FUXUSB_CPU_NUM = 4,
    /** We use '-1' to indicate an invalid CPU number. */
    INVALID_CPU_NUM = -1,
};
/** Lowest valid value for a CPU identifier.
 * Should be set according to CPU_IDENTIFIERS. */
#define LOWEST_CPU_NUM 0
/** Highest valid value for a CPU identifier.
 * Should be set according to CPU_IDENTIFIERS. */
#define HIGHEST_CPU_NUM 4

/**
 * I2C addresses of all CPUs.
 *
 * tuxcore, tuxaudio, tuxrf and fuxrf have different I2C addresses when they're
 * running in bootloader mode (BL). In normal mode, only tuxcore has an I2C
 * address as tuxaudio is master.
 */
enum I2C_ADDRESSES
{
    TUXCORE_BL_ADDR = 0X30,
    TUXAUDIO_BL_ADDR = 0X31,
    TUXRF_BL_ADDR = 0X32,
    FUXRF_BL_ADDR = 0X33,
    TUXCORE_ADDR = 0x2A,
};

/*! @} */

/**
 * \name Hardware protocol
 * Protocol between the computer program (daemon, driver, etc.) and all MCUs
 * (USB, RF, core and audio).
 */
/*! @{ */
/** Size of a command in the main communication protocol. */
#define CMD_SIZE 4

/*! @} */

/**
 * \name Versioning
 */
/*! @{ */
/**
 * Version structure (DEPRECATED).
 * This version_bf_t type can't be used to send the bytes over a channel as the
 * order of the bits in the bit field may be freely rearranged by GCC.
 *
 * \todo TODO check that no firmware is using this anymore and delete this.
 */
typedef struct
{
    uint8_t version_cmd;
    uint8_t cpu_nbr:3; /**< 3 lower bits are the CPU number */
    uint8_t ver_major:5; /**< 5 higher bits are the major version number */
    uint8_t ver_minor;
    uint8_t ver_update;
} version_bf_t;

/**
 * Version structure that holds the raw data that will be sent when the
 * VERSION_CMD is received.
 *
 * This version_t type should be used to send the bytes over a channel such as
 * USB or TCP-IP. Use the macros to set or retrieve values of the cpu_ver_maj
 * byte.
 *
 * See also VERSION_CMD.
 */
typedef struct
{
    uint8_t version_cmd;
    uint8_t cpu_ver_maj; /** 3 lower bits are the CPU number, 5 higher bits are
                            the major version number */
    uint8_t ver_minor;
    uint8_t ver_update;
} version_t;

/** Concatenate the CPU number and the major version number to create
 * 'cpu_ver_maj'.
 * \param cpu_nbr  CPU number.
 * \param ver_major  major version number.
 * \return cpu_ver_maj byte.*/
#define CPU_VER_JOIN(cpu_nbr, ver_major) ((cpu_nbr & 0x7) + (ver_major << 3))
/** Return the CPU number from 'cpu_ver_maj'. */
#define CPU_VER_CPU(cpu_ver_maj) (cpu_ver_maj & 0x07)
/** Return the major version number from 'cpu_ver_maj'. */
#define CPU_VER_MAJ(cpu_ver_maj) ((cpu_ver_maj & 0xF8) >> 3)

/**
 * Revision information from the subversion repository.
 *
 * See also REVISION_CMD.
 */
typedef struct
{
    uint8_t revision_cmd;
    /** Revision number. */
    uint16_t revision;
    /** Release type coded as a bit field with the following values:
     *     - Bit 0: set if local modifications found.
     *     - Bit 1: set if mixed update revisions found.
     *     - Bit 2: set for a tagged and original release. */
    uint8_t release_type;
} revision_t;

/**
 * Author information of the firmware.
 */
typedef struct
{
    uint8_t author_cmd;
    uint16_t author_id;
    uint8_t variation;
} author_t;

/*! @} */

/**
 * \name LEDs
 */
/*! @{ */
/**
 * Type indicating which led should be affected by the command.
 * The left LED is affected to bit0 and the right LED is at bit1. This
 * simplifies comparisons. Assigning an hex value helps keep in mind the bit
 * relation.
 */
typedef enum
{
    LED_NONE = 0,
    LED_LEFT = 0x01,
    LED_RIGHT = 0x02,
    LED_BOTH = 0x03,
} leds_t;

/*! @} */

/**
 * \name Movements
 */
/*! @{ */
/**
 * Type indicating the mouth position.
 */
typedef enum
{
    MOUTH_UNKNOWN,
    MOUTH_OPEN,
    MOUTH_CLOSED,
} MOUTH_POSITION_t;

#define MOTORS_CMD_MASK 0x7F
#define MOTORS_VALUE_MASK 0x80

/**
 * Type indicating the motor
 */
typedef enum
{
    MOT_EYES,
    MOT_MOUTH,
    MOT_FLIPPERS,
    MOT_SPIN_L,
    MOT_SPIN_R,
} MOTOR_TYPE_t;

/**
 * Defines indicating the final position
 */
typedef enum
{
    FP_UNDEFINED = 0,
    FP_OPEN = 1,
    FP_UP = 1,
    FP_CLOSE = 2,
    FP_DOWN = 2,
    FP_STOP = 3,
} MOT_FINAL_POS_t;

/**
 * Type indicating the status
 */
typedef enum
{
    STAT_STOPPED = 0,
    STAT_OPEN = 1,
    STAT_UP = 1,
    STAT_CLOSE = 2,
    STAT_DOWN = 2,
    STAT_RUNNING = 3,
} MOT_STATUS_t;

/*! @} */

/**
 * \name Various specifications
 */
/*! @{ */
#define FW_MAIN_LOOP_DELAY 0.004
/*! @} */

/** \name RF SPI frame structure
 * @{ */

/* SPI frame structure */
#define SPI_SIZE 39
#define SPI_IDX_OFFSET 0
#define SPI_CONFIG_OFFSET 1
#define SPI_DATA_OFFSET 2
#define SPI_AUDIO_OFFSET (SPI_DATA_OFFSET + CMD_SIZE)

/** Size of the audio data in the SPI frame */
#define AUDIO_SPK_SIZE 33
#define AUDIO_MIC_SIZE 17

/* Bits of the config byte */
#define CFG_CRCOK_MK _BV(0)
#define CFG_DATA_MK _BV(1)
#define CFG_AUDIO_MK _BV(2)
#define CFG_SECBUF_MK _BV(3) /* Inidicates whether the second buffer (previous\
                                audio) is valid or not. */
#define CFG_ACK_MK _BV(4)
#define CFG_WAKEUP_MK _BV(5)

/*! @} */

/*! @} */

#endif /* _DEFINES_H_ */

