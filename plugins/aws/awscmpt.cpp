/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson
              (C) 2001 F.Richter	
    
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
*****************************************************************************/

#include "cssysdef.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "aws.h"
#include "iaws/awscnvs.h"
#include "ivideo/txtmgr.h"
#include "cstool/proctex.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "awscmpt.h"
#include <math.h>
#include <stdio.h>

SCF_IMPLEMENT_IBASE (awsMultiProctexCanvas)
SCF_IMPLEMENTS_INTERFACE (iAwsCanvas)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsMultiProctexCanvas::awscG2D)
SCF_IMPLEMENTS_INTERFACE (iGraphics2D)
SCF_IMPLEMENT_IBASE_END

awsMultiProctexCanvas::awscG2D::awscG2D(awsMultiProctexCanvas *parent, iGraphics2D *aG2D) 
{
  SCF_CONSTRUCT_IBASE (NULL);

  rG2D = aG2D;
  awsc = parent;

  FontServer = rG2D->GetFontServer();
}


bool awsMultiProctexCanvas::awscG2D::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  // Get the system parameters
  Width = awsc->CanvasWidth;
  Height = awsc->CanvasHeight;

  // Get the font server: A missing font server is NOT an error
  if (!FontServer)
  {
    FontServer = CS_QUERY_REGISTRY (object_reg, iFontServer);
  }

  //pfmt = *rG2D->GetPixelFormat ();
  //Palette = rG2D->GetPalette ();

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
            "crystalspace.graphics2d.awsMultiProctexCanvas::awscG2D", "awsMultiProctexCanvas::awscG2D mode %dx%d.",
            Width, Height);

  return true;
}

bool awsMultiProctexCanvas::awscG2D::Open ()
{
  SetClipRect (0, 0, Width - 1, Height - 1);
  return true;
}

void awsMultiProctexCanvas::awscG2D::Close ()
{
}

void awsMultiProctexCanvas::awscG2D::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  ClipX1 = xmin; ClipX2 = xmax;
  ClipY1 = ymin; ClipY2 = ymax;

  int i, count=awsc->GetFlatCanvasCount();
  for (i=0; i<count; ++i)
  {
    csRect *canvasRect = awsc->GetFlatCanvasRect(i);
    csRect rect ((csRect&)*canvasRect);

    rect.Intersect(ClipX1, ClipY1, ClipX2, ClipY2);

    awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
    canvas->G2D()->SetClipRect(0, 0, rect.Width(), rect.Height());
  }
}

void awsMultiProctexCanvas::awscG2D::DrawLine (
                                              float x1, float y1, float x2, float y2, int color)
{
    int i,count=awsc->GetFlatCanvasCount();

    if (x1>x2)
    {
      float t=x2;
      x2=x1;
      x1=t;
    }

    if (y1>y2)
    {
      float t=y2;
      y2=y1;
      y1=t;
    }

    for (i=0; i<count; ++i)
    {
      csRect crect(*awsc->GetFlatCanvasRect(i));
      csRect rect((int) x1-1,(int) y1-1,(int) x2+1,(int) y2+1);
      
      if (rect.Intersects (crect))
      {
        awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
	if (((y1-crect.ymin) < crect.Height()) || ((y2-crect.ymin) < crect.Height())) {
	  // hack to avoid crash due to the behaviour of DrawLine():
	  // DrawLine() expects the clipping rect to include bottom
	  // and right edges BUT it never draws the last pixel of a line.
	  // so we cannot draw correct lines across proctexes without either
	  //  a) setting the last pixel by hand if needed
	  //  b) using a wrong clipping rect
	  // we do (b) at the moment, so we don't draw lines with both y ==
	  // Height() (otherwise it would crash because that line is obviously 
	  // invalid
          canvas->G2D()->DrawLine((float)x1-crect.xmin, (float)y1-crect.ymin, 
                                  (float)x2-crect.xmin, (float)y2-crect.ymin, color);
	}
      }
    }
}

void awsMultiProctexCanvas::awscG2D::DrawBox (int x, int y, int w, int h, int color)
{ 
  int i, count=awsc->GetFlatCanvasCount ();
  csRect boxrect (x, y, x+w, y+h);
  
  for (i=0; i<count; ++i)
  {
      csRect *canvasRect = awsc->GetFlatCanvasRect(i);
      csRect rect (*canvasRect);
      rect.Intersect(boxrect);
      if (!rect.IsEmpty())
      {
        awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
        canvas->G2D()->DrawBox(rect.xmin-canvasRect->xmin, rect.ymin-canvasRect->ymin, rect.Width(), rect.Height(), color);
      }
   }
}

