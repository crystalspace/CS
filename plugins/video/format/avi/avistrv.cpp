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
#include "isystem.h"
#include "itexture.h"
#include "itxtmgr.h"

IMPLEMENT_IBASE (csAVIStreamVideo)
  IMPLEMENTS_INTERFACE (iVideoStream)
  IMPLEMENTS_INTERFACE (iStream)
IMPLEMENT_IBASE_END

csAVIStreamVideo::csAVIStreamVideo (iBase *pBase): memimage (1,1)
{
  CONSTRUCT_IBASE (pBase);
  pChunk = NULL;
  pAVI = (csAVIFormat*)pBase;
  pSystem = NULL;
  pG3D = NULL;
  pCodec = NULL;
  pMaterial = NULL;
}

bool csAVIStreamVideo::Initialize (const csAVIFormat::AVIHeader *ph, 
				   const csAVIFormat::StreamHeader *psh, 
				   const csAVIFormat::VideoStreamFormat *pf, 
				   UShort nStreamNumber, iSystem *pTheSystem)
{

  strdesc.type = CS_STREAMTYPE_VIDEO;
  memcpy (strdesc.codec, psh->handler, 4);
  strdesc.codec[4] = '\0';
  strdesc.colordepth = pf->planes * pf->bitcount;
  strdesc.framecount = ph->framecount;
  strdesc.width = ph->width;
  strdesc.height = ph->height;
  strdesc.framerate = 1000./ph->msecperframe;
  strdesc.duration = psh->length / psh->scale;

  delete pChunk;
  pChunk = new csAVIFormat::AVIDataChunk;
  pChunk->currentframe = 0;
  pChunk->currentframepos = NULL;
  sprintf (pChunk->id, "%02dd", nStreamNumber);
  pChunk->id[3] = '\0';

  nStream = nStreamNumber;
  if (pSystem) pSystem->DecRef ();
  (pSystem = pTheSystem)->IncRef ();
  if (pG3D) pG3D->DecRef ();
  pG3D = QUERY_PLUGIN (pSystem, iGraphics3D);
  SetRect (0, 0, strdesc.width, strdesc.height);

  bTimeSynced = false;
  fxmode = CS_FX_COPY;
  polyfx.num = 4;
  polyfx.use_fog = false;
  polyfx.vertices[0].u = 0;
  polyfx.vertices[0].v = 0;
  polyfx.vertices[1].u = 1;
  polyfx.vertices[1].v = 0;
  polyfx.vertices[2].u = 1;
  polyfx.vertices[2].v = 1;
  polyfx.vertices[3].u = 0;
  polyfx.vertices[3].v = 1;
  for (int i=0; i<4; i++)
  {
    polyfx.vertices[i].r = 1;
    polyfx.vertices[i].g = 1;
    polyfx.vertices[i].b = 1;
    polyfx.vertices[i].z = 1;
  }

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
  return pAVI->GetChunk (frameindex, pChunk);
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
  int height = pG3D->GetHeight ()-1;
  rc.Set (x, y, x+w, y+h);
  
  polyfx.vertices[0].sx = x;
  polyfx.vertices[0].sy = height - y;
  polyfx.vertices[1].sx = x+w;
  polyfx.vertices[1].sy = height - y;
  polyfx.vertices[2].sx = x+w;
  polyfx.vertices[2].sy = height - (y+h);
  polyfx.vertices[3].sx = x;
  polyfx.vertices[3].sy = height - (y+h);
  memimage.Rescale (w, h);
  return true;
}

bool csAVIStreamVideo::SetFXMode (UInt mode)
{
  fxmode = mode;
  return true;
}
#include "csgfxldr/pngsave.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
iMaterialHandle* csAVIStreamVideo::NextFrameGetMaterial ()
{
  void *outdata;

  if (pAVI->GetChunk (pChunk->currentframe, pChunk))
  {
    pChunk->currentframe++;
    pCodec->Decode ((char*)pChunk->data, pChunk->length, outdata);
    if (cdesc.decodeoutput == CS_CODECFORMAT_YUV_CHANNEL)
      //      rgb_channel_2_rgba_interleave ((char **)outdata);
      yuv_channel_2_rgba_interleave ((char **)outdata);
    else
    if (cdesc.decodeoutput == CS_CODECFORMAT_RGB_CHANNEL)
      rgb_channel_2_rgba_interleave ((char **)outdata);
    else
    if (cdesc.decodeoutput == CS_CODECFORMAT_RGBA_CHANNEL)
      rgba_channel_2_rgba_interleave ((char **)outdata);
    else
    if (cdesc.decodeoutput != CS_CODECFORMAT_RGBA_INTERLEAVED)
      return NULL;
    /*
    char name[20];
    int i=0;
  do
  {
    sprintf (name, "avi%06d.png", i++);
  } while (i < 100000 && (access (name, 0) == 0));

  if (i >= 1000)
    pSystem->Printf (MSG_CONSOLE, "Too many screenshot files in current directory\n");
  else
    if (!csSavePNG (name, &memimage))
      pSystem->Printf (MSG_CONSOLE, "There was an error while writing screen shot\n");
    */
    makeMaterial ();
    return pMaterial;
  }
  return NULL;
}

