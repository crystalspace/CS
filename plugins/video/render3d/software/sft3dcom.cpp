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
#include "csutil/scfstrset.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/rendermesh.h"

#include "sft3dcom.h"
#include "soft_txt.h"

#include "clipper.h"
#include "clip_znear.h"
#include "clip_iclipper.h"
#include "scan_pix.h"
#include "tridraw.h"

namespace cspluginSoft3d
{

  using namespace CrystalSpace::SoftShader;

///---------------------------------------------------------------------------

csSoftwareGraphics3DCommon::csSoftwareGraphics3DCommon (iBase* parent) :
  scfImplementationType (this, parent)
{
  scfiEventHandler = 0;
  texman = 0;
  partner = 0;
  clipper = 0;
  cliptype = CS_CLIPPER_NONE;
  do_near_plane = false;

  do_interlaced = -1;
  do_smaller_rendering = false;
  smaller_buffer = 0;
  pixel_shift = 0;
  rstate_mipmap = 0;

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
  if (clipper)
  {
    clipper->DecRef ();
    clipper = 0;
    cliptype = CS_CLIPPER_NONE;
  }
}

bool csSoftwareGraphics3DCommon::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  string_world2camera = strings->Request ("world2camera transform");
  string_indices = strings->Request ("indices");

  return true;
}

bool csSoftwareGraphics3DCommon::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (csCommandEventHelper::GetCode(&Event))
    {
      case cscmdSystemOpen:
      {
        Open ();
        return true;
      }
      case cscmdSystemClose:
      {
        Close ();
        return true;
      }
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
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.video.software", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
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
    CS_QUERY_REGISTRY (object_reg, iPluginManager));

  CS_QUERY_REGISTRY_PLUGIN(shadermgr, object_reg,
    "crystalspace.graphics3d.shadermanager", iShaderManager);

  if (pfmt.PixelBytes == 4)
  {
    if (((pfmt.BlueMask == 0x0000ff) || (pfmt.RedMask == 0x0000ff))
      && (pfmt.GreenMask == 0x00ff00)
      && ((pfmt.RedMask == 0xff0000) || (pfmt.BlueMask == 0xff0000)))
      TriDrawMatrixFiller<Pix_Fix<uint32, 24, 0xff,
					  16, 0xff,
					  8,  0xff,
					  0,  0xff> >::Fill (this, triDraw);
    else
      TriDrawMatrixFiller<Pix_Generic<uint32> >::Fill (this, triDraw);
  }
  else
  {
    if (((pfmt.RedMask == 0xf800) || (pfmt.BlueMask == 0xf800))
      && (pfmt.GreenMask == 0x07e0)
      && ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
      TriDrawMatrixFiller<Pix_Fix<uint16, 0,  0,
					  8,  0xf8,
					  3,  0xfc,
					  -3, 0xf8> >::Fill (this, triDraw);
    else if (((pfmt.RedMask == 0x7c00) || (pfmt.BlueMask == 0x7c00))
      && (pfmt.GreenMask == 0x03e0)
      && ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
      TriDrawMatrixFiller<Pix_Fix<uint16, 0,  0,
					  7,  0xf8,
					  2,  0xf8,
					  -3, 0xf8> >::Fill (this, triDraw);
    else
      TriDrawMatrixFiller<Pix_Generic<uint16> >::Fill (this, triDraw);;
  }

  return true;
}

bool csSoftwareGraphics3DCommon::NewOpen ()
{
  // Create the texture manager
  texman = new csSoftwareTextureManager (object_reg, this, config);
  texman->SetPixelFormat (pfmt);

  //SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, do_interlaced == 0);

  polyrast_ZFill.Init (pfmt, width, height, z_buffer, line_table);

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
  if (clipper)
  {
    clipper->DecRef ();
    clipper = 0;
    cliptype = CS_CLIPPER_NONE;
  }

  delete [] z_buffer; z_buffer = 0;
  delete [] smaller_buffer; smaller_buffer = 0;
  delete [] line_table; line_table = 0;
  for (size_t n = CS_MIXMODE_FACT_COUNT*CS_MIXMODE_FACT_COUNT; n-- > 0; )
    delete triDraw[n];
  memset (triDraw, 0, sizeof (triDraw));

  G2D->Close ();
  width = height = -1;
}

void csSoftwareGraphics3DCommon::SetDimensions (int nwidth, int nheight)
{
  display_width = nwidth;
  display_height = nheight;
  if (do_smaller_rendering)
  {
    width = nwidth/2;
    height = nheight/2;
  }
  else
  {
    width = nwidth;
    height = nheight;
  }
  width2 = width/2;
  height2 = height/2;

  delete [] smaller_buffer;
  smaller_buffer = 0;
  if (do_smaller_rendering)
  {
    smaller_buffer = new uint8 [(width*height) * pfmt.PixelBytes];
  }

  delete [] z_buffer;
  z_buffer = new uint32 [width*height];
  z_buf_size = sizeof (uint32)*width*height;

  delete [] line_table;
  line_table = new uint8* [height+1];
}

void csSoftwareGraphics3DCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csSoftwareGraphics3DCommon::cliptype = cliptype;
}

