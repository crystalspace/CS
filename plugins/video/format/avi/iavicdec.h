/*
    Copyright (C) 2001 by Norman Kraemer

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

#ifndef __CS_IAVICDEC_H__
#define __CS_IAVICDEC_H__

#include "csutil/scf.h"
#include "ivideo/codec.h"

SCF_VERSION (iAVICodec, 0, 0, 1);

struct iAVICodec : public iBase
{
  /**
   * Send either video or audio stream description as input. The codec
   * will cast it.
   */
  virtual bool Initialize (csStreamDescription *desc, uint8 *pInitData,
  	uint32 nInitDataLen, uint8 *pFormatEx, uint32 nFormatEx )=0;
  virtual void GetCodecDescription (csCodecDescription &desc) = 0;
  virtual bool Decode (char *indata, uint32 inlength, void *&outdata) = 0;
  virtual bool Encode (void *indata, char *outdata, uint32 &outlength) = 0;
};

#endif // __CS_IAVICDEC_H__
