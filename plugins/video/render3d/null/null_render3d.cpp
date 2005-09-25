/*
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "csqint.h"

#include "csgeom/plane3.h"
#include "csgfx/memimage.h"
#include "cstool/initapp.h"
#include "csutil/event.h"
#include "csutil/scfstrset.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/bugplug.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include "ivideo/rndbuf.h"

#include "null_render3d.h"

#include "csplugincommon/render3d/normalizationcube.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csNullGraphics3D)

SCF_IMPLEMENT_IBASE (csNullGraphics3D)
  SCF_IMPLEMENTS_INTERFACE (iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShaderRenderInterface)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullGraphics3D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullGraphics3D::eiShaderRenderInterface)
  SCF_IMPLEMENTS_INTERFACE (iShaderRenderInterface)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csNullGraphics3D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csNullGraphics3D::csNullGraphics3D (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiShaderRenderInterface);

  scfiEventHandler = 0;
  txtmgr = 0;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.MaxAspectRatio = 32768;
  Caps.NeedsPO2Maps = false;
  Caps.SupportsPointSprites = false;
  Caps.DestinationAlpha = false;
  Caps.StencilShadows = false;

  current_drawflags = 0;
}

csNullGraphics3D::~csNullGraphics3D ()
{
  txtmgr->Clear ();
  txtmgr->DecRef (); txtmgr = 0;
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q != 0) 
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler = 0;
  }
  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csNullGraphics3D::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  if (!scfiEventHandler)
    scfiEventHandler = csPtr<EventHandler> (new EventHandler (this));
  csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (q)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);
  
  bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.renderer.stringset", iStringSet);
  if (!strings)
  { 
    strings = csPtr<iStringSet> (new csScfStringSet ());
    object_reg->Register (strings, "crystalspace.renderer.stringset");
  }

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
  	object_reg, iPluginManager);
  if (!plugin_mgr) 
    return false;
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (
  	object_reg, iCommandLineParser);

  config.AddConfig (object_reg, "/config/null3d.cfg");

  const char *driver = 0;
  if (cmdline)
    driver = cmdline->GetOption ("canvas");

  if (!driver)
    driver = config->GetStr ("Video.Null.Canvas", CS_SOFTWARE_2D_DRIVER);

  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (!G2D)
    G2D = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.graphics2d.null", iGraphics2D);
  if (!G2D)
    return false;

  object_reg->Register (G2D, "iGraphics2D");

  txtmgr = new csTextureManagerNull (object_reg, G2D, config);

  return true;
}

bool csNullGraphics3D::HandleEvent (iEvent& e)
{
  if (e.Type == csevBroadcast)
  {
    switch (csCommandEventHelper::GetCode(&e))
    {
      case cscmdSystemOpen:
        Open ();
        return true;
      case cscmdSystemClose:
        Close ();
        return true;
    }
  }
  return false;
}

bool csNullGraphics3D::Open ()
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
  	object_reg, iPluginManager);
  if (!plugin_mgr)
    return false;
  if (!G2D->Open ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.graphics3d.null",
      "Error opening Graphics2D context");
    w = h = -1;
    return false;
  }
  bool fs = G2D->GetFullScreen ();

  pfmt = *G2D->GetPixelFormat ();
  SetDimensions (G2D->GetWidth (), G2D->GetHeight());

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.render3d.null", "Using %s mode %dx%d.",
    fs ? "full screen" : "windowed", w, h);

  SetPerspectiveAspect (G2D->GetHeight ());
  SetPerspectiveCenter (G2D->GetWidth ()/2, G2D->GetHeight ()/2);

  CS_QUERY_REGISTRY_PLUGIN(shadermgr, object_reg,
    "crystalspace.graphics3d.shadermanager", iShaderManager);

  string_vertices = strings->Request ("vertices");
  string_texture_coordinates = strings->Request ("texture coordinates");
  string_normals = strings->Request ("normals");
  string_colors = strings->Request ("colors");
  string_indices = strings->Request ("indices");


  // @@@ These shouldn't be here, I guess.
  #define CS_FOGTABLE_SIZE 64
  // Each texel in the fog table holds the fog alpha value at a certain
  // (distance*density).  The median distance parameter determines the
  // (distance*density) value represented by the texel at the center of
  // the fog table.  The fog calculation is:
  // alpha = 1.0 - exp( -(density*distance) / CS_FOGTABLE_MEDIANDISTANCE)
  #define CS_FOGTABLE_MEDIANDISTANCE 10.0f
  #define CS_FOGTABLE_MAXDISTANCE (CS_FOGTABLE_MEDIANDISTANCE * 2.0f)
  #define CS_FOGTABLE_DISTANCESCALE (1.0f / CS_FOGTABLE_MAXDISTANCE)

  unsigned char *transientfogdata = new unsigned char[CS_FOGTABLE_SIZE * 4];
  for (unsigned int fogindex = 0; fogindex < CS_FOGTABLE_SIZE; fogindex++)
  {
    transientfogdata[fogindex * 4 + 0] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 1] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 2] = (unsigned char) 255;
    double fogalpha = (256 * (1.0 - exp (-float (fogindex)
      * CS_FOGTABLE_MAXDISTANCE / CS_FOGTABLE_SIZE)));
    transientfogdata[fogindex * 4 + 3] = (unsigned char) fogalpha;
  }
  transientfogdata[(CS_FOGTABLE_SIZE - 1) * 4 + 3] = 0;

  csRef<iImage> img = csPtr<iImage> (new csImageMemory (
    CS_FOGTABLE_SIZE, 1, transientfogdata, true, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  csRef<iTextureHandle> fogtex = txtmgr->RegisterTexture (
    img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);

  csRef<csShaderVariable> fogvar = csPtr<csShaderVariable>( new csShaderVariable(
    strings->Request ("standardtex fog")));
  fogvar->SetValue (fogtex);
  if (shadermgr)
    shadermgr->AddVariable(fogvar);

  {
    const int normalizeCubeSize = config->GetInt (
      "Video.Null3d.NormalizeCubeSize", 128);

    csRef<csShaderVariable> normvar = 
      csPtr<csShaderVariable> (new csShaderVariable (
      strings->Request ("standardtex normalization map")));
    csRef<iShaderVariableAccessor> normCube;
    normCube.AttachNew (new csNormalizationCubeAccessor (txtmgr, 
      normalizeCubeSize));
    normvar->SetAccessor (normCube);
    shadermgr->AddVariable(normvar);
  }


  #define CS_ATTTABLE_SIZE	  128
  #define CS_HALF_ATTTABLE_SIZE	  ((float)CS_ATTTABLE_SIZE/2.0f)

  csRGBpixel *attenuationdata = 
    new csRGBpixel[CS_ATTTABLE_SIZE * CS_ATTTABLE_SIZE * 4];
  csRGBpixel* data = attenuationdata;
  for (int y=0; y < CS_ATTTABLE_SIZE; y++)
  {
    for (int x=0; x < CS_ATTTABLE_SIZE; x++)
    {
      float yv = 3.0f * ((y + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
      float xv = 3.0f * ((x + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
      float i = exp (-0.7 * (xv*xv + yv*yv));
      unsigned char v = i>1.0f ? 255 : csQint (i*255.99f);
      (data++)->Set (v, v, v, v);
    }
  }

  img  = csPtr<iImage> (new csImageMemory (
    CS_ATTTABLE_SIZE, CS_ATTTABLE_SIZE, attenuationdata, true, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  csRef<iTextureHandle> atttex = txtmgr->RegisterTexture (
    img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);

  csRef<csShaderVariable> attvar = csPtr<csShaderVariable>( new csShaderVariable(
    strings->Request ("standardtex attenuation")));
  attvar->SetValue (atttex);
  if (shadermgr)
    shadermgr->AddVariable(attvar);

  return true;
}

void csNullGraphics3D::Close ()
{
  shadermgr = 0;

  if (G2D)
    G2D->Close ();
}

void csNullGraphics3D::SetRenderTarget (iTextureHandle*, bool)
{
  return;
}

iTextureHandle* csNullGraphics3D::GetRenderTarget () const
{
  return 0;
}

bool csNullGraphics3D::BeginDraw (int DrawFlags)
{
  if ((DrawFlags & (CSDRAW_3DGRAPHICS | CSDRAW_2DGRAPHICS))
   && (!(current_drawflags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  current_drawflags = DrawFlags;

  return true;
}

void csNullGraphics3D::FinishDraw ()
{
  if (current_drawflags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  
  current_drawflags = 0;
}

void csNullGraphics3D::Print (csRect const*area)
{
  if (bugplug)
    bugplug->ResetCounter ("Triangle Count");
  G2D->Print (area);
}

void csNullGraphics3D::SetClipper (iClipper2D* clipper, int cliptype)
{
  csNullGraphics3D::clipper = clipper;
  csNullGraphics3D::cliptype = cliptype;
  
}
iClipper2D* csNullGraphics3D::GetClipper ()
{
  return clipper;
}
int csNullGraphics3D::GetClipType () const
{
  return cliptype;
}
void csNullGraphics3D::SetNearPlane (const csPlane3& pl)
{
  do_near_plane = true;
  near_plane = pl;
}
void csNullGraphics3D::ResetNearPlane ()
{
  do_near_plane = false;
}
const csPlane3& csNullGraphics3D::GetNearPlane () const 
{
  return near_plane;
}
bool csNullGraphics3D::HasNearPlane () const
{
  return do_near_plane;
}

bool csNullGraphics3D::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{
  return false;
}

long csNullGraphics3D::GetRenderState (G3D_RENDERSTATEOPTION op) const
{
  return 0;
}

bool csNullGraphics3D::SetOption (const char*, const char*)
{
  return false;
}

void csNullGraphics3D::SetTextureState (int*, iTextureHandle**, int)
{
  return;
}

void csNullGraphics3D::DrawMesh (const csCoreRenderMesh* mymesh,
    const csRenderMeshModes& modes,
    const csArray<csShaderVariable*> &stacks)
{
  if (bugplug)
  {
    int num_tri = (mymesh->indexend-mymesh->indexstart);
    switch (mymesh->meshtype)
    {
      case CS_MESHTYPE_QUADS:
        num_tri /= 2;
        break;
      case CS_MESHTYPE_TRIANGLES:
        num_tri /= 3;
        break;
      case CS_MESHTYPE_TRIANGLESTRIP:
        num_tri -= 2;
        break;
      case CS_MESHTYPE_TRIANGLEFAN:
        num_tri -= 1;
        break;
      case CS_MESHTYPE_POINTS:
        break;
      case CS_MESHTYPE_POINT_SPRITES:
        break;
      case CS_MESHTYPE_LINES:
        num_tri /= 3;
        break;
      case CS_MESHTYPE_LINESTRIP:
        num_tri -= 2;
        break;
    }
    bugplug->AddCounter ("Triangle Count", num_tri);
    bugplug->AddCounter ("Mesh Count", 1);
  }  
}
void csNullGraphics3D::SetWriteMask (bool red, bool green, bool blue, bool alpha)
{
  red_mask = red;
  green_mask = green;
  blue_mask = blue;
  alpha_mask = alpha;
}
void csNullGraphics3D::GetWriteMask (bool& red, bool& green, bool& blue, bool& alpha) const
{
  red = red_mask;
  green = green_mask;
  blue = blue_mask;
  alpha = alpha_mask;
}
void csNullGraphics3D::EnableZOffset ()
{
 return;
}
void csNullGraphics3D::DisableZOffset ()
{
 return;
}
void csNullGraphics3D::SetShadowState (int state)
{
 return;
}

