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
#include "imater.h"
#include "itxtmgr.h"
#include "csgfxldr/rgbpixel.h"

IMPLEMENT_IBASE (csOpenDivX)
  IMPLEMENTS_INTERFACE (iVideoDecoder)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csOpenDivX)
EXPORT_CLASS_TABLE (csodivx)
  EXPORT_CLASS (csOpenDivX, "crystalspace.video.decoder.opendivx", 
		"CrystalSpace OpenDivX decoder interface")
EXPORT_CLASS_TABLE_END

csOpenDivX::csOpenDivX (iBase *pParent): memimage(1,1)
{
  CONSTRUCT_IBASE (pParent);
}

csOpenDivX::~csOpenDivX ()
{
  if (bOK && pFile)
  {
    // stop current processing
    decore ( (unsigned long)this, DEC_OPT_RELEASE, NULL, NULL);
    pFile->DecRef ();
    delete [] pData;
    delete [] ydata;
    delete [] udata;
    delete [] vdata;
  }

  if (pMaterial) pMaterial->DecRef ();
  if (pG3D) pG3D->DecRef ();
  if (pSystem) pSystem->DecRef ();
}

bool csOpenDivX::Initialize (iSystem *iSys)
{
  rc.Set (0, 0, 0, 0);
  w = h = 0;
  fxmode = CS_FX_COPY;
  bTimeSynced = false;
  bOK = false;
  datalen = 0;

  pG3D = QUERY_PLUGIN (iSys, iGraphics3D);
  pSystem = iSys;
  pSystem->IncRef ();
  pMaterial = NULL;
  pFile = NULL;
  pData = udata = vdata = NULL;

  polyfx.num = 4;
  polyfx.use_fog = false;
  for (int i=0; i<4; i++) 
  {
    polyfx.vertices[i].r=1;
    polyfx.vertices[i].g=1;
    polyfx.vertices[i].b=1;
    polyfx.vertices[i].z=1;
  }

  return pG3D != NULL;
}

void csOpenDivX::GetDecoderCaps (csVideoDecoderCap &caps)
{
  caps = (csVideoDecoderCap)0; // currently we can only play one after another, no positioning or whatsoever
}

bool csOpenDivX::Load (iFile *pVideoData)
{
  bOK = false;
  if (pFile)
  {
    // stop current processing
    decore ( (unsigned long)this, DEC_OPT_RELEASE, NULL, NULL);
    pFile->DecRef ();
    if (pData) 
      delete [] pData;
    pData = NULL;
    pFile = NULL;
  }
  
  (pFile = pVideoData)->IncRef ();
  size_t r=0;
  datalen = pFile->GetSize ();
  pData = new char[datalen];
  if (pData)
    r = pFile->Read (pData, datalen);

  if (datalen==r)
  {
    bool succ;
    dec_param.x_dim = w;
    dec_param.y_dim = h;
    succ = (decore ((unsigned long)this, DEC_OPT_INIT, &dec_param, NULL) == DEC_OK);
    if (succ)
    {
      dec_set.postproc_level = 0; // no pp
      bOK = (decore ((unsigned long)this, DEC_OPT_SETPP, &dec_set, NULL) == DEC_OK);
      return bOK;
    }
  }
  return false;
}

bool csOpenDivX::GotoFrame (ULong frameindex)
{
  (void)frameindex;
  return false;
}

bool csOpenDivX::GotoTime (ULong timeindex)
{
  (void)timeindex;
  return false;
}

bool csOpenDivX::SetPlayMethod (bool bTimeSynced)
{
  (void)bTimeSynced;
  return bTimeSynced == true;
}

bool csOpenDivX::SetRect (int x, int y, int w, int h)
{
  rc.Set (x, y, x+w, y+h);
  polyfx.vertices[0].sx = x;
  polyfx.vertices[0].sy = y;
  polyfx.vertices[1].sx = x+w;
  polyfx.vertices[1].sy = y;
  polyfx.vertices[2].sx = x+w;
  polyfx.vertices[2].sy = y+h;
  polyfx.vertices[3].sx = x;
  polyfx.vertices[3].sy = y+h;
  return true;
}

bool csOpenDivX::SetSourceSize (int w, int h)
{
  this->w = w;
  this->h = h;
  delete [] ydata;
  delete [] udata;
  delete [] vdata;
  ydata = new char[w*h];
  udata = new char[w*h];
  vdata = new char[w*h];
  memimage.Rescale (w, h);
  yuvdata[0] = ydata;
  yuvdata[1] = udata;
  yuvdata[2] = vdata;
  return true;
}

bool csOpenDivX::SetFXMode (UInt mode)
{
  fxmode = mode;
  return true;
}

void csOpenDivX::NextFrame ()
{
  if (bOK)
  {
    NextFrameGetMaterial ();
    polyfx.mat_handle = pMaterial;
    pG3D->StartPolygonFX (polyfx.mat_handle, fxmode);
    pG3D->DrawPolygonFX (polyfx);
    pG3D->FinishPolygonFX ();
  }
}

iMaterialHandle *csOpenDivX::NextFrameGetMaterial ()
{
  if (bOK)
  {
    dec_frame.length = datalen;
    dec_frame.bitstream = pData;
    dec_frame.bmp = yuvdata;
    dec_frame.render_flag = 1;
    decore ((unsigned long)this, 0, &dec_frame, NULL);

    yuv2rgb ();
    makeMaterial ();
    return pMaterial;
  }
  return NULL;
}

void csOpenDivX::yuv2rgb ()
{
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  for (int idx=0, y=0; y < h; y++)
    for (int x=0; x < w; x++)
    {
      pixel[idx].red = (unsigned char)(ydata[idx] + (1.4075 * ((int)vdata[idx] - 128)));
      pixel[idx].green = (unsigned char)(ydata[idx] - (0.3455 * ((int)udata[idx] - 128) 
				       - 0.7169 * ((int)vdata[idx] - 128)));
      pixel[idx].blue = (unsigned char)(ydata[idx] + (1.7790 * ((int)udata[idx] - 128)));
      idx++;
    }
}

void csOpenDivX::makeMaterial ()
{
  if (pMaterial)
    pMaterial->DecRef ();

  iTextureManager *txtmgr = pG3D->GetTextureManager();
  iTextureHandle *pFrameTex = txtmgr->RegisterTexture (&memimage, CS_TEXTURE_NOMIPMAPS);
  pMaterial = txtmgr->RegisterMaterial (pFrameTex);
  pMaterial->Prepare ();
}
