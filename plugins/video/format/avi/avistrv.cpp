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

#include "cssysdef.h"
#include "avistrv.h"
#include "iutil/plugin.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/txtmgr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <ctype.h>

SCF_IMPLEMENT_IBASE (csAVIStreamVideo)
  SCF_IMPLEMENTS_INTERFACE (iVideoStream)
  SCF_IMPLEMENTS_INTERFACE (iStream)
SCF_IMPLEMENT_IBASE_END

csAVIStreamVideo::csAVIStreamVideo (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  pChunk = 0;
  pAVI = (csAVIFormat*)pBase;
  object_reg = 0;
  pIA = new csImageArea (1,1,1,1);
  memimage.AttachNew (new csImageMemory (1, 1));
}

bool csAVIStreamVideo::Initialize (const csAVIFormat::AVIHeader *ph,
				   const csAVIFormat::StreamHeader *psh,
				   const csAVIFormat::VideoStreamFormat *pf,
				   uint16 nStreamNumber,
				   uint8 *pInitData, uint32 nInitDataLen,
				   char *pName,
				   uint8 *pFormatEx, uint32 nFormatEx,
				   iObjectRegistry *object_reg)
{
  strdesc.type = CS_STREAMTYPE_VIDEO;
  memcpy (strdesc.codec, psh->handler, 4);
  strdesc.colordepth = pf->bitcount;
  strdesc.framecount = ph->framecount;
  strdesc.width = ph->width;
  strdesc.height = ph->height;
  strdesc.framerate = psh->rate/psh->scale;
  strdesc.duration = psh->length / psh->scale;
  strdesc.name = pName;

  int i;
  for (i=3; i>=0 && strdesc.codec[i] == ' '; i--);
  strdesc.codec[i+1] = '\0';
  for (i=0; strdesc.codec[i]; i++)
    strdesc.codec[i] = tolower (strdesc.codec[i]);

  delete pChunk;
  pChunk = new csAVIFormat::AVIDataChunk;
  pChunk->currentframe = -1;
  pChunk->currentframepos = 0;
  sprintf (pChunk->id, "%02" PRIu16 "d%c", nStreamNumber, pf->compression == 0 ? 'b' : 'c');
  pChunk->id[4] = '\0';

  nStream = nStreamNumber;
  csAVIStreamVideo::object_reg = object_reg;
  pG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  pG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);

  pIA->w = 0;
  pIA->h = 0;
  pIA->x = 0;
  pIA->y = 0;
  delete pIA->data;
  pIA->data = 0;

  SetRect(0, 0, strdesc.width, strdesc.height);

  bTimeSynced = false;
  fxmode = CS_FX_COPY;
/*
  polyfx.num = 4;
  polyfx.use_fog = false;
  polyfx.texels[0].x = 0;
  polyfx.texels[0].y = 0;
  polyfx.texels[1].x = 1;
  polyfx.texels[1].y = 0;
  polyfx.texels[2].x = 1;
  polyfx.texels[2].y = 1;
  polyfx.texels[3].x = 0;
  polyfx.texels[3].y = 1;
  for (i=0; i<4; i++)
  {
    polyfx.colors[i].red = 1;
    polyfx.colors[i].green = 1;
    polyfx.colors[i].blue = 1;
    polyfx.z[i] = 1;
  }
*/
  pMaterial = 0;
  return LoadCodec (pInitData, nInitDataLen, pFormatEx, nFormatEx);
}

csAVIStreamVideo::~csAVIStreamVideo ()
{
  delete pChunk;
  delete pIA->data;
  delete pIA;
  SCF_DESTRUCT_IBASE();
}

void csAVIStreamVideo::GetStreamDescription (csStreamDescription &desc)
{
  memcpy (&desc, (csStreamDescription*)&strdesc, sizeof (csStreamDescription));
}

bool csAVIStreamVideo::GotoFrame (uint32 frameindex)
{
  return pAVI->GetChunk (frameindex, pChunk);
}

bool csAVIStreamVideo::GotoTime (uint32 timeindex)
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
//  int height = pG3D->GetHeight ()-1;

  rc.Set (x, y, x+w, y+h);
/*  polyfx.vertices[0].x = x;
  polyfx.vertices[0].y = height - y;
  polyfx.vertices[1].x = x+w;
  polyfx.vertices[1].y = height - y;
  polyfx.vertices[2].x = x+w;
  polyfx.vertices[2].y = height - (y+h);
  polyfx.vertices[3].x = x;
  polyfx.vertices[3].y = height - (y+h);*/

  memimage.AttachNew (new csImageMemory (rc.Width(), rc.Height()));
  return true;
}

