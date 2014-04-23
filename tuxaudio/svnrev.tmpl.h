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

/* $Id: svnrev.tmpl.h 676 2007-11-09 15:39:21Z jaguarondi $ */

/* This file is a template to generate svnrev.h automatically.
 * SubWCRev performs keyword susbstitution whith SVN information.
 */

#ifndef _SVNREH_H_
#define _SVNREV_H_

#define SVN_REV     $WCREV$
#define SVN_REVSTR  "$WCREV$"
#define SVN_REVDATE "$WCDATE$"
#define SVN_MOD     $WCMODS?1:0$
#define SVN_MIX     $WCMIXED?1:0$
#define SVN_STATUS  (SVN_MOD + (SVN_MIX<<1))
#define SVN_RANGE   "$WCRANGE$"
#define SVN_URL     "$WCURL$"

#endif /* _SVNREV_H_ */
