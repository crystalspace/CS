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

#ifndef _AVI_VIDEOSTREAM_H_
#define _AVI_VIDEOSTREAM_H_

#include "ividecod.h"

class csAVIStreamVideo : public iVideoStream
{
 protected:
  csRect rc;
  UInt fxmode;
  bool bTimeSynced;
  G3DPolygonDPFX polyfx;
  csImageMemory memimage;
  DataChunk *pChunk;

  iMaterialHandle *pMaterial;
  iGraphics3D *pG3D;
  iSystem *pSystem;
  iVideoCodec *pCodec;

  void yuv_channel_2_rgba_interleave (char *data[3]);
  void rgb_channel_2_rgba_interleave (char *data[3]);
  void rgba_channel_2_rgba_interleave (char *data[4]);

 public:
  DECLARE_IBASE;

  csAVIStreamVideo (iBase *pBase);
  bool Initialize (const csVideoStreamDescription &desc, csAVIStreamFormat *pTheAVI, 
		   iSystem *pTheSystem);
  virtual ~csAVIStreamVideo ();

  // iStream
  virtual void GetStreamDescription (csStreamDescription &desc);
  virtual bool GotoFrame (ULong frameindex);
  virtual bool GotoTime (ULong timeindex);
  virtual bool SetPlayMethod (bool bTimeSynced);
  virtual void NextFrame ();
  // iVideoStream
  virtual void GetStreamDescription (csVideoStreamDescription &desc);
  virtual bool SetRect (int x, int y, int w, int h);
  virtual bool SetFXMode (UInt mode);
  virtual iMaterialHandle* NextFrameGetMaterial ();
};

#endif