bool csSoftwareGraphics3DCommon::BeginDraw (int DrawFlags)
{
  clipportal_dirty = true;
  clipportal_floating = 0;
  CS_ASSERT (clipportal_stack.Length () == 0);

  if ((G2D->GetWidth() != display_width) ||
      (G2D->GetHeight() != display_height))
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());

  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;
  }

  // Initialize the line table.
  int i;
  for (i = 0 ; i < height ; i++)
    if (do_smaller_rendering)
      line_table[i] = smaller_buffer + ((i*width)*pfmt.PixelBytes);
    else
      line_table[i] = G2D->GetPixelAt (0, i);

  if (render_target)
  {
    int txt_w, txt_h;
    render_target->GetRendererDimensions (txt_w, txt_h);
    if (!rt_cliprectset)
    {
      G2D->GetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
      G2D->SetClipRect (-1, -1, txt_w+1, txt_h+1);
      rt_cliprectset = true;
    }

    if (!rt_onscreen)
    {
      /*int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();*/
      //csSoftwareTexture *tex_0 = (csSoftwareTexture*)(tex_mm->get_texture (0));
      //int x, y;
      //uint32* bitmap = tex_0->bitmap;
      /* @@@ FIXME
      switch (pfmt.PixelBytes)
      {
	case 2:
	  {
	    uint16* pal2glob = (uint16*)(tex_mm->GetPaletteToGlobal ());
	    for (y = txt_h-1 ; y >= 0 ; y--)
	    {
              uint16* d = (uint16*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
	        uint8 pix = *bitmap++;
		*d++ = pal2glob[pix];
	      }
	    }
	  }
	  break;
	case 4:
	  {
	    uint32* pal2glob = (uint32*)(tex_mm->GetPaletteToGlobal ());
	    for (y = txt_h-1 ; y >= 0 ; y--)
	    {
              uint32* d = (uint32*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
	        uint8 pix = *bitmap++;
		*d++ = pal2glob[pix];
	      }
	    }
	  }
	  break;
      }*/
      rt_onscreen = true;
    }
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
    memset (z_buffer, 0, z_buf_size);

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

if (DrawMode & CSDRAW_3DGRAPHICS)
  {
    // Finished 3D drawing. If we are simulating the flush output to real frame buffer.
    if (do_smaller_rendering)
    {
      int x, y;
      switch (pfmt.PixelBytes)
      {
        case 2:
          if (pfmt.GreenBits == 5)
            for (y = 0 ; y < height ; y++)
            {
              uint16* src = (uint16*)line_table[y];
              uint16* dst1 = (uint16*)G2D->GetPixelAt (0, y+y);
              uint16* dst2 = (uint16*)G2D->GetPixelAt (0, y+y+1);
              for (x = 0 ; x < width ; x++)
              {
                dst1[x+x] = src[x];
                dst1[x+x+1] = ((src[x]&0x7bde)>>1) + ((src[x+1]&0x7bde)>>1);
                dst2[x+x] = ((src[x]&0x7bde)>>1) + ((src[x+width]&0x7bde)>>1);
                dst2[x+x+1] = ((dst1[x+x+1]&0x7bde)>>1) + ((dst2[x+x]&0x7bde)>>1);
              }
            }
          else
            for (y = 0 ; y < height ; y++)
            {
              uint16* src = (uint16*)line_table[y];
              uint16* dst1 = (uint16*)G2D->GetPixelAt (0, y+y);
              uint16* dst2 = (uint16*)G2D->GetPixelAt (0, y+y+1);
              for (x = 0 ; x < width ; x++)
              {
                dst1[x+x] = src[x];
                dst1[x+x+1] = ((src[x]&0xf7de)>>1) + ((src[x+1]&0xf7de)>>1);
                dst2[x+x] = ((src[x]&0xf7de)>>1) + ((src[x+width]&0xf7de)>>1);
                dst2[x+x+1] = ((dst1[x+x+1]&0xf7de)>>1) + ((dst2[x+x]&0xf7de)>>1);
              }
            }
          break;
        case 4:
          for (y = 0 ; y < height ; y++)
          {
            uint32* src = (uint32*)line_table[y];
            uint32* dst1 = (uint32*)G2D->GetPixelAt (0, y+y);
            uint32* dst2 = (uint32*)G2D->GetPixelAt (0, y+y+1);
            for (x = 0 ; x < width ; x++)
            {
              dst1[x+x] = src[x];
              dst1[x+x+1] = ((src[x]&0xfefefe)>>1) + ((src[x+1]&0xfefefe)>>1);
              dst2[x+x] = ((src[x]&0xfefefe)>>1) + ((src[x+width]&0xfefefe)>>1);
              dst2[x+x+1] = ((dst1[x+x+1]&0xfefefe)>>1) + ((dst2[x+x]&0xfefefe)>>1);
            }
          }
          break;
      }
    }
  }

  DrawMode = DrawFlags;
  return true;
}

