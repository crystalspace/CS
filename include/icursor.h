/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Michael Dale Long
  
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

#ifndef __CS_ICURSOR_H__
#define __CS_ICURSOR_H__

#include "csutil/scf.h"

/**
 * This is the iCursor interface.  You can implement to provide customized
 * cursor abilities (animated cursors and so on).  iConsole uses it to allow
 * customized cursors for text input.  Using it for drawing customized mouse
 * and other types of cursors should also be possible.
 */
SCF_VERSION(iCursor, 0, 0, 1);
struct iCursor : public iBase
{
  /** 
   * Draw the cursor.  The area given by rect is the 2D space the cursor may
   * occupy on the screen.  Note that this doesn't necessarily restrict it to
   * a 2D cursor, but the 3D rendering must not be drawn outside of the given
   * area.
   */
  virtual void Draw(csRect &area) = 0;

};

#endif // ! __CS_ICURSOR_H__
