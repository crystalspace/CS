/*
    Copyright (C) 2002 by Norman Kraemer
  
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

#ifndef __CS_ODIVX4_H__
#define __CS_ODIVX4_H__

#include "ivideo/codec.h"
#include "plugins/video/format/avi/iavicdec.h"

class csDivX4 : public iAVICodec
{
 protected:
  bool bOK;
  int w,h;
  char *result;

  DEC_SET dec_set;
  DEC_FRAME dec_frame;
  DEC_MEM_REQS decMemReqs;
  DEC_PARAM decParam;

 public:
  SCF_DECLARE_IBASE;
  csDivX4 (iBase *pParent);
  virtual ~csDivX4 ();

  virtual bool Initialize (csStreamDescription *desc, uint8 *, uint32,
  	uint8 *, uint32);
  virtual void GetCodecDescription (csCodecDescription &desc);
  virtual bool Decode (char *indata, uint32 inlength, void *&outdata);
  virtual bool Encode (void *indata, char *outdata, uint32 &outlength);
};

#endif // __CS_ODIVX4_H__