void csSoftwareGraphics3DCommon::Print (csRect const* area)
{
  G2D->Print (area);
  if (do_interlaced != -1)
    do_interlaced ^= 1;
  //if (tcache)
    //tcache->frameno++;
}


void csSoftwareGraphics3DCommon::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;

  if (render_target)
  {
    if (rt_cliprectset)
    {
      rt_cliprectset = false;
      G2D->SetClipRect (rt_old_minx, rt_old_miny, rt_old_maxx, rt_old_maxy);
    }

    if (rt_onscreen)
    {
      rt_onscreen = false;
      /*int txt_w, txt_h;
      render_target->GetRendererDimensions (txt_w, txt_h);
      csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();*/
      //tex_mm->DeleteMipmaps ();
      //csSoftwareTexture *tex_0 = (csSoftwareTexture*)(tex_mm->get_texture (0));
      //int x, y;
      //uint32* bitmap = tex_0->bitmap;
      /* @@@ FIXME
      switch (pfmt.PixelBytes)
      {
	case 2:
	  {
	    for (y = 0 ; y < txt_h ; y++)
	    {
              uint16* d = (uint16*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
		uint16 pix = *d++;
		uint8 pix8;
		pix8 = (((pix & pfmt.RedMask) >> pfmt.RedShift) >>
			(pfmt.RedBits - 3)) << 5;
		pix8 |= (((pix & pfmt.GreenMask) >> pfmt.GreenShift) >>
			(pfmt.GreenBits - 3)) << 2;
		pix8 |= (((pix & pfmt.BlueMask) >> pfmt.BlueShift) >>
			(pfmt.BlueBits - 2));
		*bitmap++ = pix8;
	      }
	    }
	  }
	  break;
	case 4:
	  {
	    for (y = 0 ; y < txt_h ; y++)
	    {
              uint32* d = (uint32*)line_table[y];
	      for (x = 0 ; x < txt_w ; x++)
	      {
		uint32 pix = *d++;
		uint8 pix8;
		pix8 = (((pix & pfmt.RedMask) >> pfmt.RedShift) >> 5) << 5;
		pix8 |= (((pix & pfmt.GreenMask) >> pfmt.GreenShift) >> 5) << 2;
		pix8 |= (((pix & pfmt.BlueMask) >> pfmt.BlueShift) >> 6);
		*bitmap++ = pix8;
	      }
	    }
	  }
	  break;
      }*/
    }
  }
  render_target = 0;
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
				 bool floating)
{
  csClipPortal* cp = new csClipPortal ();
  cp->poly = new csVector2[numVertices];
  memcpy (cp->poly, vertices, numVertices * sizeof (csVector2));
  cp->num_poly = (int)numVertices;
  cp->normal = normal;
  clipportal_stack.Push (cp);
  clipportal_dirty = true;

  // If we already have a floating portal then we increase the
  // number. Otherwise we start at one.
  if (clipportal_floating)
    clipportal_floating++;
  else if (floating)
    clipportal_floating = 1;
}

