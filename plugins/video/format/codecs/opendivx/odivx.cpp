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

#include <divx/decore.h>
#include "cssysdef.h"
#include "odivx.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csOpenDivX)
  SCF_IMPLEMENTS_INTERFACE (iAVICodec)
  SCF_IMPLEMENTS_INTERFACE (iBase)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csOpenDivX)
SCF_EXPORT_CLASS_TABLE (odivx)
  SCF_EXPORT_CLASS (csOpenDivX, "crystalspace.video.codec.avi.dvx1", "CrystalSpace OpenDivX codec")
  SCF_EXPORT_CLASS (csOpenDivX, "crystalspace.video.codec.avi.divx", "CrystalSpace OpenDivX codec")
SCF_EXPORT_CLASS_TABLE_END

csOpenDivX::csOpenDivX (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  ydata = udata = vdata = NULL;
}

csOpenDivX::~csOpenDivX ()
{
  if (bOK)
    // stop current processing
    decore ( (unsigned long)this, DEC_OPT_RELEASE, NULL, NULL);

  delete [] ydata;
  delete [] udata;
  delete [] vdata;
}

bool csOpenDivX::Initialize (csStreamDescription *desc, uint8 *, uint32, uint8 *, uint32)
{
  csVideoStreamDescription *vd = (csVideoStreamDescription *)desc;
  w = vd->width;
  h = vd->height;
  bOK = false;

  ydata = new char[w*h];
  udata = new char[w*h];
  vdata = new char[w*h];
  yuvdata[0] = ydata;
  yuvdata[1] = udata;
  yuvdata[2] = vdata;

  bool succ;
  dec_param.x_dim = w;
  dec_param.y_dim = h;
  dec_param.color_depth = 32;
  succ = (decore ((unsigned long)this, DEC_OPT_INIT, &dec_param, NULL) == DEC_OK);

  if (succ)
  {
    dec_set.postproc_level = 0; // no pp
    bOK = (decore ((unsigned long)this, DEC_OPT_SETPP, &dec_set, NULL) == DEC_OK);
  }
  return bOK;
}

void csOpenDivX::GetCodecDescription (csCodecDescription &desc)
{
  desc.bEncode = false; // not implemented yet
  desc.bDecode = true;
  desc.decodeoutput = CS_CODECFORMAT_YUV_CHANNEL;
  desc.encodeinput = CS_CODECFORMAT_YUV_CHANNEL;
}

bool csOpenDivX::Decode (char *indata, uint32 inlength, void *&outdata)
{
  if (bOK)
  {
    dec_frame.length = inlength;
    dec_frame.bitstream = indata;
    dec_frame.bmp = yuvdata;
    dec_frame.render_flag = 1;
    decore ((unsigned long)this, 0, &dec_frame, NULL);
    outdata = yuvdata;

    return true;
  }
  return false;
}

bool csOpenDivX::Encode (void *indata, char *outdata, uint32 &outlength)
{
  (void)indata;
  (void)outdata;
  (void)outlength;
  return false;
}
