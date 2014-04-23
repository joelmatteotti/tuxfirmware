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

/* $Id: version.h 5309 2009-08-14 02:41:48Z ks156 $ */

#ifndef VERSION_H
#define VERSION_H

#include "svnrev.h"

/*
 * Version number
 */

#define VER_MAJOR           0
#define VER_MINOR           9
#define VER_UPDATE          4
/** RELEASE should be set to '1' prior to tagging a release, and reset
 * immediately after. It's like appending (SVN_UNRELEASED) to a version number
 * when equal to '0'. */
#define RELEASE             0

#define AUTHOR_ID           0 /* official release */
#define VARIATION           0 /* generic firmware */

#define RELEASE_TYPE (SVN_STATUS | (RELEASE << 2))

#endif /* VERSION_H */