void csSoftwareGraphics3DCommon::ClosePortal (bool use_zfill_portal)
{
  if (clipportal_stack.Length () <= 0) return;
  csClipPortal* cp = clipportal_stack.Pop ();

  if (use_zfill_portal)
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
	bool persistent)
{
  render_target = handle;
  csSoftwareTextureHandle* tex_mm = (csSoftwareTextureHandle *)
	    render_target->GetPrivateObject ();
  // We don't generate mipmaps or so...
  tex_mm->flags |= CS_TEXTURE_NOMIPMAPS;

  rt_onscreen = !persistent;
  rt_cliprectset = false;
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
      CS_BUF_DYNAMIC, CS_BUFCOMP_UNSIGNED_INT, 0, 0);
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
    csRenderBufferLock<uint> indexLock (scrapIndices);
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

    const float vwf = (float)(width);
    const float vhf = (float)(height);

    camtrans.SetO2T (
      csMatrix3 (1.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, 0.0f, 1.0f));
    camtrans.SetO2TTranslation (csVector3 (
      vwf / 2.0f, vhf / 2.0f, -aspect));

    SetWorldToCamera (camtrans.GetInverse ());
  }

  rmesh.object2world = mesh.object2world;

  csShaderVarStack stacks;
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
    const csArray<csShaderVariable*> &stacks)
{
  if (!scanlineRenderer) return;

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

  uint32 *indices = (uint32*)indexbuf->Lock (CS_BUF_LOCK_READ);

  const csMatrix3& w2c_m = w2c.GetO2T();
  const csMatrix3& o2w_m = mesh->object2world.GetO2T();
  const csVector3& w2c_t = w2c.GetO2TTranslation();
  const csVector3& o2w_t = mesh->object2world.GetO2TTranslation();
  csReversibleTransform object2camera (
    o2w_m * w2c_m,
    w2c_t + w2c_m.GetTranspose()*o2w_t);

  const size_t rangeStart = indexbuf->GetRangeStart();
  const size_t rangeEnd = indexbuf->GetRangeEnd();

  if (!object2camera.IsIdentity())
  {
    if (!translatedVerts.IsValid()
      || (translatedVerts->GetElementCount() <= rangeEnd))
    {
      translatedVerts = csRenderBuffer::CreateRenderBuffer (
	rangeEnd + 1, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
    }

    iRenderBuffer* vbuf = activebuffers[VATTR_SPEC(POSITION)];
    csRenderBufferLock<csVector3, iRenderBuffer*> f1 (vbuf, CS_BUF_LOCK_READ);
    if (!f1) return; 
    csVector3* tr_verts = 
      (csVector3*)translatedVerts->Lock (CS_BUF_LOCK_NORMAL);

    size_t i;
    const size_t maxVert = csMin (rangeEnd, vbuf->GetElementCount());
    // Make sure we don't process too many vertices;
    for (i = rangeStart; i <= maxVert; i++)
    {
      tr_verts[i] = object2camera.This2Other (f1[i]);
    }

    translatedVerts->Release();

    activebuffers[VATTR_SPEC(POSITION)] = translatedVerts;
  }

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

  const csRenderMeshType meshtype = mesh->meshtype;
  if ((meshtype >= CS_MESHTYPE_TRIANGLES) 
    && (meshtype <= CS_MESHTYPE_TRIANGLEFAN))
  {
    uint32* tri = indices + mesh->indexstart;
    const uint32* triEnd = indices + mesh->indexend;

    const uint triDrawIndex = 
      CS_MIXMODE_BLENDOP_SRC(usedModes.mixmode)*CS_MIXMODE_FACT_COUNT
      + CS_MIXMODE_BLENDOP_DST(usedModes.mixmode);

    triDraw[triDrawIndex]->DrawMesh (activebuffers, rangeStart, rangeEnd, mesh, 
      renderInfoMesh, meshtype, tri, triEnd);
  }
}

} // namespace cspluginSoft3d