void awsMultiProctexCanvas::awscG2D::DrawPixel (int x, int y, int color)
{
  if ((x >= ClipX1) && (y < ClipX2) && (y >= ClipY1) && (y < ClipY2))
  {
    int i, count=awsc->GetFlatCanvasCount();

    for (i=0; i<count; ++i)
    {
        csRect *canvasRect = awsc->GetFlatCanvasRect(i);

        if (canvasRect->Contains(x, y))
        {
          awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
          canvas->G2D()->DrawPixel(x-canvasRect->xmin, y-canvasRect->ymin, color);
        }
    }
  }
}

void awsMultiProctexCanvas::awscG2D::Write (iFont *font, int x, int y, int fg, int bg,
                                            const char *text)
{
  if (!font)
    return;

  int tW, tH;

  font->GetDimensions(text, tW, tH);

  csRect clipRect (ClipX1, ClipY1, ClipX2+1, ClipY2+1);
  csRect textRect (x, y, x+tW+1, y+tH+1);

  if (!textRect.Intersects(clipRect))
    return;

  int i,count=awsc->GetFlatCanvasCount();
  
  for (i=0; i<count; ++i)
  {
      csRect *canvasRect = awsc->GetFlatCanvasRect(i);

      if (canvasRect->Intersects(textRect))
      {
        awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
        canvas->G2D()->Write(font, x-canvasRect->xmin, y-canvasRect->ymin, fg, bg, text);
      }
   }
}

unsigned char* awsMultiProctexCanvas::awscG2D::GetPixelAt (int x, int y)
{
  int i,count=awsc->GetFlatCanvasCount();
  
  for (i=0; i<count; ++i)
  {
      csRect *canvasRect = awsc->GetFlatCanvasRect(i);

      if (canvasRect->Contains(x, y))
      {
        awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
        return canvas->G2D()->GetPixelAt(x-canvasRect->xmin, y-canvasRect->ymin);
      }
   }
  return NULL;
}

void awsMultiProctexCanvas::awscG2D::GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB)
{
  oR = oG = oB = 0;
  int i,count=awsc->GetFlatCanvasCount();
  
  for (i=0; i<count; ++i)
  {
      csRect *canvasRect = awsc->GetFlatCanvasRect(i);

      if (canvasRect->Contains(x, y))
      {
        awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
        canvas->G2D()->GetPixel(x-canvasRect->xmin, y-canvasRect->ymin, oR, oG, oB);
	return;
      }
   }
}

bool awsMultiProctexCanvas::awscG2D::BeginDraw ()
{
  if (!FrameBufferLocked)
  {
    int i, count=awsc->GetFlatCanvasCount();

    for (i=0; i<count; ++i)
    {
      awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);

      canvas->G2D()->BeginDraw();
    }
  }
  FrameBufferLocked++;
  return true;
}

void awsMultiProctexCanvas::awscG2D::FinishDraw ()
{
  if (FrameBufferLocked) 
  {
    FrameBufferLocked--;
    if (!FrameBufferLocked) 
    {
      int i, count=awsc->GetFlatCanvasCount();

      for (i=0; i<count; ++i)
      {
	awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);

	canvas->G2D()->FinishDraw();
      }
    }
  }
}

void awsMultiProctexCanvas::awscG2D::Clear(int color)
{
  DrawBox (0, 0, Width, Height, color);
}

void awsMultiProctexCanvas::awscG2D::ClearAll(int color)
{
  DrawBox (0, 0, Width, Height, color);
}

bool awsMultiProctexCanvas::awscG2D::DoubleBuffer (bool )
{
  return true;
}

bool awsMultiProctexCanvas::awscG2D::GetDoubleBufferState ()
{
  return false;
}

int awsMultiProctexCanvas::awscG2D::GetPage ()
{
  return 0;
}

int awsMultiProctexCanvas::awscG2D::GetPalEntryCount ()
{ 
/*  int count=awsc->GetFlatCanvasCount();
  if (count) {
    return awsc->GetFlatCanvas(0)->G2D()->GetPalEntryCount();
  } else {*/
    return 0; 
//  }
}

