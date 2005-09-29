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

#ifndef __CS_AVI_AUDIOSTREAM_H__
#define __CS_AVI_AUDIOSTREAM_H__

#include "ivideo/codec.h"
#include "csavi.h"
#include "iavicdec.h"

class csAVIStreamAudio : public iAudioStream
{
 protected:
  csAVIFormat *pAVI;
  bool bTimeSynced;
  csAVIFormat::AVIDataChunk *pChunk;
  uint16 nStream;
  csAudioStreamDescription strdesc;

  iObjectRegistry *object_reg;
  csRef<iAVICodec> pCodec;

  bool LoadCodec (uint8 *pInitData, uint32 nInitDataLen, uint8 *pFormatEx,
  	uint32 nFormatEx);

 public:
  SCF_DECLARE_IBASE;

  csAVIStreamAudio (iBase *pBase);
  bool Initialize (const csAVIFormat::AVIHeader *ph,
		   const csAVIFormat::StreamHeader *psh,
		   const csAVIFormat::AudioStreamFormat *pf,
		   uint16 nStreamNumber,
		   uint8 *pInitData, uint32 nInitDataLen,
		   char *pName,
		   uint8 *pFormatEx, uint32 nFormatEx,
		   iObjectRegistry *object_reg);
  virtual ~csAVIStreamAudio ();

  // iStream
  virtual void GetStreamDescription (csStreamDescription &desc);
  virtual bool GotoFrame (uint32 frameindex);
  virtual bool GotoTime (uint32 timeindex);
  virtual bool SetPlayMethod (bool bTimeSynced);
  virtual void NextFrame ();
  // iAudioStream
  virtual void GetStreamDescription (csAudioStreamDescription &desc);
};

#endif // __CS_AVI_AUDIOSTREAM_H__
