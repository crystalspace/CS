/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "cssysdef.h"

#include "csqint.h"

#include "csgeom/math.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "csgeom/poly2d.h"
#include "csgeom/polyclip.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "cstool/rbuflock.h"
#include "csutil/cscolor.h"
#include "csutil/event.h"
#include "csutil/scfarray.h"
#include "csutil/scfstrset.h"
#include "csutil/sysfunc.h"
#include "csutil/eventnames.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/rendermesh.h"

#include "csplugincommon/shader/shaderplugin.h"

#include "sft3dcom.h"
#include "soft_txt.h"
#include "clip_znear.h"
#include "vertextransform.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

using namespace CS::PluginCommon::SoftShader;

///---------------------------------------------------------------------------

csSoftwareGraphics3DCommon::csSoftwareGraphics3DCommon (iBase* parent) :
  scfImplementationType (this, parent)
{
  scfiEventHandler = 0;
  texman = 0;
  partner = 0;
  cliptype = CS_CLIPPER_NONE;
  clipperDirty = true;
  do_near_plane = false;

  do_interlaced = oldIlaceMode = -1;
  do_smaller_rendering = false;
  smallerActive = false;
  smaller_buffer = 0;
  pixel_shift = 0;
  rstate_mipmap = 0;
  ilaceSaveBuf = 0;
  ilaceSaveBufSize = 0;

  z_buffer = 0;
  line_table = 0;

  Caps.CanClip = true;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.MaxAspectRatio = 32768;
  Caps.NeedsPO2Maps = true;
  Caps.SupportsPointSprites = false;
  Caps.DestinationAlpha = false;
  Caps.StencilShadows = false;

  width = height = -1;
  partner = 0;

  object_reg = 0;

  memset (activebuffers, 0, sizeof (activebuffers));
  memset (activeSoftTex, 0, sizeof (activeSoftTex));

  clipportal_dirty = true;
  clipportal_floating = 0;

  scrapIndicesSize = 0;
  scrapVerticesSize = 0;

  memset (processedColorsFlag, 0, sizeof (processedColorsFlag));
  memset (triDraw, 0, sizeof (triDraw));
  specifica = 0;
}

csSoftwareGraphics3DCommon::~csSoftwareGraphics3DCommon ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  Close ();
  if (partner) partner->DecRef ();
  clipper = 0;
  cliptype = CS_CLIPPER_NONE;
}

bool csSoftwareGraphics3DCommon::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
  {
    csEventID events[3] = { csevSystemOpen(object_reg), csevSystemClose(object_reg), CS_EVENTLIST_END };
    q->RegisterListener (scfiEventHandler, events);
  }

  strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  string_world2camera = strings->Request ("world2camera transform");
  string_indices = strings->Request ("indices");

  return true;
}

bool csSoftwareGraphics3DCommon::HandleEvent (iEvent& Event)
{
  if (Event.Name == csevSystemOpen(object_reg))
  {
    Open ();
    return true;
  }
  else if (Event.Name == csevSystemClose(object_reg))
  {
    Close ();
    return true;
  }
  return false;
}

void csSoftwareGraphics3DCommon::NewInitialize ()
{
  config.AddConfig(object_reg, "/config/soft3d.cfg");
  do_smaller_rendering = config->GetBool ("Video.Software.Smaller", false);
  mipmap_coef = config->GetFloat ("Video.Software.TextureManager.MipmapCoef", 1.3f);
  do_interlaced = config->GetBool ("Video.Software.Interlacing", false) ? 0 : -1;
}

void csSoftwareGraphics3DCommon::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.video.software", msg, arg);
  va_end (arg);
}

bool csSoftwareGraphics3DCommon::Open ()
{
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;
    return false;
  }

  pfmt = *G2D->GetPixelFormat ();

  if (pfmt.PixelBytes == 4)
    pixel_shift = 2;
  else if (pfmt.PixelBytes == 2)
    pixel_shift = 1;
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"8-bit palette mode no longer works in the software renderer!");
    return false;
  }
  pixelBGR = (pfmt.BlueMask > pfmt.RedMask);

  DrawMode = 0;
  SetDimensions (G2D->GetWidth (), G2D->GetHeight ());

  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (object_reg));

  shadermgr = csQueryRegistryOrLoad<iShaderManager> (object_reg,
    "crystalspace.graphics3d.shadermanager");
  if (!shadermgr) return false;

  SetupSpecifica();

  vertexTransform = new VertexTransform;

  csRef<iDefaultShader> shaderPlugin = csLoadPlugin<iDefaultShader> (
    plugin_mgr, "crystalspace.graphics3d.shader.software");
  if (!shaderPlugin.IsValid())
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Can't find default software shader plugin");
  }
  else
  {
    defaultRendererState = shaderPlugin->GetDefaultRenderer ();
    if (defaultRendererState.IsValid())
      defaultRenderer = scfQueryInterface<iScanlineRenderer> (
	defaultRendererState);
  }

  return true;
}

