/*
    Copyright (C) 2000 by Norman Krmer
              (C) 2004 by Marten Svanfeldt

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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "csutil/sysfunc.h"
#include "fancycon.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "csgeom/csrect.h"
#include "csutil/csstring.h"
#include "csutil/cfgacc.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csFancyConsole)
  SCF_IMPLEMENTS_INTERFACE(iConsoleOutput)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFancyConsole::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFancyConsole::EventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY(csFancyConsole)



void csFancyConsole::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.console.output.fancy", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csFancyConsole::csFancyConsole (iBase *p) :
  object_reg(0), border_computed(false),
  pix_loaded(false), system_ready(false), auto_update(true), visible(true)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
}

csFancyConsole::~csFancyConsole ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFancyConsole::Initialize (iObjectRegistry *object_reg)
{
  csFancyConsole::object_reg = object_reg;

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
    return false;

  csConfigAccess ini(object_reg, "/config/fancycon.cfg");
  char const* baseclass = ini->GetStr("FancyConsole.General.Superclass",
    "crystalspace.console.output.standard");
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  base = CS_LOAD_PLUGIN (plugin_mgr, baseclass, iConsoleOutput);
  if (!base)
    return false;

  G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D)
    return false;
  G2D = G3D->GetDriver2D ();

  ImageLoader = 0;

  // Tell event queue that we want to handle broadcast events
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  int x, y, w, h;
  base->PerformExtension("GetPos", &x, &y, &w, &h);
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
            ImageLoader = CS_QUERY_REGISTRY (object_reg, iImageIO);
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

void csFancyConsole::PutTextV (const char *iText, va_list args)
{
  base->AutoUpdate(false);
  base->PutTextV (iText, args);
  base->AutoUpdate(auto_update);
  if (auto_update && system_ready && G3D->BeginDraw (CSDRAW_2DGRAPHICS))
  {
    int bgcolor;
    base->PerformExtension("GetBackgroundColor", &bgcolor);
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
  csSimpleRenderMesh mesh;
  if (!border_computed)
  {
    // determine what space left to draw the actual console
    memset (&bordersize, 0, sizeof (bordersize));
    if (deco.border [0].txt)
      deco.border [0].txt->GetRendererDimensions (
        bordersize.xmin, bordersize.ymin);
    if (deco.border[4].txt)
      deco.border [4].txt->GetRendererDimensions (
        bordersize.xmax, bordersize.ymax);

    SetTransparency (true); // Otherwise 2D-part will overdraw what we paint.
    border_computed = true;
    SetPosition (
      outersize.xmin, outersize.ymin, outersize.Width (), outersize.Height ());
  }

  zBuf = G3D->GetRenderState (G3DRENDERSTATE_ZBUFFERMODE);
  btext = G3D->GetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE);
  bgour = G3D->GetRenderState (G3DRENDERSTATE_GOURAUDENABLE);

  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);

  // first draw the background
  // do we draw gouraud/flat or with texture ?

  bool with_color = (deco.bgnd.txt == 0);

  csRect size (outersize);
  size.xmin +=  bordersize.xmin - deco.p2lx - deco.lx;
  size.xmax += -bordersize.ymax + deco.p2rx + deco.rx;
  size.ymin +=  bordersize.ymin - deco.p2ty - deco.ty;
  size.ymax += -bordersize.ymax + deco.p2by + deco.by;

  static uint indices[4] = {0, 1, 2, 3};
  csVector3 vertices[4];
  csVector2 texels[4];
  csVector4 colors[4];

  vertices[0].Set (size.xmin, size.ymin, 0);
  vertices[1].Set (size.xmax,  size.ymin, 0);
  vertices[2].Set (size.xmax,  size.ymax, 0);
  vertices[3].Set (size.xmin, size.ymax, 0);


  float u_stretch = 1.0, v_stretch = 1.0;

  if (!deco.bgnd.do_stretch)
  {
    int w, h;
    deco.bgnd.txt->GetRendererDimensions (w, h);
    u_stretch = ((float)(size.xmax - size.xmin)) / ((float)w);
    v_stretch = ((float)(size.ymax - size.ymin)) / ((float)h);
  }

  texels [0].x = 0;
  texels [0].y = 0;
  texels [1].x = u_stretch;
  texels [1].y = 0;
  texels [2].x = u_stretch;
  texels [2].y = v_stretch;
  texels [3].x = 0;
  texels [3].y = v_stretch;

  float alpha = 1;
  if (deco.bgnd.do_alpha)
  {
    alpha = deco.bgnd.alpha;
    mesh.mixmode = CS_FX_COPY | CS_FX_FLAT;
    mesh.alphaType.autoAlphaMode = false;
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
  }
  else
  {
    mesh.mixmode = CS_FX_COPY | CS_FX_FLAT;
  }
  

  for (i = 0; i < 4; i++)
  {
    if (with_color)
    {
      colors [i].x = ((float)deco.bgnd.kr) * (1 / 255.0);
      colors [i].y = ((float)deco.bgnd.kg) * (1 / 255.0);
      colors [i].z = ((float)deco.bgnd.kb) * (1 / 255.0);
    }
    else
      colors [i].x = colors [i].y = colors [i].z = 1.0f;
    colors [i].w = alpha;
  }
  
  mesh.vertices = vertices;
  mesh.texcoords = texels;
  mesh.indices = indices;
  mesh.indexCount = 4;
  mesh.vertexCount = 4;

  if (!with_color)
    mesh.texture = deco.bgnd.txt;
  mesh.colors = colors;

  mesh.meshtype = CS_MESHTYPE_QUADS;
  
  G3D->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);

  // draw the top left decoration
  DrawBorder (outersize.xmin, outersize.ymin, bordersize.xmin,
    bordersize.ymin, deco.border[0], 0);
  // draw the top decoration
  DrawBorder (p2size.xmin-deco.p2lx, outersize.ymin,
    p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymin,  deco.border[1], 1);
  // draw the top right decoration
  DrawBorder (p2size.xmax, outersize.ymin, bordersize.xmax,
    bordersize.ymin, deco.border[2], 0);
  // draw the right decoration
  DrawBorder (p2size.xmax, p2size.ymin-deco.p2ty, bordersize.xmax,
    p2size.Height()+deco.p2by+deco.p2ty, deco.border[3], 2);
  // draw the bottom right decoration
  DrawBorder (p2size.xmax, p2size.ymax, bordersize.xmax,
      bordersize.ymax, deco.border[4], 0);
  // draw the bottom decoration
  DrawBorder (p2size.xmin-deco.p2lx, p2size.ymax,
    p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymax, deco.border[5], 3);
  // draw the bottom left decoration
  DrawBorder (outersize.xmin, p2size.ymax, bordersize.xmin,
    bordersize.ymax, deco.border[6], 0);
  // draw the left decoration
  DrawBorder (outersize.xmin, p2size.ymin-deco.p2ty, bordersize.xmin,
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
  if (border.txt)
  {
    csSimpleRenderMesh mesh;
    static uint indices[4] = {0, 1, 2, 3};
    csVector3 vertices[4];
    csVector2 texels[4];
    csVector4 colors[4];
    int i;

    float u_stretch = 1.0, v_stretch = 1.0;
    int w, h;

    border.txt->GetRendererDimensions (w, h);
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
        y += MAX (0, height - h);
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

    texels [0].x = 0;
    texels [0].y = 0;
    texels [1].x = u_stretch;
    texels [1].y = 0;
    texels [2].x = u_stretch;
    texels [2].y = v_stretch;
    texels [3].x = 0;
    texels [3].y = v_stretch;

    vertices [0].x = x;
    vertices [0].y = y;
    vertices [0].z = 0;
    vertices [1].x = x + width ;
    vertices [1].y = y;
    vertices [1].z = 0;
    vertices [2].x = x + width;
    vertices [2].y = y + height;
    vertices [2].z = 0;
    vertices [3].x = x;
    vertices [3].y = y + height;
    vertices [3].z = 0;

    float alpha = 1;

    if (border.do_alpha)
    {
      alpha = deco.bgnd.alpha;
      mesh.alphaType.autoAlphaMode = false;
      mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
    }
    

    for (i = 0; i < 4; i++)
    {
      vertices [i].x -= border.offx;
      vertices [i].y += border.offy;
      colors [i].x = 1;
      colors [i].y = 1;
      colors [i].z = 1;
      colors [i].w = alpha;
    }

    mesh.texture = border.txt;
    mesh.mixmode = CS_FX_COPY | CS_FX_FLAT;
    
    mesh.vertices = vertices;
    mesh.texcoords = texels;
    mesh.indices = indices;
    mesh.indexCount = 4;
    mesh.vertexCount = 4;
    mesh.meshtype = CS_MESHTYPE_QUADS;

    G3D->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
  }
}

void csFancyConsole::SetPosition (int x, int y, int width, int height)
{
  base->PerformExtension("SetPos", x, y, width, height);
  base->PerformExtension("GetPos", &x, &y, &width, &height);
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
    base->PerformExtension(
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
  csConfigAccess ini(object_reg, "/config/fancycon.cfg");

  const char* dir = ini->GetStr ("FancyConsole.General.Archive");
  const char* mountdir = ini->GetStr ("FancyConsole.General.Mount");
  if (!*mountdir)
    Report (CS_REPORTER_SEVERITY_WARNING,
      "FancyConsole: Data resource location unknown");
  else 
  {
    if (!*dir || VFS->Mount (mountdir, dir))
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
      if (strlen (dir))
	VFS->Unmount (mountdir, dir);
    }
    else
      Report (CS_REPORTER_SEVERITY_WARNING,
    	  "Could not mount %s on %s", dir, mountdir);
  }
}

void csFancyConsole::PrepPix (iConfigFile *ini, const char *sect,
  ConDecoBorder &border, bool bgnd)
{
  csString Keyname;
  Keyname.Clear() << "FancyConsole." << sect << ".pic";
  const char* pix = ini->GetStr (Keyname, "");

  border.txt = 0;
  border.do_keycolor = false;
  border.do_alpha = false;
  border.do_stretch = false;

  if (strlen (pix))
  {
    csRef<iDataBuffer> Fbuf = VFS->ReadFile (pix, false);
    if (Fbuf)
    {
      iTextureManager *tm = G3D->GetTextureManager ();
      csRef<iImage> image (
        ImageLoader->Load (Fbuf, tm->GetTextureFormat ()));
      if (image)
      {
	border.txt =
	  tm->RegisterTexture (image, CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);

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
	  border.txt->SetKeyColor(
	    border.kr, border.kg, border.kb);
	}

        Keyname.Clear() << "FancyConsole." << sect << ".do_stretch";
	border.do_stretch = ini->GetBool (Keyname, false);
      }
    }
    else
      Report (CS_REPORTER_SEVERITY_WARNING, "Could not read %s", pix);
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

bool csFancyConsole::PerformExtension (const char *iCommand, ...)
{
  va_list args;
  va_start (args, iCommand);
  bool rc = PerformExtensionV(iCommand, args);
  va_end (args);
  return rc;
}

bool csFancyConsole::PerformExtensionV (const char *iCommand, va_list args)
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
    rc = base->PerformExtensionV(iCommand, args);
  return rc;
}
