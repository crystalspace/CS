/*
	Copyright (C) 2010 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef _CEGUIIMPORTS_H_H
#define _CEGUIIMPORTS_H_H

#include "cssysdef.h"
#include "csutil/custom_new_disable.h"
#if defined(CS_CEGUI_PATH)
#include <CoreFoundation/CFBundle.h>
#include CS_HEADER_GLOBAL(CS_CEGUI_PATH,CEGUI.h)
#else
#include <CEGUI.h>
#endif
#include "csutil/custom_new_enable.h"

#endif // _CEGUIIMPORTS_H_H