bool csSoftwareGraphics3DCommon::NewOpen ()
{
  // Create the texture manager
  texman = new csSoftwareTextureManager (object_reg, this, config);
  texman->SetPixelFormat (pfmt);

  //SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);

  return true;
}

bool csSoftwareGraphics3DCommon::SharedOpen ()
{
  pixel_shift = partner->pixel_shift;
  texman = partner->texman;
  return true;
}

void csSoftwareGraphics3DCommon::Close ()
{
  if ((width == height) && (width == -1))
    return;

  if (!partner)
  {
    texman->Clear();
    texman->DecRef(); texman = 0;
  }
  clipper = 0;
  cliptype = CS_CLIPPER_NONE;

  delete [] z_buffer; z_buffer = 0;
  delete [] smaller_buffer; smaller_buffer = 0;
  delete [] line_table; line_table = 0;
  for (size_t n = CS_MIXMODE_FACT_COUNT*CS_MIXMODE_FACT_COUNT; n-- > 0; )
    delete triDraw[n];
  memset (triDraw, 0, sizeof (triDraw));
  delete specifica; specifica = 0;
  delete[] ilaceSaveBuf; ilaceSaveBuf = 0;
  delete vertexTransform; vertexTransform = 0;

  G2D->Close ();
  width = height = -1;
}

void csSoftwareGraphics3DCommon::SetDimensions (int nwidth, int nheight)
{
  display_width = nwidth;
  display_height = nheight;
  width = nwidth;
  height = nheight;
  persp_center_x = width/2;
  persp_center_y = height/2;

  delete [] smaller_buffer;
  smaller_buffer = 0;
  if (do_smaller_rendering)
  {
    size_t smallSize = (width*height)/4 * pfmt.PixelBytes;
    smaller_buffer = new uint8 [smallSize];
    memset (smaller_buffer, 0, smallSize);
  }

  delete [] z_buffer;
  z_buffer = new uint32 [width*height];
  z_buf_size = sizeof (uint32)*width*height;

  delete [] line_table;
  line_table = new uint8* [height+1];
}

void csSoftwareGraphics3DCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  userClipper = clip;
  if (!userClipper) cliptype = CS_CLIPPER_NONE;
  csSoftwareGraphics3DCommon::cliptype = cliptype;
  clipperDirty = true;
  clipper = 0;
}