bool csAVIStreamVideo::SetFXMode (uint mode)
{
  fxmode = mode;
  return true;
}

iMaterialHandle* csAVIStreamVideo::NextFrameGetMaterial ()
{
  if (NextFrameGetData ())
  {
    makeMaterial ();
    return pMaterial;
  }
  return 0;
}

void csAVIStreamVideo::PrepImageArea ()
{
  int nPB = pG2D->GetPixelBytes ();
  if (rc.Height () != pIA->h || rc.Width () != pIA->w)
  {
    delete pIA->data;
    pIA->data = new char [nPB * rc.Width () * rc.Height ()];
    pIA->x = rc.xmin;
    pIA->y = rc.ymin;
    pIA->w = rc.Width ();
    pIA->h = rc.Height ();
  }

  csRGBpixel *pixel = (csRGBpixel *)memimage->GetImageData ();
  char *p = pIA->data;

  int row, col;
  for (row=0; row<rc.Height (); row++)
    for (col=0; col<rc.Width (); col++)
    {
      int color = pG2D->FindRGB (pixel->red, pixel->green, pixel->blue);
      memcpy (p, ((char*)&color), nPB);
      p += nPB;
      pixel++;
    }
}

bool csAVIStreamVideo::NextFrameGetData ()
{
  void *outdata;

  if (pAVI->GetChunk (pChunk->currentframe+1, pChunk))
  {
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
    if (cdesc.decodeoutput == CS_CODECFORMAT_RGBA_INTERLEAVED)
      rgba_interleave ((char*)outdata);
    else
      return false;
    return true;

  }
  return false;
}

void csAVIStreamVideo::NextFrame ()
{
  if (NextFrameGetData ())
  {
    /*
    unsigned int zbuf = pG3D->GetRenderState( G3DRENDERSTATE_ZBUFFERMODE );
    bool btex = pG3D->GetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE ) != 0;
    bool bgour = pG3D->GetRenderState( G3DRENDERSTATE_GOURAUDENABLE ) != 0;

    pG3D->SetRenderState( G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE );
    pG3D->SetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE, true );
    pG3D->SetRenderState( G3DRENDERSTATE_GOURAUDENABLE, false );

    polyfx.mat_handle = pMaterial;
    polyfx.mixmode = fxmode;
    pG3D->DrawPolygonFX (polyfx);

    pG3D->SetRenderState( G3DRENDERSTATE_ZBUFFERMODE, zbuf );
    pG3D->SetRenderState( G3DRENDERSTATE_TEXTUREMAPPINGENABLE, btex );
    pG3D->SetRenderState( G3DRENDERSTATE_GOURAUDENABLE, bgour );
    */
    PrepImageArea ();
    pG2D->RestoreArea (pIA, false);
  }
}

//#define yuvmmx
#ifdef yuvmmx
extern "C"{
void yuv2rgba_mmx (unsigned char* ybuffer, unsigned char* ubuffer,
		   unsigned char* vbuffer, unsigned char* outbuffer,
		   int width, int height);
}
#endif
void csAVIStreamVideo::yuv_channel_2_rgba_interleave (char *data[3])
{
#ifdef yuvmmx
  unsigned char *ydata = (unsigned char *)data[0];
  unsigned char *udata = (unsigned char *)data[1];
  unsigned char *vdata = (unsigned char *)data[2];
  csRGBpixel *pixel = (csRGBpixel *)memimage.GetImageData ();
  yuv2rgba_mmx (ydata, udata, vdata, (unsigned char *)pixel,
		strdesc.width, strdesc.height);

#else
#define FIX_RANGE(x) (unsigned char)((x)>255.f ? 255 : (x) < 0.f ? 0 : (x))

  char *ydata = data[0];
  char *udata = data[1];
  char *vdata = data[2];
  int idx=0, uvidx=0, tidx=0;
  int sw = strdesc.width;
  int sh = strdesc.height;
  int halfsw = sw>>1;
  int tw = rc.Width ();
  int th = rc.Height ();
  int ytic=th, xtic;
  int ls = 0;
  float y,u,v, uf1 = 0, uf2 = 0, vf1 = 0, vf2 = 0;
  int sr=0, sc; // source row and col
  int row, col;

  csRGBpixel *pixel = (csRGBpixel *)memimage->GetImagePtr ();
  for (row=0; row < th; row++)
  {
    xtic = 0;
    sc = 0;
    for (col=0; col < tw; col++)
    {
      if (uvidx != (sc>>1)) // this YUV is a 1:4:4 scheme
      {
	uvidx = (sr>>1) * halfsw  + (sc>>1);
	u=((float)(unsigned char)udata[uvidx]) - 128.f;
	v=((float)(unsigned char)vdata[uvidx]) - 128.f;
	uf1 = 2.018f * u;
	uf2 = 0.813f * u;
	vf1 = 0.391f * v;
	vf2 = 1.596f * v;
	uvidx = sc  >>1;
      }
      y=1.164f*(((float)(unsigned char)ydata[idx]) - 16.f);
      pixel[tidx].red = FIX_RANGE(y + uf1);
      pixel[tidx].green= FIX_RANGE(y - uf2 - vf1);
      pixel[tidx].blue  = FIX_RANGE(y + vf2);

      while (xtic < sw)
      {
		idx++;
		sc++;
		xtic += tw;
      }
      xtic -=sw;
      tidx++;
    }
    while (ytic < sh)
    {
      ls += sw;
      sr++;
      ytic += th;
    }
    ytic -=sh;
    idx = ls;
  }

#undef FIX_RANGE
#endif
}

