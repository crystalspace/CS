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

#ifndef _ODIVX_H_
#define _ODIVX_H_

#include "ividecod.h"
#include "igraph3d.h"
#include "ivfs.h"
#include "csutil/csrect.h"
#include "csgfxldr/memimage.h"

struct iMaterialHandle;
struct iSystem;

class csOpenDivX : public iVideoDecoder
{
 protected:
  csRect rc;
  int w,h;
  UInt fxmode;
  bool bTimeSynced, bOK;
  size_t datalen;

  iGraphics3D *pG3D;
  iSystem *pSystem;
  iFile *pFile;
  char *pData;
  char *ydata, *udata, *vdata;
  char *yuvdata[3];

  iMaterialHandle *pMaterial;
  G3DPolygonDPFX polyfx;
  csImageMemory memimage;

  DEC_PARAM dec_param;
  DEC_SET dec_set;
  DEC_FRAME dec_frame;

 protected:
  void yuv2rgb ();
  void makeMaterial ();

 public:
  DECLARE_IBASE;
  csOpenDivX (iBase *pParent);
  virtual ~csOpenDivX ();

  virtual bool Initialize (iSystem *iSys);

  virtual void GetDecoderCaps (csVideoDecoderCap &caps);
  virtual bool GotoFrame (ULong frameindex);
  virtual bool GotoTime (ULong timeindex);
  virtual bool SetPlayMethod (bool bTimeSynced);
  virtual bool SetRect (int x, int y, int w, int h);
  virtual bool SetSourceSize (int w, int h);
  virtual bool SetFXMode (UInt mode);
  virtual void NextFrame ();
  virtual iMaterialHandle* NextFrameGetMaterial ();
  virtual bool Load (iFile *pVideoData);
};
#endif