template<typename T, int pixelMask1, int pixelMask2>
class SmallBufferScaler
{
  static const T Mix2 (const T a, const T b)
  {
    const T a1 = a & pixelMask1;
    const T a2 = a & pixelMask2;
    const T b1 = b & pixelMask1;
    const T b2 = b & pixelMask2;
    return (((a1 >> 1) + (b1 >> 1)) & pixelMask1)
      | (((a2 >> 1) + (b2 >> 1)) & pixelMask2);
  }
  static const T Mix4 (const T a, const T b, const T c, const T d)
  {
    const T a1 = a & pixelMask1;
    const T a2 = a & pixelMask2;
    const T b1 = b & pixelMask1;
    const T b2 = b & pixelMask2;
    const T c1 = c & pixelMask1;
    const T c2 = c & pixelMask2;
    const T d1 = d & pixelMask1;
    const T d2 = d & pixelMask2;
    return (((a1 >> 2) + (b1 >> 2) + (c1 >> 2) + (d1 >> 2)) & pixelMask1)
      | (((a2 >> 2) + (b2 >> 2) + (c2 >> 2) + (d2 >> 2)) & pixelMask2);
  }
public:
  static void ScaleUp (int scrwidth, int scrheight, int areax, int areay, 
    int areaw, int areah, uint8** line_table, iGraphics2D* G2D)
  {
    const int width2 = scrwidth/2, height2 = scrheight/2;
    const int srcStartX = (areax+1)/2;
    const int srcEndXreal = ((areax+areaw) / 2);
    const int srcEndX = csMin (srcEndXreal, width2-1);
    const int srcStartY = (areay+1)/2;
    const int srcEndYreal = ((areay+areah) / 2);
    const int srcEndY = csMin (srcEndYreal, height2-1);
    /* Right/bottom edges need special handling since there is no right and/or
     * bottom pixel to use with interpolation. */
    const bool hasRightEdge = srcEndY < srcEndYreal;
    const bool hasBottomEdge = srcEndX < srcEndXreal;
    int x, y;
    for (y = srcStartY; y < srcEndY; y++)
    {
      T* src = (T*)line_table[y];
      T* dst1 = (T*)G2D->GetPixelAt (0, y+y);
      T* dst2 = (T*)G2D->GetPixelAt (0, y+y+1);
      for (x = srcStartX; x < srcEndX; x++)
      {
	const T s00 = src[x];
	const T s01 = src[x+width2];
	const T s10 = src[x+1];
	const T s11 = src[x+width2+1];

	const T p00 = s00;
	const T p10 = Mix2 (s00, s10);
	const T p01 = Mix2 (s00, s01);
	const T p11 = Mix4 (s00, s01, s10, s11);
	dst1[x+x] = p00;
	dst1[x+x+1] = p10;
	dst2[x+x] = p01;
	dst2[x+x+1] = p11;
      }
      if (hasRightEdge)
      {
        const T p00 = src[x];
        const T p01 = Mix2 (p00, src[x+width2]);
        dst1[x+x] = p00;
        dst1[x+x+1] = p00;
        dst2[x+x] = p01;
        dst2[x+x+1] = p01;
      }
    }

    if (hasBottomEdge)
    {
      T* src = (T*)line_table[y];
      T* dst1 = (T*)G2D->GetPixelAt (0, y+y);
      T* dst2 = (T*)G2D->GetPixelAt (0, y+y+1);
      for (x = srcStartX; x < srcEndX; x++)
      {
        const T p00 = src[x];
        const T p10 = Mix2 (p00, src[x+1]);
        dst1[x+x] = p00;
        dst1[x+x+1] = p10;
        dst2[x+x] = p00;
        dst2[x+x+1] = p10;
      }
      if (hasRightEdge)
      {
        const T p00 = src[x];
        dst1[x+x] = p00;
        dst1[x+x+1] = p00;
        dst2[x+x] = p00;
        dst2[x+x+1] = p00;
      }
    }
  }
};

void csSoftwareGraphics3DCommon::FlushSmallBufferToScreen()
{
  csRect clipRect;
  G2D->GetClipRect (clipRect.xmin, clipRect.ymin, clipRect.xmax, clipRect.ymax);
  switch (pfmt.PixelBytes)
  {
    case 2:
      if (pfmt.GreenBits == 5)
	SmallBufferScaler<uint16, 0x781e, 0x03c0>::ScaleUp (width, height, 
          clipRect.xmin, clipRect.ymin, clipRect.Width(), clipRect.Height(), line_table, G2D);
      else
	SmallBufferScaler<uint16, 0xf01e, 0x07c0>::ScaleUp (width, height, 
          clipRect.xmin, clipRect.ymin, clipRect.Width(), clipRect.Height(), line_table, G2D);
      break;
    case 4:
      SmallBufferScaler<uint32, 0xff00ff00, 0x00ff00ff>::ScaleUp (width, height, 
        clipRect.xmin, clipRect.ymin, clipRect.Width(), clipRect.Height(), line_table, G2D);
      break;
  }
}

void csSoftwareGraphics3DCommon::SetupClipper()
{
  csRect clipBox;
  G2D->GetClipRect (clipBox.xmin, clipBox.ymin, clipBox.xmax, clipBox.ymax);
  if (!clipperDirty && (clipBox == lastClipBox)) return;
  lastClipBox = clipBox;

  int t = height - clipBox.ymin;
  clipBox.ymin = height - clipBox.ymax;
  clipBox.ymax = t;

  if (userClipper.IsValid())
  {
    csBoxClipper boxClip (clipBox);
    const size_t vertCount = userClipper->GetVertexCount();
    size_t outCount = vertCount + 4;
    
    CS_ALLOC_STACK_ARRAY(csVector2, clippedPoly, outCount);
    csBox2 bbox (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
      CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE);
    uint8 result = boxClip.Clip (userClipper->GetClipPoly(), vertCount, 
      clippedPoly, outCount, bbox);
    switch (result)
    {
      default:
      case CS_CLIP_OUTSIDE:
        clipper = 0;
        break;
      case CS_CLIP_INSIDE:
        clipper = userClipper;
        break;
      case CS_CLIP_CLIPPED:
        if (userClipper->GetClipperType() == iClipper2D::clipperBox)
        {
          clipper.AttachNew (new csBoxClipper (bbox));
        }
        else
        {
          clipper.AttachNew (new csPolygonClipper (clippedPoly, outCount, false, true));
        }
        break;
    }
  }
  else
  {
    clipper.AttachNew (new csBoxClipper (clipBox));
  }

  clipperDirty = false;
}

