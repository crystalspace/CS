/*
    Copyright (C) 2000 by Jorrit Tyberghein and K. Robert Bate.

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

/*----------------------------------------------------------------
	Written by K. Robert Bate 2000.
----------------------------------------------------------------*/

#define USE_OLD_INPUT_SPROCKET_LABELS 1
#include <InputSprocket.h>

ISpNeed gInputNeeds[] =
{
	{
		"\pAbort Game",
		256,
		0,		// player number
		0,		// group number
		kISpElementKind_Button,
		kISpElementLabel_Btn_PauseResume,
		0,		// flags
		0,		// reserved 1
		0,		// reserved 2
		0		// reserved 3
	},
	{
		"\pButton",
		257,
		0,		// player number
		0,		// group number
		kISpElementKind_Button,
		kISpElementLabel_Btn_Fire,
		0,		// flags
		0,		// reserved 1
		0,		// reserved 2
		0		// reserved 3
	},
	{
		"\pX Axis Movement",
		258,
		0,		// player number
		0,		// group number
		kISpElementKind_Axis,
		kISpElementLabel_Axis_XAxis,
		0,		// flags
		0,		// reserved 1
		0,		// reserved 2
		0		// reserved 3
	},
	{
		"\pY Axis Movement",
		259,
		0,		// player number
		0,		// group number
		kISpElementKind_Axis,
		kISpElementLabel_Axis_YAxis,
		0,		// flags
		0,		// reserved 1
		0,		// reserved 2
		0		// reserved 3
	}
};

int gNumInputNeeds = ( sizeof( gInputNeeds ) / sizeof( ISpNeed ));
ISpElementReference gInputElements[ sizeof( gInputNeeds ) / sizeof( ISpNeed ) ];
bool gInputUseKeyboard = false;
bool gInputUseMouse = false;