void csAVIStreamVideo::NextFrame ()
{
  if (NextFrameGetMaterial ())
  {
  unsigned int zbuf = pG3D->GetRenderState( G3DRENDERSTATE_ZBUFFERMODE );
  bool btex = pG3D->GetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE ) != 0;
  bool bgour = pG3D->GetRenderState( G3DRENDERSTATE_GOURAUDENABLE ) != 0;

  pG3D->SetRenderState( G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE );
  pG3D->SetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE, true );
  pG3D->SetRenderState( G3DRENDERSTATE_GOURAUDENABLE, false );

    polyfx.mat_handle = pMaterial;
    pG3D->StartPolygonFX (polyfx.mat_handle, fxmode);
    pG3D->DrawPolygonFX (polyfx);
    pG3D->FinishPolygonFX ();

  pG3D->SetRenderState( G3DRENDERSTATE_ZBUFFERMODE, zbuf );
  pG3D->SetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE, btex );
  pG3D->SetRenderState( G3DRENDERSTATE_GOURAUDENABLE, bgour );
  }
}

void csAVIStreamVideo::yuv_channel_2_rgba_interleave (char *data[3])
{
  char *ydata = data[0];
  char *udata = data[1];
  char *vdata = data[2];
  int idx=0;
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  for (int y=0; y < rc.Height (); y++)
    for (int x=0; x < rc.Width (); x++)
    {
      int y=MAX(0,(ydata[idx]) - 16);
      int u=MAX(0,(udata[idx]) - 128);
      int v=MAX(0,(vdata[idx]) - 128);
      pixel[idx].blue = (1.164*y + 2.018 * v);
      pixel[idx].green= (1.164*y - 0.813 * u - 0.391 * v);
      pixel[idx].red  = (1.164*y + 1.596 * u);
      idx++;
    }
  /*
  static int n=0;
  char tt[20];
  sprintf (tt,"pic%03d.Y", n);
  int f = creat ( tt, 0);
  write (f,ydata,idx);
  close (f);
  sprintf (tt,"pic%03d.U", n);
  f = creat ( tt, 0);
  write (f,udata,idx);
  close (f);
  sprintf (tt,"pic%03d.V", n);
  f = creat ( tt, 0);
  write (f,vdata,idx);
  close (f);
  n++;
  */
}

void csAVIStreamVideo::rgb_channel_2_rgba_interleave (char *data[3])
{
  char *rdata = data[0];
  char *gdata = data[1];
  char *bdata = data[2];
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  for (int idx=0, y=0; y < rc.Height (); y++)
    for (int x=0; x < rc.Width (); x++)
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
  for (int idx=0, y=0; y < rc.Height (); y++)
    for (int x=0; x < rc.Width (); x++)
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
  pFrameTex->Prepare ();
  pMaterial = txtmgr->RegisterMaterial (pFrameTex);
  pMaterial->Prepare ();
}

bool csAVIStreamVideo::LoadCodec ()
{
  // based on the codec id we try to load the apropriate codec
  iSCF *pSCF = QUERY_INTERFACE (pSystem, iSCF);
  if (pSCF)
  {
    // create a classname from the coec id
    char cn[128];
    sprintf (cn, "crystalspace.video.codec.%s", strdesc.codec);
    // try open this class
    pCodec = (iCodec*)pSCF->scfCreateInstance (cn, "iCodec", 0);
    pSCF->DecRef ();
    if (pCodec)
    {
      // codec exists, now try to initialize it
      if (pCodec->Initialize (&strdesc))
      {
	pCodec->GetCodecDescription (cdesc);
       	return true;
      }
      else
      {
	pSystem->Printf (MSG_WARNING, "CODEC class \"%s\" could not be initialized !", cn);
	pCodec->DecRef ();
	pCodec = NULL;
      }
    }
    else
      pSystem->Printf (MSG_WARNING, "CODEC class \"%s\" could not be loaded !", cn);
  }
  else
    pSystem->Printf (MSG_DEBUG_0, "Could not get an SCF interface from the systemdriver !");
  return false;
}
