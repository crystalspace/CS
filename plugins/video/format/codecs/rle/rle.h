/*
    Copyright (C) 2001 by Norman Krämer

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

#ifndef _CSRLECODEC_H_
#define _CSRLECODEC_H_

#include "ivideo/codec.h"
#include "plugins/video/format/avi/iavicdec.h"
#include "csgfx/rgbpixel.h"

class csRLECodec : public iAVICodec
{
 protected:
  csRGBcolor cmap[256];
  csRGBpixel *pixel;
  int w,h;
  bool bOK;
  void (*decode)(uint8 *dst, uint8 *src, uint32 inlen, csRGBcolor *pMap, int w, int h);

 public:
  SCF_DECLARE_IBASE;
  csRLECodec (iBase *pParent);
  virtual ~csRLECodec ();

  virtual bool Initialize (csStreamDescription *desc, uint8 *, uint32, uint8 *, uint32);
  virtual void GetCodecDescription (csCodecDescription &desc);
  virtual bool Decode (char *indata, uint32 inlength, void *&outdata);
  virtual bool Encode (void *indata, char *outdata, uint32 &outlength);
};
#endif
