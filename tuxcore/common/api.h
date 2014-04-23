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

/* $Id: api.h 1236 2008-06-17 14:46:21Z jaguarondi $*/

/** \file api.h
    \brief Firmware API
    \ingroup api
*/

/** \defgroup api Firmware API

    These firmware commands constitute the program interface between all CPUs
    and the computer. Both actions (also called commands) and status are
    controlled by these commands.
 */
/*!  @{ */

#ifndef _API_H_
#define _API_H_

#include <stdint.h>

/*
 * COMMANDS
 *
 * Commands are divided in 4 groups depending on the number of parameters they
 * have:
 * - 0b00xxxxxx (0x00-0x3F) for void functions
 * - 0b01xxxxxx (0x40-0x7F) for functions requesting 1 parameter
 * - 0b10xxxxxx (0x80-0xBF) for functions requesting 2 parameters
 * - 0b11xxxxxx (0xC0-0xFF) for functions requesting 3 parameters
 *
 * The same definition of commands are used for all communications between the
 * computer and Tux. But depending on the communication way, the set of
 * commands will differ. There will be 3 sets based on Tux's architecture:
 * Tux's commands, audio commands, and status.
 *
 * 1. Tux's commands are sent from the computer to Tux in order to control all
 *    actuators: motors, LEDs, IR, etc. They can have any number of parameters
 *    (from 0 to 3) and are defined in "Tux's core API";
 *
 * 2. Audio commands are sent from the computer or from Tux to the audio chip.
 *    They never have 3 parameters;
 *
 * 3. Status are sent from Tux back to the computer in order to give feedback
 *    on sensor states or send some received IR code.
 *
 * Note: Audio commands are sent from the computer or from Tux to the audio
 * chip. The audio chip also receives status which it transfers to the
 * computer. In order to differentiate status from audio commands easily, we
 * chose to have audio commands <0xC0 which means they can have 0, 1 or 2
 * parameters but not 3.  Status commands will always have 3 parameters has we
 * always want to send as much information as possible anyway.
 *
 * \todo TODO this is still relevant but should be clarified, use this filed to
 * explain the api on a higher level. The command flow through all CPUs need to
 * be reviewed prior to update this.
 */

/** NULL command, does nothing and can be dropped anywhere in the communication
 * chain. */
#define NULL_CMD 0x00

/**
 * \name Request general information
 * These commands request some general information of the CPUs and their
 * firmware. This includes identifiers, versions, revisions, author, etc.
 *  @{ */

/** Request generic informations from tuxcore. */
#define INFO_TUXCORE_CMD 0x02
/** Request generic informations from tuxcaudio. */
#define INFO_TUXAUDIO_CMD 0x03
/** Request generic informations from tuxrf. */
#define INFO_TUXRF_CMD 0x04
/** Request generic informations from fuxrf. */
#define INFO_FUXRF_CMD 0x05
/** Request generic informations from fuxusb. */
#define INFO_FUXUSB_CMD 0x06

/*! @} */

/**
 * \name Return general information
 * These commands return some general information of the CPUs and their
 * firmware. This includes identifiers, versions, revisions, author, etc.
 *
 * Only the version command includes the CPU identifier. This command should
 * then be sent first so that the daemon knows which CPU is sending all their
 * information commands.
 *  @{ */
/**
 * \page versioning Versioning
 *
 * Tuxdroid is made of 5 CPUs having each their own firmware. Releasing a new
 * tuxdroid firmware only makes sense in releasing all 5 firmware as there's a
 * lot of interactions between them.
 *
 * We then have 5 firmware version numbers and added a 6th version number for
 * the package that will group all 5 firmware. This package will be called the
 * firmware release.
 *
 * In order to keep all numbers coherent, we decided to follow these rules when
 * releasing a new set of firmware:
 *   - all major and minor numbers should be equal;
 *   - the higher update number of all CPUs will be the update number of the
 *     release package;
 *   - when increasing the update number, use the highest update number among
 *     all CPUs + 1.
 */
/**
 * Return the CPU number and the firmware version in major.minor.update
 * format.
 *
 * In the current implementation, each firmware simply fetches the version_t
 * structure.
 *
 * Parameters:
 *   - 1 - 3 lower bits are the CPU number,
 *         5 higher bits are the major version number.
 *   - 2 - minor version number.
 *   - 3 - update version number.
 */
#define VERSION_CMD 0xC8

/**
 * Return the firmware revision number.
 *
 * In the current implementation, each firmware simply fetches the revision_t
 * structure.
 * As the AVR is little-endian, the 16 bits revision is sent with the LSB
 * first.
 *
 * Parameters:
 *   - 1 - LSB of the revision number.
 *   - 2 - MSB of the revision number.
 *   - 3 - Release type coded as a bit field with the following values:
 *     - Bit 0: set if local modifications found.
 *     - Bit 1: set if mixed update revisions found.
 *     - Bit 2: set for a tagged and original release.
 */