#define CSDRAW_MASK2D3D	  (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS)

bool csSoftwareGraphics3DCommon::BeginDraw (int DrawFlags)
{
  clipportal_dirty = true;
  clipportal_floating = 0;
  CS_ASSERT (clipportal_stack.Length () == 0);

  if ((G2D->GetWidth() != display_width) ||
      (G2D->GetHeight() != display_height))
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());

  // if 2D graphics is not locked, lock it
  if ((DrawFlags & CSDRAW_MASK2D3D) && (!(DrawMode & CSDRAW_MASK2D3D)))
  {
    if (!G2D->BeginDraw())
      return false;
  }
  bool wasSmallerActive = smallerActive;
  smallerActive = do_smaller_rendering 
    && ((DrawFlags & CSDRAW_MASK2D3D) == CSDRAW_3DGRAPHICS)
    && !(render_target && !rt_onscreen);
  ilaceActive = ((DrawFlags & CSDRAW_MASK2D3D) == CSDRAW_3DGRAPHICS);

  if (wasSmallerActive && !smallerActive)
  {
    FlushSmallBufferToScreen();
  }

  int i;
  // Initialize the linetable.
  if (smallerActive)
  {
    for (i = 0 ; i < height/2; i++)
      line_table[i] = smaller_buffer + ((i*width/2)*pfmt.PixelBytes);
  }
  else
  {
    for (i = 0 ; i < height; i++)
      line_table[i] = G2D->GetPixelAt (0, i);
  }
  
  polyrast_ZFill.Init (pfmt, width, height, z_buffer, line_table, 
    do_interlaced);

  if (render_target)
  {
    int txt_w, txt_h;
    render_target->GetRendererDimensions (txt_w, txt_h);

    if (do_interlaced != -1)
    {
      const size_t rowSize = txt_w * pfmt.PixelBytes;
      const size_t bufSize = txt_h * rowSize;
      if (ilaceSaveBufSize < bufSize)
      {
        ilaceSaveBufSize = bufSize;
        delete[] ilaceSaveBuf;
        ilaceSaveBuf = new uint8[ilaceSaveBufSize];
      }
      
      uint8* dest = ilaceSaveBuf;
      for (int i = 0; i < txt_h; i++)
      {
        memcpy (dest, line_table[i], rowSize);
        dest += rowSize;
      }
      
      oldIlaceMode = do_interlaced;
      do_interlaced = -1;
      ilaceRestore = true;
    }
    else
      ilaceRestore = false;

    if (!rt_onscreen)
    {
      int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
      csSoftwareTexture *tex_0 = tex_mm->GetTexture (0);
      uint32* bitmap = tex_0->bitmap;
      specifica->BlitTextureToScreen (line_table, txt_w, txt_h, 
	width, height, bitmap);
      rt_onscreen = true;
    }
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
    memset (z_buffer, 0, z_buf_size);

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;
  return true;
}

void csSoftwareGraphics3DCommon::Print (csRect const* area)
{
  G2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
}

void csSoftwareGraphics3DCommon::FinishDraw ()
{
  if (smallerActive)
    FlushSmallBufferToScreen();

  if (render_target)
  {
    if (rt_onscreen)
    {
      if (smallerActive)
      {
	// Need full line table since we copy from there.
	for (int i = 0 ; i < height; i++)
	  line_table[i] = G2D->GetPixelAt (0, i);
      }
      rt_onscreen = false;
      int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
      csSoftwareTexture *tex_0 = tex_mm->GetTexture (0);
      uint32* bitmap = tex_0->bitmap;
      specifica->BlitScreenToTexture (line_table, txt_w, txt_h, 
	width, height, bitmap);
      
      if (ilaceRestore)
      {
        const size_t rowSize = txt_w * pfmt.PixelBytes;
        uint8* src = ilaceSaveBuf;
        for (int i = 0; i < txt_h; i++)
        {
          memcpy (line_table[i], src, rowSize);
          src += rowSize;
        }
      }
    
      SetRenderTarget (0, false, 0);
      if (oldIlaceMode != -1) do_interlaced = oldIlaceMode;
    }
  }

  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;
}

#define SMALL_D 0.01

