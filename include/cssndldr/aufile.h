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

#ifndef __AUFILE_H
#define __AUFILE_H

#include "cssndldr/sndload.h"

///
class AULoader : public csSoundBufferLoader  
{
protected:
  ///
  virtual csSoundBuffer* loadsound(UByte* buf, ULong size);

public:
  ///
  virtual const char* get_name() const
  { return "AU"; }
  ///
  virtual const char* get_desc() const 
  { return "Sun sound format"; }
};

#endif // __AUFILE_H
