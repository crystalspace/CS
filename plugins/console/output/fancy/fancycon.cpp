/*
    Copyright (C) 2000 by Norman Krmer

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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cssysdef.h"
#include "fancycon.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "isys/event.h"
#include "isys/vfs.h"
#include "csutil/csrect.h"
#include "csutil/csstring.h"
#include "csutil/cfgacc.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"

IMPLEMENT_IBASE(csFancyConsole)
  IMPLEMENTS_INTERFACE(iConsoleOutput)
  IMPLEMENTS_EMBEDDED_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csFancyConsole::eiPlugIn)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_FACTORY(csFancyConsole)

EXPORT_CLASS_TABLE (fancycon)
  EXPORT_CLASS (csFancyConsole, "crystalspace.console.output.fancy",
		"Crystal Space fancy output console")
EXPORT_CLASS_TABLE_END

csFancyConsole::csFancyConsole (iBase *p) :
  System(0), VFS(0), base(0), G2D(0), G3D(0), ImageLoader(NULL), border_computed(false),
  pix_loaded(false), system_ready(false), auto_update(true), visible(true)
{
  CONSTRUCT_IBASE (p);
  CONSTRUCT_EMBEDDED_IBASE(scfiPlugIn);
}

csFancyConsole::~csFancyConsole ()
{
  if (ImageLoader)
    ImageLoader->DecRef();
  if (G2D)
    G2D->DecRef ();
  if (G3D)
    G3D->DecRef ();    
  if (base)
    base->DecRef ();
  if (VFS)
    VFS->DecRef ();
}

bool csFancyConsole::Initialize (iSystem *system) 
{
  System = system;

  VFS = QUERY_PLUGIN_ID (System, CS_FUNCID_VFS, iVFS);
  if (!VFS)
    return false;

  csConfigAccess ini(System, "/config/fancycon.cfg");
  char const* baseclass = ini->GetStr("FancyConsole.General.Superclass",
    "crystalspace.console.output.standard");
  base = LOAD_PLUGIN(System, baseclass, 0, iConsoleOutput);
  if (!base)
    return false;

  G3D = QUERY_PLUGIN_ID (System, CS_FUNCID_VIDEO, iGraphics3D);
  if (!G3D)
    return false;
  G2D = G3D->GetDriver2D ();
  G2D->IncRef ();

  ImageLoader = NULL;

  // Tell system driver that we want to handle broadcast events
  if (!System->CallOnEvents (&scfiPlugIn, CSMASK_Broadcast))
    return false;

  int x, y, w, h;
  base->ConsoleExtension("GetPos", &x, &y, &w, &h);
  outersize.Set (x, y, x + w, y + h);

  return true;
}

bool csFancyConsole::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdSystemOpen:
          system_ready = true;
	  if (!pix_loaded)
	  {
            ImageLoader = QUERY_PLUGIN_ID (System, CS_FUNCID_IMGLOADER, iImageIO);
	    LoadPix ();
	    pix_loaded = true;
	  }
          return true;
        case cscmdSystemClose:
          system_ready = false;
          return true;
      }
      break;
  }
  return false;
}

void csFancyConsole::PutText (int iMode, const char *iText)
{
  base->AutoUpdate(false);
  base->PutText (iMode, iText);
  base->AutoUpdate(auto_update);
  if (auto_update && system_ready && G3D->BeginDraw (CSDRAW_2DGRAPHICS))
  {
    int bgcolor;
    base->ConsoleExtension("GetBackgroundColor", &bgcolor);
    G2D->Clear (bgcolor);
    csRect rect2;
    Draw2D (&rect2);

    G3D->BeginDraw (CSDRAW_3DGRAPHICS);
    csRect rect3;
    Draw2D (&rect3);
    rect2.Union (rect3);
    G3D->FinishDraw ();
    G3D->Print (&rect2);
  }
}

void csFancyConsole::Draw3D (csRect *oArea)
{
  if (!visible || !ImageLoader) return;

  bool btext, bgour;
  int i;
  long int zBuf;
  G3DPolygonDPFX poly;
  if (!border_computed)
  {
    // determine what space left to draw the actual console
    memset (&bordersize, 0, sizeof (bordersize));
    if (deco.border [0].mat)
      deco.border [0].mat->GetTexture ()->GetMipMapDimensions(
        0, bordersize.xmin, bordersize.ymin);
    if (deco.border[4].mat)
      deco.border [4].mat->GetTexture ()->GetMipMapDimensions(
        0, bordersize.xmax, bordersize.ymax);

    SetTransparency (true); // Otherwise 2D-part will overdraw what we paint.
    border_computed = true;
    SetPosition (
      outersize.xmin, outersize.ymin, outersize.Width (), outersize.Height ());
  }

  zBuf = G3D->GetRenderState (G3DRENDERSTATE_ZBUFFERMODE);
  btext = G3D->GetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE);
  bgour = G3D->GetRenderState (G3DRENDERSTATE_GOURAUDENABLE);

  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);

  int height = G3D->GetHeight () - 1;

  // first draw the background
  // do we draw gouraud/flat or with texture ?
 
  bool with_color = deco.bgnd.mat == NULL;

  csRect size (outersize);
  size.xmin +=  bordersize.xmin - deco.p2lx - deco.lx;
  size.xmax += -bordersize.ymax + deco.p2rx + deco.rx;
  size.ymin +=  bordersize.ymin - deco.p2ty - deco.ty;
  size.ymax += -bordersize.ymax + deco.p2by + deco.by;

  poly.num = 4;
  poly.vertices [0].sx = size.xmin;
  poly.vertices [0].sy = height - size.ymin;
  poly.vertices [1].sx = size.xmax;
  poly.vertices [1].sy = height - size.ymin;
  poly.vertices [2].sx = size.xmax;
  poly.vertices [2].sy = height - size.ymax;
  poly.vertices [3].sx = size.xmin;
  poly.vertices [3].sy = height - size.ymax;
  poly.use_fog = false;

  float u_stretch = 1.0, v_stretch = 1.0;

  if (!with_color && !deco.bgnd.do_stretch)
  {
    int w, h;
    deco.bgnd.mat->GetTexture ()->GetMipMapDimensions (0, w, h);
    u_stretch = ((float)(size.xmax - size.xmin)) / ((float)w);
    v_stretch = ((float)(size.ymax - size.ymin)) / ((float)h);
  }

  poly.vertices [0].u = 0;
  poly.vertices [0].v = 0;
  poly.vertices [1].u = u_stretch;
  poly.vertices [1].v = 0;
  poly.vertices [2].u = u_stretch;
  poly.vertices [2].v = v_stretch;
  poly.vertices [3].u = 0;
  poly.vertices [3].v = v_stretch;
    
  for (i = 0; i < poly.num; i++)
  {
    poly.vertices [i].r = ((float)deco.bgnd.kr) * (1 / 255.0);
    poly.vertices [i].g = ((float)deco.bgnd.kg) * (1 / 255.0);
    poly.vertices [i].b = ((float)deco.bgnd.kb) * (1 / 255.0);
    poly.vertices [i].z = 1;
  }

  poly.mat_handle = deco.bgnd.mat;
  if (with_color)
    G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, false);
  
  float alpha = deco.bgnd.do_alpha ? deco.bgnd.alpha : 0.0;

  poly.mixmode = CS_FX_SETALPHA (alpha) |
    CS_FX_COPY | (with_color && deco.bgnd.do_keycolor ? CS_FX_GOURAUD : 0);
  G3D->DrawPolygonFX (poly);

  if (with_color)
    G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, true);
  
  // draw the top left decoration
  DrawBorder (outersize.xmin, height-outersize.ymin, bordersize.xmin,
    bordersize.ymin, deco.border[0], 0);
  // draw the top decoration
  DrawBorder (p2size.xmin-deco.p2lx, height-outersize.ymin,
    p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymin,  deco.border[1], 1);
  // draw the top right decoration
  DrawBorder (p2size.xmax, height-outersize.ymin, bordersize.xmax,
    bordersize.ymin, deco.border[2], 0);
  // draw the right decoration
  DrawBorder (p2size.xmax, height-p2size.ymin+deco.p2ty, bordersize.xmax,
    p2size.Height()+deco.p2by+deco.p2ty, deco.border[3], 2);
  // draw the bottom right decoration
  DrawBorder (p2size.xmax, height-p2size.ymax, bordersize.xmax,
    bordersize.ymax, deco.border[4], 0);
  // draw the bottom decoration
  DrawBorder (p2size.xmin-deco.p2lx, height-p2size.ymax,
    p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymax, deco.border[5], 3);
  // draw the bottom left decoration
  DrawBorder (outersize.xmin, height-p2size.ymax, bordersize.xmin,
    bordersize.ymax, deco.border[6], 0);
  // draw the left decoration
  DrawBorder (outersize.xmin, height-p2size.ymin+deco.p2ty, bordersize.xmin,
    p2size.Height()+deco.p2by+deco.p2ty, deco.border[7], 4);
  
  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zBuf);
  G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, btext);
  G3D->SetRenderState (G3DRENDERSTATE_GOURAUDENABLE, bgour);

  if (oArea)
    *oArea = outersize;
}

void csFancyConsole::DrawBorder (int x, int y, int width, int height,
  ConDecoBorder &border, int align)
{
  if (border.mat)
  {
    G3DPolygonDPFX poly;
    int i;

    float u_stretch = 1.0, v_stretch = 1.0;
    int w, h;
    
    border.mat->GetTexture ()->GetMipMapDimensions (0, w, h);
    switch (align)
    {
      case 1:
        height = MIN (height, h);
        h = height;
        break;
      case 2:
        x += MAX (0, width - w);
        width = MIN (width, w);
        w = width;
        break;
      case 3:
        y -= MAX (0, height - h);
        height = MIN (h, height);
        h = height;
        break;
      case 4:
        width = MIN (width, w);
        w = width;
        break;
    }
    
    if (!border.do_stretch)
    {
      u_stretch = ((float)width) / ((float)w);
      v_stretch = ((float)height) / ((float)h);
    }

    poly.num = 4;
    poly.use_fog = false;
    poly.vertices [0].u = 0;
    poly.vertices [0].v = 0;
    poly.vertices [1].u = u_stretch;
    poly.vertices [1].v = 0;
    poly.vertices [2].u = u_stretch;
    poly.vertices [2].v = v_stretch;
    poly.vertices [3].u = 0;
    poly.vertices [3].v = v_stretch;

    poly.vertices [0].sx = x;
    poly.vertices [0].sy = y;
    poly.vertices [1].sx = x + width ;
    poly.vertices [1].sy = y;
    poly.vertices [2].sx = x + width;
    poly.vertices [2].sy = y - height;
    poly.vertices [3].sx = x;
    poly.vertices [3].sy = y - height;

    for (i = 0; i < 4; i++)
    {
      poly.vertices [i].sx -= border.offx;
      poly.vertices [i].sy += border.offy;
      poly.vertices [i].z = 1;
      poly.vertices [i].r = 1;
      poly.vertices [i].g = 1;
      poly.vertices [i].b = 1;
    }

    poly.mat_handle = border.mat;

    float alpha = border.do_alpha ? border.alpha : 0.0;
    poly.mixmode = CS_FX_SETALPHA (alpha)
      | (border.do_keycolor ? CS_FX_KEYCOLOR : 0);
    G3D->DrawPolygonFX (poly);
  }
}

void csFancyConsole::SetPosition (int x, int y, int width, int height)
{
  base->ConsoleExtension("SetPos", x, y, width, height);
  base->ConsoleExtension("GetPos", &x, &y, &width, &height);
  csRect size;
  size.Set (x, y, x + width, y + height);
  outersize.Set (size);
  p2size.Set (size);
  p2size.xmin +=  bordersize.xmin; // - deco.p2lx;
  p2size.xmax += -bordersize.xmax; // + deco.p2rx;
  p2size.ymin +=  bordersize.ymin; // - deco.p2ty;
  p2size.ymax += -bordersize.ymax; // + deco.p2by;

  if (border_computed)
  {
    size.xmin +=  bordersize.xmin - deco.p2lx - deco.lx;
    size.xmax += -bordersize.xmax + deco.p2rx + deco.rx;
    size.ymin +=  bordersize.ymin - deco.p2ty - deco.ty;
    size.ymax += -bordersize.ymax + deco.p2by + deco.by;
    // call again with the final size
    base->ConsoleExtension(
      "SetPos", size.xmin, size.ymin, size.Width(), size.Height());
  }
}

void csFancyConsole::GetPosition(int &x, int &y, int &width, int &height) const
{
  x = outersize.xmin;
  y = outersize.ymin;
  width = outersize.Width ();
  height = outersize.Height ();
}

void csFancyConsole::LoadPix ()
{
  csConfigAccess ini(System, "/config/fancycon.cfg");

  const char* dir = ini->GetStr ("FancyConsole.General.Archive");
  const char* mountdir = ini->GetStr ("FancyConsole.General.Mount");
  if (!dir || !mountdir)
    System->Printf (MSG_WARNING,
      "FancyConsole: Data resource location unknown\n");
  else if (VFS->Mount (mountdir, dir))
  {
    VFS->PushDir ();
    VFS->ChDir (mountdir);

    // scan in all sections
    PrepPix (ini, "Background", deco.bgnd, true);
    PrepPix (ini, "TopLeft", deco.border[0], false);
    PrepPix (ini, "Top", deco.border[1], false);
    PrepPix (ini, "TopRight", deco.border[2], false);
    PrepPix (ini, "Right", deco.border[3], false);
    PrepPix (ini, "BottomRight", deco.border[4], false);
    PrepPix (ini, "Bottom", deco.border[5], false);
    PrepPix (ini, "BottomLeft", deco.border[6], false);
    PrepPix (ini, "Left", deco.border[7], false);

    // internal increase/decrease
    deco.p2lx = ini->GetInt ("FancyConsole.General.p2lx");
    deco.p2rx = ini->GetInt ("FancyConsole.General.p2rx");
    deco.p2ty = ini->GetInt ("FancyConsole.General.p2ty");
    deco.p2by = ini->GetInt ("FancyConsole.General.p2by");
    deco.lx = ini->GetInt ("FancyConsole.General.lx");
    deco.rx = ini->GetInt ("FancyConsole.General.rx");
    deco.ty = ini->GetInt ("FancyConsole.General.ty");
    deco.by = ini->GetInt ("FancyConsole.General.by");

    VFS->PopDir ();
    VFS->Unmount (mountdir, dir);
  }
  else
    System->Printf (MSG_WARNING, "Could not mount %s on %s\n", dir, mountdir);
}

void csFancyConsole::PrepPix (iConfigFile *ini, const char *sect,
  ConDecoBorder &border, bool bgnd)
{
  csString Keyname;
  Keyname.Clear() << "FancyConsole." << sect << ".pic";
  const char* pix = ini->GetStr (Keyname, "");

  border.mat = NULL;
  border.do_keycolor = false;
  border.do_alpha = false;
  border.do_stretch = false;

  if (strlen (pix))
  {
    size_t len = 0;
    char *data = NULL;
    iFile *F = VFS->Open (pix, VFS_FILE_READ);
    if (F)
    {
      len = F->GetSize ();
      data = new char [len];
      if (data) len = F->Read (data, len);
      F->DecRef ();
    }
    if (len)
    {
      iTextureManager *tm = G3D->GetTextureManager ();
      iImage *image =
        ImageLoader->Load ((UByte *)data, len, tm->GetTextureFormat ());
      if (image)
      {
	iTextureHandle* txt =
	  tm->RegisterTexture ( image, CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS );
	iMaterialHandle* mat = tm->RegisterMaterial (txt);
	border.mat = mat;
	image->DecRef();

        Keyname.Clear() << "FancyConsole." << sect << ".x";
	border.offx = ini->GetInt (Keyname, 0);
        Keyname.Clear() << "FancyConsole." << sect << ".y";
        border.offy = ini->GetInt (Keyname, 0);

        Keyname.Clear() << "FancyConsole." << sect << ".do_keycolor";
	border.do_keycolor = ini->GetBool (Keyname, false);
	if (border.do_keycolor)
        {
	  int r,g,b;
          Keyname.Clear() << "FancyConsole." << sect << ".keycolor";
	  const char *kc = ini->GetStr(Keyname, "0,0,0" );
	  sscanf( kc, "%d,%d,%d", &r, &g, &b );
	  border.kr=r; border.kg=g; border.kb=b;
	  border.mat->GetTexture ()->SetKeyColor(
	    border.kr, border.kg, border.kb);
	}

        Keyname.Clear() << "FancyConsole." << sect << ".do_stretch";
	border.do_stretch = ini->GetBool (Keyname, false);
      }
      delete [] data;
    }
    else
      System->Printf(MSG_WARNING, "Could not read %s\n", pix);
  }

  Keyname.Clear() << "FancyConsole." << sect << ".do_alpha";
  border.do_alpha = ini->GetBool (Keyname, false);
  if (border.do_alpha)
    Keyname.Clear() << "FancyConsole." << sect << ".alpha";
    border.alpha = ini->GetFloat (Keyname, 0.0);
  
  if (bgnd)
  {
    int r,g,b;
    Keyname.Clear() << "FancyConsole." << sect << ".do_keycolor";
    border.do_keycolor = ini->GetBool (Keyname, false);
    Keyname.Clear() << "FancyConsole." << sect << ".keycolor";
    const char *kc = ini->GetStr (Keyname, "1,1,1");
    sscanf (kc, "%d,%d,%d", &r, &g, &b);
    border.kr = r; border.kg = g; border.kb = b;
  }
}

bool csFancyConsole::ConsoleExtension (const char *iCommand, ...)
{
  va_list args;
  va_start (args, iCommand);
  bool rc = ConsoleExtensionV(iCommand, args);
  va_end (args);
  return rc;
}

bool csFancyConsole::ConsoleExtensionV (const char *iCommand, va_list args)
{
  bool rc = true;
  if (!strcmp (iCommand, "GetPos"))
  {
    int *x = va_arg (args, int *);
    int *y = va_arg (args, int *);
    int *w = va_arg (args, int *);
    int *h = va_arg (args, int *);
    GetPosition (*x, *y, *w, *h);
  }
  else if (!strcmp (iCommand, "SetPos"))
  {
    int x = va_arg (args, int);
    int y = va_arg (args, int);
    int w = va_arg (args, int);
    int h = va_arg (args, int);
    SetPosition (x, y, w, h);
  }
  else
    rc = base->ConsoleExtensionV(iCommand, args);
  return rc;
}