/**
 * The engine often generates "empty" polygons, for example
 * (2, 2) - (317,2) - (317,2) - (2, 2)
 * To avoid too much computations, DrawPolygon detects such polygons by
 * counting the number of "real" vertices (i.e. the number of vertices,
 * distance between which is bigger that some amount). The "right" formula
 * for distance is sqrt(dX^2 + dY^2) but to avoid root and multiply
 * DrawPolygon checks abs(dX) + abs(dY). This is enough.
 */
#define VERTEX_NEAR_THRESHOLD   0.001

void csSoftwareGraphics3DCommon::OpenPortal (size_t numVertices, 
				 const csVector2* vertices,
				 const csPlane3& normal,
				 csFlags flags)
{
  csClipPortal* cp = new csClipPortal ();
  cp->poly = new csVector2[numVertices];
  memcpy (cp->poly, vertices, numVertices * sizeof (csVector2));
  cp->num_poly = (int)numVertices;
  cp->normal = normal;
  cp->flags = flags;
  clipportal_stack.Push (cp);
  clipportal_dirty = true;

  // If we already have a floating portal then we increase the
  // number. Otherwise we start at one.
  if (clipportal_floating)
    clipportal_floating++;
  else if (flags.Check(CS_OPENPORTAL_FLOAT))
    clipportal_floating = 1;
}

void csSoftwareGraphics3DCommon::ClosePortal ()
{
  if (clipportal_stack.Length () <= 0) return;
  csClipPortal* cp = clipportal_stack.Pop ();

  if (cp->flags.Check(CS_OPENPORTAL_ZFILL))
  {
    CS_ALLOC_STACK_ARRAY(csVector3, vertices, cp->num_poly);
    for (int v = 0; v < cp->num_poly; v++)
    {
      vertices[v].Set (cp->poly[v].x, cp->poly[v].y, 1.0f/Z_NEAR);
    }
    SLLogic_ZFill sll;
    polyrast_ZFill.DrawPolygon (cp->num_poly, vertices, sll);
  }

  delete cp;
  clipportal_dirty = true;
  if (clipportal_floating > 0)
    clipportal_floating--;
}

void csSoftwareGraphics3DCommon::ClearCache()
{
}

void csSoftwareGraphics3DCommon::RemoveFromCache (
	      iRendererLightmap* /*rlm*/)
{
}

void csSoftwareGraphics3DCommon::DumpCache()
{
}