int awsMultiProctexCanvas::awscG2D::GetPixelBytes ()
{
  int count=awsc->GetFlatCanvasCount();
  if (count) {
    return awsc->GetFlatCanvas(0)->G2D()->GetPixelBytes();
  } else {
    return 0; 
  }
}

csPixelFormat* awsMultiProctexCanvas::awscG2D::GetPixelFormat ()
{ 
  int count=awsc->GetFlatCanvasCount();
  if (count) {
    return awsc->GetFlatCanvas(0)->G2D()->GetPixelFormat();
  } else {
    return NULL; 
  }
}

void awsMultiProctexCanvas::awscG2D::Print (csRect *area)
{
  int i, count=awsc->GetFlatCanvasCount();

  awsc->realG3D()->BeginDraw(CSDRAW_2DGRAPHICS);

  for (i=0; i<count; ++i)
  {
    csRect *canvasRect = awsc->GetFlatCanvasRect(i);
    csRect rect ((csRect&)*canvasRect);

    if (area) rect.Intersect(*area);
    if ((!area) || (!rect.IsEmpty()))
    {
      awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
      awsc->realG3D()->DrawPixmap(canvas->GetTextureWrapper()->GetTextureHandle(),
                                  rect.xmin, rect.ymin, rect.Width()+1,rect.Height()+1,
                                  rect.xmin-canvasRect->xmin, rect.ymin-canvasRect->ymin, 
                                  rect.Width()+1,rect.Height()+1,
                                  0);

      //awsc->realG3D()->GetDriver2D()->DrawBox(rect.xmin, rect.ymin, rect.Width(), rect.Height(), 0);
    }
  }

  awsc->realG3D()->FinishDraw();
}

//
SCF_IMPLEMENT_IBASE (awsMultiProctexCanvas::awscG3D)
SCF_IMPLEMENTS_INTERFACE (iGraphics3D)
SCF_IMPLEMENT_IBASE_END

awsMultiProctexCanvas::awscG3D::awscG3D (awsMultiProctexCanvas *parent, iGraphics3D *aG3D) : G2D (NULL)
{
  SCF_CONSTRUCT_IBASE (NULL);

  awsc = parent;

  rG3D = aG3D;
  G2D = NULL;

  texman = NULL;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_NONE;
  Caps.NeedsPO2Maps = false;
  Caps.MaxAspectRatio = 32768;
}

awsMultiProctexCanvas::awscG3D::~awscG3D ()
{
  Close ();
  texman->Clear ();
  texman->DecRef (); texman = NULL;
  if (G2D)
    G2D->DecRef ();
}

bool awsMultiProctexCanvas::awscG3D::Initialize (iObjectRegistry *r)
{
  object_reg = r;
//  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
//  plugin_mgr->DecRef ();

  config.AddConfig(object_reg, "");

  width = height = -1;

  G2D = new awsMultiProctexCanvas::awscG2D(awsc, rG3D->GetDriver2D());
  if (!G2D)
    return false;
  G2D->Initialize(r);

  texman = new csTextureManagerNull (object_reg, G2D, config);

  return true;
}

