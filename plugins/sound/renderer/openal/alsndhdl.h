/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __SNDHDL_H__
#define __SNDHDL_H__

#include "../common/shdl.h"

class csSoundRenderOpenAL;
class csSoundHandleOpenAL : public csSoundHandle
{
  csSoundRenderOpenAL *parent;
  ALuint buffer;

public:
  // constructor
  csSoundHandleOpenAL(csSoundRenderOpenAL *srdr, iSoundData *snd);
  // destructor
  ~csSoundHandleOpenAL();

  csPtr<iSoundSource> CreateSource(int m);
  void vUpdate (void *buf, long samples);

  ALuint GetID () { return buffer; }
};

#endif // __SNDHDL_H__
