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

#include "cssysdef.h"
#include "avistrv.h"

IMPLEMENT_IBASE (csAVIStreamVideo)
  IMPLEMENTS_INTERFACE (iVideoStream)
  IMPLEMENTS_INTERFACE (iStream)
IMPLEMENT_IBASE_END

csAVIStreamVideo::csAVIStreamVideo (iBase *pBase)
{
  CONSTRUCT_IBASE (pBase);
  pChunk = NULL;
  pAVI = NULL;
  pSystem = NULL;
  pG3D = NULL;
  pCodec = NULL;
  pMaterial = NULL;
}

bool csAVIStreamVideo::Initialize (const csVideoStreamDescription &desc, 
				   csAVIStreamFormat *pTheAVI, 
				   iSystem *pTheSystem)
{
  memcpy (strdesc, desc, sizeof (csVideoStreamDescription));
  delete pChunk;
  pChunk = new DataChunk;
  pAVI = pTheAVI;
  if (pSystem) pSystem->DecRef ();
  (pSystem = pTheSystem)->IncRef ();
  if (pG3D) pG3D->DecRef ();
  pG3D = QUERY_PLUGIN (pSystem, iGraphics3D);
  SetRect (0, 0, width, height);
  pos = 0;
  bTimeSynced = false;
  fxmode = CS_FX_COPY;
  if (pMaterial) pMaterial->DecRef ();
  pMaterial = NULL;
  // load the CODEC
  return LoadCodec ();
}

csAVIStreamVideo::~csAVIStreamVideo ()
{
  delete pChunk;
  if (pMaterial) pMaterial->DecRef ();
  if (pCodec) pCodec->DecRef ();
  if (pG3D) pG3D->DecRef ();
  if (pSystem) pSystem->DecRef ();
}

void csAVIStreamVideo::GetStreamDescription (csStreamDescription &desc)
{
  memcpy (&desc, (csStreamDescription*)&strdesc, sizeof (csStreamDescription));
}

bool csAVIStreamVideo::GotoFrame (ULong frameindex)
{
  if (pAVI->HasChunk (frameindex))
  {
    pos = frameindex;
    return true;
  }
  return false;
}

bool csAVIStreamVideo::GotoTime (ULong timeindex)
{
  (void)timeindex;
  // not yet implemented
  return false;
}

bool csAVIStreamVideo::SetPlayMethod (bool bTimeSynced)
{
  // timesynced isnt yet implemented, so return false if its requested.
  return bTimeSynced == false;
}

void csAVIStreamVideo::GetStreamDescription (csVideoStreamDescription &desc)
{
  memcpy (&desc, &strdesc, sizeof (csVideoStreamDescription));
}

bool csAVIStreamVideo::SetRect (int x, int y, int w, int h)
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

bool csAVIStreamVideo::SetFXMode (UInt mode)
{
  fxmode = mode;
  return true;
}

iMaterialHandle* csAVIStreamVideo::NextFrameGetMaterial ()
{
  pAVI->Chunk (pos++, pChunk);
  pCodec->Decode (pChunk->data, pChunk->length, outdata);
  if (pCodecDesc->decodeoutput == CS_CODECFORMAT_YUV_CHANNEL)
    yuv_channel_2_rgba_interleave (outdata);
  else
  if (pCodecDesc->decodeoutput == CS_CODECFORMAT_RGB_CHANNEL)
    rgb_channel_2_rgba_interleave (outdata);
  else
  if (pCodecDesc->decodeoutput == CS_CODECFORMAT_RGBA_CHANNEL)
    rgba_channel_2_rgba_interleave (outdata);
  else
  if (pCodecDesc->decodeoutput != CS_CODECFORMAT_RGBA_INTERLEAVE)
    return NULL;
  makeMaterial ();
  return pMaterial;
}

void csAVIStreamVideo::NextFrame ()
{
  NextFrameGetMaterial ();
  polyfx.mat_handle = pMaterial;
  pG3D->StartPolygonFX (polyfx.mat_handle, fxmode);
  pG3D->DrawPolygonFX (polyfx);
  pG3D->FinishPolygonFX ();
}

void csAVIStreamVideo::yuv_channel_2_rgba_interleave (char *data[3])
{
  char *ydata = data[0];
  char *udata = data[1];
  char *vdata = data[2];
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

void csAVIStreamVideo::rgb_channel_2_rgba_interleave (char *data[3])
{
  char *rdata = data[0];
  char *gdata = data[1];
  char *bdata = data[2];
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  for (int idx=0, y=0; y < h; y++)
    for (int x=0; x < w; x++)
    {
      pixel[idx].red   = rdata[idx];
      pixel[idx].green = gdata[idx];
      pixel[idx].blue  = bdata[idx];
      idx++;
    }
}

void csAVIStreamVideo::rgba_channel_2_rgba_interleave (char *data[4])
{
  char *rdata = data[0];
  char *gdata = data[1];
  char *bdata = data[2];
  char *adata = data[2];
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  for (int idx=0, y=0; y < h; y++)
    for (int x=0; x < w; x++)
    {
      pixel[idx].red   = rdata[idx];
      pixel[idx].green = gdata[idx];
      pixel[idx].blue  = bdata[idx];
      pixel[idx].alpha = adata[idx];
      idx++;
    }
}

void csAVIStreamVideo::makeMaterial ()
{
  if (pMaterial)
    pMaterial->DecRef ();

  iTextureManager *txtmgr = pG3D->GetTextureManager();
  iTextureHandle *pFrameTex = txtmgr->RegisterTexture (&memimage, CS_TEXTURE_NOMIPMAPS);
  pMaterial = txtmgr->RegisterMaterial (pFrameTex);
  pMaterial->Prepare ();
}