bool awsMultiProctexCanvas::awscG3D::Open ()
{
  DrawMode = 0;

  if (!G2D->Open ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.graphics3d.awsMultiProctexCanvas::awscG3D",
              "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;

    return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();
  //bool bFullScreen = G2D->GetFullScreen ();

  pfmt = *G2D->GetPixelFormat ();
  if (pfmt.PalEntries)
  {
    // If we don't have truecolor we simulate 5:6:5 bits
    // for R:G:B in the masks anyway because we still need the
    // 16-bit format for our light mixing
    pfmt.RedShift   = RGB2PAL_BITS_G + RGB2PAL_BITS_B;
    pfmt.GreenShift = RGB2PAL_BITS_B;
    pfmt.BlueShift  = 0;
    pfmt.RedMask    = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.RedShift;
    pfmt.GreenMask  = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.GreenShift;
    pfmt.BlueMask   = ((1 << RGB2PAL_BITS_B) - 1);
    pfmt.RedBits    = RGB2PAL_BITS_R;
    pfmt.GreenBits  = RGB2PAL_BITS_G;
    pfmt.BlueBits   = RGB2PAL_BITS_B;
  }
  texman->SetPixelFormat (pfmt);

  SetDimensions (nWidth, nHeight);

  z_buf_mode = CS_ZBUF_NONE;

  return true;
}

void awsMultiProctexCanvas::awscG3D::Close()
{
  if ((width == height) && (width == -1))
    return;

  texman->Clear ();
  texman->DecRef (); texman = NULL;

  G2D->Close ();
  width = height = -1;
}

void awsMultiProctexCanvas::awscG3D::SetDimensions (int nwidth, int nheight)
{
  width = nwidth;
  height = nheight;
  width2 = width/2;
  height2 = height/2;
}

void awsMultiProctexCanvas::awscG3D::SetPerspectiveCenter (int x, int y)
{
  width2 = x;
  height2 = y;
}

bool awsMultiProctexCanvas::awscG3D::BeginDraw (int DrawFlags)
{
  if ((G2D->GetWidth() != width) || 
      (G2D->GetHeight() != height))
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
      && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  awsc->realG3D()->BeginDraw(DrawFlags);

  int i, count=awsc->GetFlatCanvasCount();

  for (i=0; i<count; ++i)
  {
    awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
    canvas->G3D()->BeginDraw(DrawFlags);
  }

  DrawMode = DrawFlags;

  return true;
}

void awsMultiProctexCanvas::awscG3D::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();

  DrawMode = 0;
  awsc->realG3D()->FinishDraw();

  int i, count=awsc->GetFlatCanvasCount();

  for (i=0; i<count; ++i)
  {
    awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);
    canvas->G3D()->Print(NULL);
    canvas->G3D()->FinishDraw();
  }
}

void awsMultiProctexCanvas::awscG3D::Print (csRect *area)
{
  G2D->Print (area);
}

void awsMultiProctexCanvas::awscG3D::DrawPolygon (G3DPolygonDP& /*poly*/)
{
}

void awsMultiProctexCanvas::awscG3D::DrawPolygonFX (G3DPolygonDPFX& /*poly*/)
{
}

bool awsMultiProctexCanvas::awscG3D::SetRenderState (G3D_RENDERSTATEOPTION op,
                                                     long value)
{
  switch (op)
  {
  case G3DRENDERSTATE_ZBUFFERMODE:
    z_buf_mode = csZBufMode (value);
    break;
  default:
    return false;
  }

  return true;
}

long awsMultiProctexCanvas::awscG3D::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
  case G3DRENDERSTATE_ZBUFFERMODE:
    return z_buf_mode;
  default:
    return 0;
  }
}

void awsMultiProctexCanvas::awscG3D::DrawLine (const csVector3&, const csVector3&,
                                               float /*fov*/, int /*color*/)
{
}

void awsMultiProctexCanvas::awscG3D::DrawPixmap (iTextureHandle *hTex,
                                                 int sx, int sy, int sw, int sh,
                                                 int tx, int ty, int tw, int th, uint8 Alpha)
{
  if (Alpha == 255)
    return;

  csRect maprect (sx, sy, sx+sw, sy+sh);

  int i, count=awsc->GetFlatCanvasCount ();
  for (i=0; i<count; ++i)
  {
    csRect *canvasRect = awsc->GetFlatCanvasRect(i);
    csRect rect ((csRect&)*canvasRect);

    rect.Intersect(maprect);
    if (!rect.IsEmpty())
    {
      awsSimpleCanvas *canvas = awsc->GetFlatCanvas(i);

      int ntx = tx + QInt((((float)rect.xmin - (float)sx) / (float)sw) * (float)tw);
      int nty = ty + QInt((((float)rect.ymin - (float)sy) / (float)sh) * (float)th);
      int ntw = QInt((float)tw * ((float)rect.Width() / (float)sw));
      int nth = QInt((float)th * ((float)rect.Height() / (float)sh));

      canvas->G3D()->DrawPixmap(hTex, rect.xmin-canvasRect->xmin, rect.ymin-canvasRect->ymin, rect.Width(), rect.Height(),
                                ntx, nty, ntw, nth, Alpha);
    }

  }

}

////////