void csSoftwareGraphics3DCommon::DrawLine (const csVector3& v1,
	const csVector3& v2, float fov, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x1 = t*(x2-x1)+x1;
    y1 = t*(y2-y1)+y1;
    z1 = SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x2 = t*(x2-x1)+x1;
    y2 = t*(y2-y1)+y1;
    z2 = SMALL_Z;
  }

  float iz1 = fov/z1;
  int px1 = csQint (x1 * iz1 + (width/2));
  int py1 = height - csQint (y1 * iz1 + (height/2));
  float iz2 = fov/z2;
  int px2 = csQint (x2 * iz2 + (width/2));
  int py2 = height - csQint (y2 * iz2 + (height/2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

float csSoftwareGraphics3DCommon::GetZBuffValue (int x, int y)
{
  unsigned long zbf = z_buffer [x + y * width];
  if (!zbf) return 1000000000.;
  return 16777216.0 / float (zbf);
}

void csSoftwareGraphics3DCommon::SetRenderTarget (iTextureHandle* handle,
	bool persistent,
	int subtexture)
{
  render_target = handle;
  rt_onscreen = !persistent;
  rt_cliprectset = false;

  if (handle)
  {
    int txt_w, txt_h;
    render_target->GetRendererDimensions (txt_w, txt_h);
    GetDriver2D()->PerformExtension ("vp_set", txt_w, txt_h);
    csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	      render_target->GetPrivateObject ();
    // We don't generate mipmaps or so...
    tex_mm->flags |= CS_TEXTURE_NOMIPMAPS;
    tex_mm->DeleteMipmaps ();

    GetDriver2D()->GetClipRect (rt_old_minx, rt_old_miny, 
      rt_old_maxx, rt_old_maxy);
    if ((rt_old_minx != 0) && (rt_old_miny != 0)
      && (rt_old_maxx != txt_w) && (rt_old_maxy != txt_h))
    {
      GetDriver2D()->SetClipRect (0, 0, txt_w, txt_h);
    }

    SetDimensions (txt_w, txt_h);
  }
  else
  {
    GetDriver2D()->PerformExtension ("vp_reset");
    GetDriver2D()->SetClipRect (rt_old_minx, rt_old_miny, 
      rt_old_maxx, rt_old_maxy);

    SetDimensions (G2D->GetWidth(), G2D->GetHeight());
  }
}

void csSoftwareGraphics3DCommon::DrawSimpleMesh (const csSimpleRenderMesh &mesh,
						 uint flags)
{
  csRef<csRenderBufferHolder> scrapBufferHolder;
  scrapBufferHolder.AttachNew (new csRenderBufferHolder);

  uint indexCount = mesh.indices ? mesh.indexCount : mesh.vertexCount;
  if (scrapIndicesSize < indexCount)
  {
    scrapIndices = csRenderBuffer::CreateIndexRenderBuffer (indexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_UNSIGNED_INT, 0, mesh.vertexCount - 1);
    scrapIndicesSize = indexCount;
  }
  if (scrapVerticesSize < mesh.vertexCount)
  {
    scrapVertices = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 3);
    scrapTexcoords = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 2);
    scrapColors = csRenderBuffer::CreateRenderBuffer (
      mesh.vertexCount,
      CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 4);

    scrapVerticesSize = mesh.vertexCount;
  }

  csShaderVariable* sv;
  sv = scrapContext.GetVariableAdd (strings->Request ("indices"));
  if (mesh.indices)
  {
    scrapIndices->CopyInto (mesh.indices, mesh.indexCount);
  }
  else
  {
    csRenderBufferLock<uint, iRenderBuffer*> indexLock (scrapIndices);
    for (uint i = 0; i < mesh.vertexCount; i++)
      indexLock[(size_t)i] = i;
  }
  sv->SetValue (scrapIndices);
  scrapBufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, scrapIndices);

  sv = scrapContext.GetVariableAdd (strings->Request ("vertices"));
  if (mesh.vertices)
  {
    scrapVertices->CopyInto (mesh.vertices, mesh.vertexCount);
    ActivateBuffer (CS_VATTRIB_POSITION, scrapVertices);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, scrapVertices);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_POSITION);
  }
  sv = scrapContext.GetVariableAdd (strings->Request ("texture coordinates"));
  if (mesh.texcoords)
  {
    scrapTexcoords->CopyInto (mesh.texcoords, mesh.vertexCount);
    ActivateBuffer (CS_VATTRIB_TEXCOORD, scrapTexcoords);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, scrapTexcoords);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_TEXCOORD);
  }
  sv = scrapContext.GetVariableAdd (strings->Request ("colors"));
  if (mesh.colors)
  {
    scrapColors->CopyInto (mesh.colors, mesh.vertexCount);
    scrapBufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, scrapColors);
    ActivateBuffer (CS_VATTRIB_COLOR, scrapColors);
  }
  else
  {
    DeactivateBuffer (CS_VATTRIB_COLOR);
  }

  if (mesh.texture)
    ActivateTexture (mesh.texture);
  else
    DeactivateTexture ();

  csRenderMesh rmesh;
  //rmesh.z_buf_mode = mesh.z_buf_mode;
  rmesh.mixmode = mesh.mixmode;
  rmesh.clip_portal = 0;
  rmesh.clip_plane = 0;
  rmesh.clip_z_plane = 0;
  rmesh.do_mirror = false;
  rmesh.meshtype = mesh.meshtype;
  rmesh.indexstart = 0;
  rmesh.indexend = indexCount;
  rmesh.variablecontext = &scrapContext;
  rmesh.buffers = scrapBufferHolder;

  if (flags & csSimpleMeshScreenspace)
  {
    csReversibleTransform camtrans;

    camtrans.SetO2T (
      csMatrix3 (1.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, 0.0f, 1.0f));
    camtrans.SetO2TTranslation (csVector3 (
      float (persp_center_x), 
      float (GetHeight() - persp_center_y), -aspect));

    SetWorldToCamera (camtrans.GetInverse ());
  }

  rmesh.object2world = mesh.object2world;

  csRef<iShaderVarStack> stacks;
  stacks.AttachNew (new scfArray<iShaderVarStack>);
  shadermgr->PushVariables (stacks);
  scrapContext.PushVariables (stacks);

  if (mesh.alphaType.autoAlphaMode)
  {
    csAlphaMode::AlphaType autoMode = csAlphaMode::alphaNone;

    iTextureHandle* tex = 0;

    csShaderVariable* texVar =
      csGetShaderVariableFromStack (stacks, mesh.alphaType.autoModeTexture);

    if (texVar)
      texVar->GetValue (tex);

    if (tex == 0)
      tex = mesh.texture;
    if (tex != 0)
      autoMode = tex->GetAlphaType ();

    rmesh.alphaType = autoMode;
  }
  else
  {
    rmesh.alphaType = mesh.alphaType.alphaType;
  }

  SetZMode (mesh.z_buf_mode);
  DrawMesh (&rmesh, rmesh, stacks);
}