void csAVIStreamVideo::rgba_interleave (char *data)
{
  int idx=0, tidx=0;
  int sw = strdesc.width;
  int sh = strdesc.height;
  int tw = rc.Width ();
  int th = rc.Height ();
  int ytic=th, xtic;
  int ls = 0;
  int sr=0, sc; // source row and col
  int row, col;

  csRGBpixel *src = (csRGBpixel*)data;
  csRGBpixel *pixel = (csRGBpixel *)memimage->GetImageData ();
  for (row=0; row < th; row++)
  {
    xtic = 0;
    sc = 0;
    for (col=0; col < tw; col++)
    {
      memcpy (&pixel[tidx], &src[idx], sizeof (csRGBpixel));
      while (xtic < sw)
      {
		idx++;
		sc++;
		xtic += tw;
      }
      xtic -=sw;
      tidx++;
    }
    while (ytic < sh)
    {
      ls += sw;
      sr++;
      ytic += th;
    }
    ytic -=sh;
    idx = ls;
  }
}

void csAVIStreamVideo::rgb_channel_2_rgba_interleave (char *data[3])
{
  int idx, x, y;
  char *rdata = data[0];
  char *gdata = data[1];
  char *bdata = data[2];
  csRGBpixel *pixel = (csRGBpixel *)memimage->GetImagePtr ();
  for (idx=0, y=0; y < memimage->GetHeight (); y++)
    for (x=0; x < memimage->GetWidth (); x++)
    {
      pixel[idx].red   = rdata[idx];
      pixel[idx].green = gdata[idx];
      pixel[idx].blue  = bdata[idx];
      idx++;
    }
}

void csAVIStreamVideo::rgba_channel_2_rgba_interleave (char *data[4])
{
  int idx, x, y;
  char *rdata = data[0];
  char *gdata = data[1];
  char *bdata = data[2];
  char *adata = data[3];
  csRGBpixel *pixel = (csRGBpixel *)memimage->GetImagePtr ();
  for (idx=0, y=0; y < memimage->GetHeight (); y++)
    for (x=0; x < memimage->GetWidth (); x++)
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
  iTextureManager *txtmgr = pG3D->GetTextureManager();
  csRef<iTextureHandle> pFrameTex (
  	txtmgr->RegisterTexture (memimage, CS_TEXTURE_NOMIPMAPS));
  pMaterial = txtmgr->RegisterMaterial (pFrameTex);
}

bool csAVIStreamVideo::LoadCodec (uint8 *pInitData, uint32 nInitDataLen,
				  uint8 *pFormatEx, uint32 nFormatEx)
{
  // based on the codec id we try to load the apropriate codec

  // create a classname from the coec id
  char cn[128];
  sprintf (cn, "crystalspace.video.codec.avi.%s", strdesc.codec);
  // try open this class
  pCodec = SCF_CREATE_INSTANCE (cn, iAVICodec);
  if (pCodec)
  {
    // codec exists, now try to initialize it
    if (pCodec->Initialize (&strdesc, pInitData, nInitDataLen, pFormatEx, nFormatEx))
    {
      pCodec->GetCodecDescription (cdesc);
      return true;
    }
    else
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		"crystalspace.video.avi",
      		"CODEC class \"%s\" could not be initialized !", cn);
      pCodec = 0;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		"crystalspace.video.avi",
      		"CODEC class \"%s\" could not be loaded !", cn);
  }

  return false;
}