awsMultiProctexCanvas::awsMultiProctexCanvas(int w, int h, iObjectRegistry* object_reg, iEngine* engine,
                                             iTextureManager* txtmgr/*, iGraphics3D *myG3D*/): canvas_list(0), rect_list(0)
{
  SCF_CONSTRUCT_IBASE (NULL);

  const int DesiredCanvasWidth = w;
  const int DesiredCanvasHeight = h;

  int MaxCanvasWidth = 256;
  int MaxCanvasHeight = 256;

  while ((MaxCanvasWidth >> 1) >= DesiredCanvasWidth) MaxCanvasWidth >>= 1;
  HorzCanvases = DesiredCanvasWidth / MaxCanvasWidth;
  if (DesiredCanvasWidth % MaxCanvasWidth) HorzCanvases++;

  while ((MaxCanvasHeight >> 1) >= DesiredCanvasHeight) MaxCanvasHeight >>= 1;
  VertCanvases = DesiredCanvasHeight / MaxCanvasHeight;
  if (DesiredCanvasHeight % MaxCanvasHeight) VertCanvases++;

  int vert=0,hor=0,i,count=GetFlatCanvasCount();

  // Initialize the canvas and rect list.
  canvas_list = new awsSimpleCanvas[HorzCanvases*VertCanvases];
  rect_list   = new csRect[HorzCanvases*VertCanvases];

  for (i=0; i<count; ++i)
  {
    // Setup the canvas that deals with this patch.  
    awsSimpleCanvas *canvas = GetFlatCanvas(i);

    canvas->DisableAutoUpdate();
    canvas->SetSize(MaxCanvasWidth, MaxCanvasHeight);
    canvas->SetKeyColor(255,0,255);
    canvas->Initialize(object_reg, engine, txtmgr, NULL);
    canvas->PrepareAnim();

    // Setup the rect that identifies where we are in screen coords.
    csRect *rect = GetFlatCanvasRect(i);

    int sx = hor*MaxCanvasWidth;
    int sy = vert*MaxCanvasHeight;

    rect->Set(sx,sy,sx+MaxCanvasWidth,sy+MaxCanvasHeight);
    
    // Inc hor and vert counters
    if (!(++hor<HorzCanvases))
    {
      hor=0; ++vert;
    }
  }

  CanvasWidth = DesiredCanvasWidth;
  CanvasHeight = DesiredCanvasHeight;

  rG3D = engine->GetContext();
  rG2D = rG3D->GetDriver2D();

  //iPluginManager *plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  vG3D = new awsMultiProctexCanvas::awscG3D(this, canvas_list[0].G3D());
  ((awsMultiProctexCanvas::awscG3D*)vG3D)->Initialize(object_reg);
  vG3D->Open();
  vG2D = (awscG2D*)vG3D->GetDriver2D();
  //SCF_DEC_REF (plugin_mgr);
}

awsMultiProctexCanvas::~awsMultiProctexCanvas ()
{
  if (canvas_list) delete canvas_list;
  if (rect_list)   delete rect_list;

  vG3D->Close();
  vG2D->Close();

  SCF_DEC_REF (vG3D);
  SCF_DEC_REF (vG2D);
  SCF_DEC_REF (rG3D);
  SCF_DEC_REF (rG2D);
}

void 
awsMultiProctexCanvas::Animate (csTicks current_time)
{
  (void)current_time;
}

void awsMultiProctexCanvas::Show (csRect *area, iGraphics3D *g3d, uint8 Alpha)
{
  (void) g3d;
  //G3D->Print (area);

  int i, count=GetFlatCanvasCount();

  realG3D()->BeginDraw(CSDRAW_2DGRAPHICS);

  for (i=0; i<count; ++i)
  {
    csRect *canvasRect = GetFlatCanvasRect(i);
    csRect rect ((csRect&)*canvasRect);

    if (area) rect.Intersect(*area);
    if ((!area) || (!rect.IsEmpty()))
    {
      awsSimpleCanvas *canvas = GetFlatCanvas(i);
      realG3D()->DrawPixmap(canvas->GetTextureWrapper()->GetTextureHandle(),
                            rect.xmin, rect.ymin, rect.Width()+1,rect.Height()+1,
                            rect.xmin-canvasRect->xmin, rect.ymin-canvasRect->ymin, 
                            rect.Width()+1,rect.Height()+1,
                            Alpha);

    }
  }

  realG3D()->FinishDraw();
}

