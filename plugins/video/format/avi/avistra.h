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

#ifndef _AVI_AUDIOSTREAM_H_
#define _AVI_AUDIOSTREAM_H_

#include "ividecod.h"
#include "csavi.h"

class csAVIStreamAudio : public iAudioStream
{
 protected:
  csAVIFormat *pAVI;
  bool bTimeSynced;
  csAVIFormat::AVIDataChunk *pChunk;
  UShort nStream;
  csAudioStreamDescription strdesc;

  iSystem *pSystem;
  iCodec *pCodec;

  bool LoadCodec (UByte *pInitData, ULong nInitDataLen);

 public:
  DECLARE_IBASE;

  csAVIStreamAudio (iBase *pBase);
  bool Initialize (const csAVIFormat::AVIHeader *ph, 
		   const csAVIFormat::StreamHeader *psh, 
		   const csAVIFormat::AudioStreamFormat *pf, 
		   UShort nStreamNumber,
		   UByte *pInitData, ULong nInitDataLen,
		   char *pName, iSystem *pTheSystem);
  virtual ~csAVIStreamAudio ();

  // iStream
  virtual void GetStreamDescription (csStreamDescription &desc);
  virtual bool GotoFrame (ULong frameindex);
  virtual bool GotoTime (ULong timeindex);
  virtual bool SetPlayMethod (bool bTimeSynced);
  virtual void NextFrame ();
  // iAudioStream
  virtual void GetStreamDescription (csAudioStreamDescription &desc);
};

#endif
