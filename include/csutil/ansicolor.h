/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_ANSICOLOR_H__
#define __CS_CSUTIL_ANSICOLOR_H__

/**\file
 * ANSI codes for colors. Can be passed to csPrintf() for colored text output.
 */
 
/// Reset custom set colors
#define	CS_ANSI_RST		"\033[0m"

/**\name Foreground colors
 * @{ */
/// Black
#define	CS_ANSI_FK		"\033[30m"
/// Red
#define	CS_ANSI_FR		"\033[31m"
/// Green
#define	CS_ANSI_FG		"\033[32m"
/// Yellow
#define	CS_ANSI_FY		"\033[33m"
/// Blue
#define	CS_ANSI_FB		"\033[34m"
/// Magenta
#define	CS_ANSI_FM		"\033[35m"
/// Cyan
#define	CS_ANSI_FC		"\033[36m"
/// White
#define	CS_ANSI_FW		"\033[36m"
/** @} */

/// Bold or intense foreground text on
#define	CS_ANSI_FI		"\033[1m"
/// Bold or intense foreground text off
#define	CS_ANSI_FI_OFF		"\033[22m"

/**\name Background colors
 * @{ */
/// Black
#define	CS_ANSI_BK		"\033[40m"
/// Red
#define	CS_ANSI_BR		"\033[41m"
/// Green
#define	CS_ANSI_BG		"\033[42m"
/// Yellow
#define	CS_ANSI_BY		"\033[43m"
/// Blue
#define	CS_ANSI_BB		"\033[44m"
/// Magenta
#define	CS_ANSI_BM		"\033[45m"
/// Cyan
#define	CS_ANSI_BC		"\033[46m"
/// White
#define	CS_ANSI_BW		"\033[46m"
/** @} */

#endif // __CS_CSUTIL_ANSICOLOR_H__