static csZBufMode GetZModePass2 (csZBufMode mode)
{
  switch (mode)
  {
    case CS_ZBUF_NONE:
    case CS_ZBUF_TEST:
    case CS_ZBUF_EQUAL:
      return mode;
    case CS_ZBUF_FILL:
    case CS_ZBUF_USE:
      return CS_ZBUF_EQUAL;
    default:
      return CS_ZBUF_NONE;
  }
}

static iRenderBuffer* ColorFixup (iRenderBuffer* srcBuffer, 
				  csRef<csRenderBuffer>& dstBuffer,
				  bool swapRB, bool doAlphaScale,
				  float alphaScale)
{
  CS_ASSERT(srcBuffer->GetComponentType() == CS_BUFCOMP_FLOAT);
  const size_t elemCount = srcBuffer->GetElementCount();
  const uint comps = doAlphaScale ? 4 : 3;
  const size_t srcComps = srcBuffer->GetComponentCount();
  if (!dstBuffer.IsValid()
    || (dstBuffer->GetSize() < (elemCount * comps * sizeof (float))))
  {
    dstBuffer = csRenderBuffer::CreateRenderBuffer (elemCount,
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, comps);
  }
  else
  {
    dstBuffer->SetRenderBufferProperties (elemCount, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, comps);
  }
  csRenderBufferLock<csVector4, iRenderBuffer*> src (srcBuffer, 
    CS_BUF_LOCK_READ);
  csRenderBufferLock<csVector4, iRenderBuffer*> dst (dstBuffer,
    CS_BUF_LOCK_NORMAL);
  const float defComponentsF[] = {0.0f, 0.0f, 0.0f, 1.0f};

  for (size_t e = 0; e < elemCount; e++)
  {
    const csVector4& s = src[e];
    float sv[4];
    for (int c = 0; c < 4; c++)
    {
      sv[c] = c < (int)srcComps ? s[c] : defComponentsF[c];
    }
    csVector4& d = dst[e];
    d.x = swapRB ? sv[2] : sv[0];
    d.y = sv[1];
    d.z = swapRB ? sv[0] : sv[2];
    if (doAlphaScale)
      d.w = sv[3] * alphaScale;
  }
  return dstBuffer;
}

