/*
    Copyright (C) 1998 by Jorrit Tyberghein and K. Robert Bate.
  
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
	Written by K. Robert Bate 1998.
	
	Initial pass at this class.  Should eventually be modified to use
	DrawSprockets and InputSprockets.
----------------------------------------------------------------*/

#include "sysdef.h"
#include "MacMouse.h"

SysMouseDriver::SysMouseDriver()
		: csMouseDriver()
{
}

SysMouseDriver::~SysMouseDriver( void )
{
  Close();
}

bool SysMouseDriver::Open( iSystem* system, csEventQueue *EvQueue )
{
  csMouseDriver::Open ( system, EvQueue );
  return true;
}

void SysMouseDriver::Close( void )
{
}