#define REVISION_CMD 0xC9

/**
 * Return the firmware author information.
 *
 * In the current implementation, each firmware simply fetches the author_t
 * structure.
 * As the AVR is little-endian, the 16 bits author number is sent with the LSB
 * first.
 *
 * The author id can be used to differentiate official firmware from other
 * custom versions.
 * The variation number can be used by one author to differentiate multiple
 * variations of a firmware. This is useful if someone develops a couple of
 * firmware that would require specific external sensors for example, or .
 * firmware that implements some functions differently like the flash usage, or
 * driving the motors another way.
 *
 * Official firmware from Kysoh has the author id '0'. If you're developing
 * firmware, use another number.
 *
 * Parameters:
 *   - 1 - LSB of the author id.
 *   - 2 - MSB of the author id.
 *   - 3 - variation number.
 */
#define AUTHOR_CMD 0xCA

/**
 * Return the sound flash information.
 *
 * Parameters:
 *   - 1 - number of sounds stored in the flash.
 */
#define SOUND_VAR_CMD 0xCB

/*! @} */

/**
 * \name LEDs
 * The blue LEDs located inside tux's eyes can be controlled individually by
 * changing their intensity from 0 (OFF) to 255 (ON).
 *  @{ */

/**
 * Set the speed and step which determine the speed of the fading effect.
 *
 * The fading increases or decreases the intensity by 'step' each 'speed' times
 * 4ms.
 *
 * Parameters:
 *   - 1 - LEDs affected by the command, either left, right or both.
 *         The values are defined in LEDS_t.
 *   - 2 - speed of the fading effect, the time base is 4ms.
 *         A value from 1 to 255 will change the speed,
 *         0 will leave it unaffected.
 *   - 3 - step of the fading effect.
 *         A value from 1 to 255 will change the step,
 *         0 will leave it unaffected.
 */
#define LED_FADE_SPEED_CMD 0xD0

/**
 * Set both LEDs to a given intensity with a fading effect controlled by
 * 'speed' and 'step'.
 *
 * Parameters:
 *   - 1 - Which LEDs.
 *   - 2 - Intensity that the LEDs should be set to. Fading may be applied if
 *   set.
 */
#define LED_SET_CMD 0xD1

/**
 * Set the intensity boundaries for the pulse command.
 *
 * Parameters:
 *   - 1 - Which LEDs.
 *   - 2 - Maximum intensity.
 *   - 3 - Minimum intensity.
 */
#define LED_PULSE_RANGE_CMD 0xD2

/**
 * Pulse LEDs 'number' times with a frequency determined by 'pulse_width'.
 *
 * Parameters:
 *   - 1 - Which LEDs
 *   - 2 - Number of toggles the LED should do.
 *   - 3 - Pulse width of the pulsing effect. 0 is ignored. The value from 1
 *   to 255 represents half the delay, in 4ms unit, between 2 pulses.
 *
 * The pulse width won't be shorter than what is set for the fading effect,
 * even if you set a low value here. If the pulse width is larger than the
 * fading period, then a pause will occur after fading in order to meet the
 * pulse width.
 */
#define LED_PULSE_CMD 0xD3

/*! @} */

/**
 * \name Status and return values
 * These commands are sent by tux to the computer instead of the opposite and
 * usually represent tux's status.
 *  @{ */

/**
 * LEDs status, both intensities and whether any effect is ongoing.
 *
 * Parameters:
 *   - 1 - Intensity of the left LED
 *   - 2 - Intensity of the right LED
 *   - 3 - Effects status
 *     - .0: Left LED fading
 *     - .1: Left LED pulsing
 *     - .3: Right LED fading
 *     - .4: Right LED pulsing
 *     - .5: LED mask, if set the LEDs are not lit even though the
 *           intensity is non zero.
 */
#define STATUS_LED_CMD 0xCE

/*! @} */

/** \name Movement commands
 *  Theses commands are used to move Tux.
 * @{ */

/**
 * Execute a defined number of movements, with a specified final state.
 *
 * Parameters:
 *    - 1 : The motor to command
 *    - 2 : The count or/ timeout value
 *    - 3 :
 *          .0..1 : The final state
 *          .2    : 0 for count, 1 for timeout
 */
#define MOTORS_SET_CMD  0xD4

/**
 * Execute a movement for a defined time and specify the PWM duty cycle.
 *
 * Parameters:
 *    - 1 : The motor to command
 *    - 2 : The PWM frequency
 */
#define MOTORS_CONFIG_CMD 0x81
/*! @} */


/*! @} */

/** States of the audio recording (flash programming) process */
typedef enum audiorec_status
{
    STANDBY = 0,
    IN_PROGRESS,
    WAITING_FOR_CONFIRMATION,
    WRITE_TOC,
    ERASING_LAST_SOUND,
    FLASH_FULL,
    NO_SOUND,
} audiorec_status_t;

#endif /* _API_H_ */