void csSoftwareGraphics3DCommon::DrawMesh (const csCoreRenderMesh* mesh,
    const csRenderMeshModes& modes,
    const iShaderVarStack* stacks)
{
  ScanlineRendererHelper aNameThatDoesNotReallyMatter (this);
  if (!scanlineRenderer) return;

  SetupClipper();
  if (!clipper) return;

  csRenderMeshModes usedModes (modes);
  if (zBufMode == CS_ZBUF_MESH2)
    usedModes.z_buf_mode = GetZModePass2 (usedModes.z_buf_mode);
  else if (zBufMode != CS_ZBUF_MESH)
    usedModes.z_buf_mode = zBufMode;
  switch (modes.mixmode & CS_MIXMODE_TYPE_MASK)
  {
    case CS_MIXMODE_TYPE_BLENDOP:
      usedModes.mixmode = modes.mixmode;
      break;
    case CS_MIXMODE_TYPE_AUTO:
    default:
      if (usedModes.alphaType == csAlphaMode::alphaSmooth)
	usedModes.mixmode = CS_MIXMODE_BLEND(SRCALPHA, SRCALPHA_INV);
      else
	usedModes.mixmode = CS_MIXMODE_BLEND(ONE, ZERO);
      break;
  }

  BuffersMask buffersMask = 0;
  for (size_t b = 0; b < maxBuffers; b++)
  {
    if (activebuffers[b] == 0) continue;
    buffersMask |= 1 << b;
  }

  iScanlineRenderer::RenderInfoMesh renderInfoMesh;
  TexturesMask availableTextures = 0;
  for (size_t t = 0; t < activeTextureCount; t++)
  {
    if (activeSoftTex[t])
      availableTextures |= (1 << t);
  }
  if (!scanlineRenderer->SetupMesh (availableTextures, buffersMask, usedModes, 
    (CS_MIXMODE_BLENDOP_SRC(usedModes.mixmode) != CS_MIXMODE_FACT_ZERO)
    || (CS_MIXMODE_BLENDOP_DST(usedModes.mixmode) == CS_MIXMODE_FACT_SRCCOLOR)
    || (CS_MIXMODE_BLENDOP_DST(usedModes.mixmode) == CS_MIXMODE_FACT_SRCCOLOR_INV)
    || (CS_MIXMODE_BLENDOP_DST(usedModes.mixmode) == CS_MIXMODE_FACT_SRCALPHA)
    || (CS_MIXMODE_BLENDOP_DST(usedModes.mixmode) == CS_MIXMODE_FACT_SRCALPHA_INV),
    renderInfoMesh)) 
    return;
  
  iRenderBuffer* indexbuf = 
    (modes.buffers ? modes.buffers->GetRenderBuffer(CS_BUFFER_INDEX) : 0);

  if (!indexbuf)
  {
    csShaderVariable* indexBufSV = csGetShaderVariableFromStack (stacks, string_indices);
    CS_ASSERT (indexBufSV != 0);
    indexBufSV->GetValue (indexbuf);
  }
  CS_ASSERT(indexbuf);

  csRenderBufferLock<uint8, iRenderBuffer*> indices (indexbuf, CS_BUF_LOCK_READ);

  size_t rangeStart = indexbuf->GetRangeStart();
  size_t rangeEnd = indexbuf->GetRangeEnd();

  // @@@ Hm... color processing, probably *after* TransformVertices()...
  const bool alphaScale = ((modes.mixmode & CS_FX_MASK_ALPHA) != 0);
  if (pixelBGR || alphaScale)
  {
    const float alpha = 
      1.0f - ((modes.mixmode & CS_FX_MASK_ALPHA) / 255.0f);

    if ((activebuffers[VATTR_SPEC(PRIMARY_COLOR)] != 0)
      && (!processedColorsFlag[0]))
    {
      activebuffers[VATTR_SPEC(PRIMARY_COLOR)] = ColorFixup (
	activebuffers[VATTR_SPEC(PRIMARY_COLOR)], processedColors[0],
	pixelBGR, alphaScale, alpha);
      processedColorsFlag[0] = true;
    }

    if ((activebuffers[VATTR_SPEC(SECONDARY_COLOR)] != 0)
      && (!processedColorsFlag[1]))
    {
      activebuffers[VATTR_SPEC(SECONDARY_COLOR)] = ColorFixup (
	activebuffers[VATTR_SPEC(SECONDARY_COLOR)], processedColors[1],
	pixelBGR, alphaScale, alpha);
      processedColorsFlag[1] = true;
    }
  }

  VerticesLTN inBuffers;
  VerticesLTN outBuffers;
  size_t useCompNum[activeBufferCount];
  size_t* compPtr = useCompNum;
  const size_t* compNum = renderInfoMesh.bufferComps;
  for (size_t b = 0; b < activeBufferCount; b++)
  {
    if (renderInfoMesh.desiredBuffers & (1 << b))
    {
      *compPtr++ = *compNum++;
    }
    else if (b == CS_SOFT3D_VA_BUFINDEX(POSITION))
    {
      *compPtr++ = 3;
    }
  }
  uint useBuffers = 
    renderInfoMesh.desiredBuffers | CS_SOFT3D_BUFFERFLAG(POSITION);
  inBuffers.Linearize (activebuffers, useCompNum, useBuffers);
  outBuffers.SetupEmpty (useCompNum, useBuffers);

  csTriangle* triangles;
  size_t trianglesCount;
  vertexTransform->TransformVertices (mesh->object2world, w2c,
    indexbuf, mesh->meshtype, inBuffers, 
    mesh->indexstart, mesh->indexend, triangles, trianglesCount,
    outBuffers, rangeStart, rangeEnd);

  const csRenderMeshType meshtype = mesh->meshtype;
  if ((meshtype >= CS_MESHTYPE_TRIANGLES) 
    && (meshtype <= CS_MESHTYPE_TRIANGLEFAN))
  {
    const uint triDrawIndex = 
      CS_MIXMODE_BLENDOP_SRC(usedModes.mixmode)*CS_MIXMODE_FACT_COUNT
      + CS_MIXMODE_BLENDOP_DST(usedModes.mixmode);

    triDraw[triDrawIndex]->DrawMesh (outBuffers, rangeStart, rangeEnd, mesh, 
      renderInfoMesh, triangles, trianglesCount);
  }
}

void csSoftwareGraphics3DCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh,
  int tx, int ty, int tw, int th, uint8 Alpha)
{
  specifica->DrawPixmap (this, hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
}

}
CS_PLUGIN_NAMESPACE_END(Soft3D)
