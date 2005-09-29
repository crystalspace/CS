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

#ifndef __CS_AVI_VIDEOSTREAM_H__
#define __CS_AVI_VIDEOSTREAM_H__

#include "ivideo/codec.h"
#include "csavi.h"
#include "csgeom/csrect.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "csgfx/memimage.h"
#include "iavicdec.h"

class csAVIStreamVideo : public iVideoStream
{
 protected:
  csRect rc;
  uint fxmode;
  bool bTimeSynced;
  csRef<csImageMemory> memimage;
  csAVIFormat::AVIDataChunk *pChunk;
  uint16 nStream;
  csVideoStreamDescription strdesc;
  csAVIFormat *pAVI;
  csImageArea *pIA;

  csRef<iTextureHandle> pTexture;
  csRef<iGraphics3D> pG3D;
  csRef<iGraphics2D> pG2D;
  iObjectRegistry *object_reg;
  csRef<iAVICodec> pCodec;
  csCodecDescription cdesc;

  void yuv_channel_2_rgba_interleave (char *data[3]);
  void rgb_channel_2_rgba_interleave (char *data[3]);
  void rgba_channel_2_rgba_interleave (char *data[4]);
  void rgba_interleave (char *data);

  bool LoadCodec (uint8 *pInitData, uint32 nInitDataLen, uint8 *pFormatEx,
  	uint32 nFormatEx);
  void makeTexture ();
  bool NextFrameGetData ();
  void PrepImageArea ();

 public:
  SCF_DECLARE_IBASE;

  csAVIStreamVideo (iBase *pBase);
  bool Initialize (const csAVIFormat::AVIHeader *ph,
		   const csAVIFormat::StreamHeader *psh,
		   const csAVIFormat::VideoStreamFormat *pf,
		   uint16 nStreamNumber,
		   uint8 *pInitData, uint32 nInitDataLen,
		   char *pName,
		   uint8 *pFormatEx, uint32 nFormatEx,
		   iObjectRegistry *object_reg);
  virtual ~csAVIStreamVideo ();

  // iStream
  virtual void GetStreamDescription (csStreamDescription &desc);
  virtual bool GotoFrame (uint32 frameindex);
  virtual bool GotoTime (uint32 timeindex);
  virtual bool SetPlayMethod (bool bTimeSynced);
  virtual void NextFrame ();
  // iVideoStream
  virtual void GetStreamDescription (csVideoStreamDescription &desc);
  virtual bool SetRect (int x, int y, int w, int h);
  virtual bool SetFXMode (uint mode);
  virtual iTextureHandle* NextFrameGetTexture ();
};

#endif // __CS_AVI_VIDEOSTREAM_H__
