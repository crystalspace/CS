/*
    Copyright (C) 1998 by Jorrit Tyberghein    

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

#ifndef __AIFFFILE_H
#define __AIFFFILE_H

#include "cssndldr/sndload.h"

///
class AIFFLoader : public csSoundBufferLoader  
{
protected:
  ///
  virtual csSoundBuffer* loadsound(UByte* buf, ULong size); 

public:
  ///
  virtual const char* get_name() const
  { return "AIFF"; }
  ///
  virtual const char* get_desc() const 
  { return "MacIntosh AIFF sound format"; }
};

#endif // __AIFFFILE_H
