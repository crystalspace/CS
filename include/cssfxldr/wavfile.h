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

#ifndef __WAVFILE_H
#define __WAVFILE_H

#include "cssfxldr/sndload.h"

///
class WAVLoader : public csSoundLoader
{
protected:
  ///
  virtual csSoundData* loadsound(UByte* buf, ULong size);

public:
  ///
  virtual const char* get_name() const
  { return "WAV"; }
  ///
  virtual const char* get_desc() const 
  { return "Microsoft WAV sound format"; }
};

#endif // __WAVFILE_H
