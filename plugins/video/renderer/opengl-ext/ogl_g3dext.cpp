/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: ogl_g3dext.cpp
 *
 * DESCRIPTION:
 *  Base class of the OpenGL EXT renderer.  This implements the interface
 *  "iGraphics3d".
 *
 * LICENSE:
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * AUTHOR:
 *    Thomas H. Hendrick
 *	  Philipp R. Aumayr (HangMan)
 *
 * CVS/RCS ID:
 *    $Id$
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */

/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */

// Crystal Space Headers that define interfaces for us
#include "cssysdef.h"
#include "csutil/scf.h"
#include "cssys/sysfunc.h"
#include "csutil/cfgacc.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/vbufmgr.h"
#include "igeom/clip2d.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "csutil/util.h"

#include "ogl_g3dext.h"
#include "gl_clipperobj_ext.h"

/* -----------------------------------------------------------------
 * Preprocessor Defines
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Method Implementations
 * ----------------------------------------------------------------- */
SCF_IMPLEMENT_IBASE(csGraphics3DGLext)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
SCF_IMPLEMENT_IBASE_END

/* -----------------------------------------------------------------
 * Static Data Declarations
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Public Function Defintions
 * ----------------------------------------------------------------- */


// =================================================================
// CONSTRUCTOR: csGraphics3DGLext
//
// DESCRIPTION:
//  Basic public constructor for this class
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  iBase *parent : the iBase parent
//
// RETURN VALUE:
//  N/A
// =================================================================

csGraphics3DGLext::csGraphics3DGLext (iBase *parent ) :
  object_reg (NULL)
{
  SCF_CONSTRUCT_IBASE (parent);

  db_asp_center_x = 0;
  db_asp_center_y = 0;
  db_width = 0;
  db_height = 0;
  db_asp_ratio = 2.0f/3.0f;
  frustum_valid = false;

  m_clipper = new ClipperObjectStencil(this, 16, m_glstates);
}


// =================================================================
// DESTRUCTOR : csGraphics3DGLext
//
// DESCRIPTION:
//  Basic public destructor for this class
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  None
//
// RETURN VALUE:
//  N/A
// =================================================================

csGraphics3DGLext::~csGraphics3DGLext ( ) 
{
  // Make sure we close the display
  Close ();

  // destroy the clipper

  if(m_clipper)
    delete m_clipper;
}



// =================================================================
// FUNCTION: Initialize 
//
// DESCRIPTION:
//  Initializes this object using the Object Registry Given
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  iObjectRegistry *reg: the Object Registry to use
//
// RETURN VALUE:
//  bool: true on success, false on failure
// =================================================================

bool csGraphics3DGLext::Initialize(iObjectRegistry* reg) 
{
  object_reg = reg;
  CS_ASSERT( NULL != object_reg );

  config.AddConfig(object_reg, "/config/opengl.cfg");

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
    iCommandLineParser));

  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", CS_OPENGL_2D_DRIVER);

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (!G2D)
    return false;
  if (!object_reg->Register (G2D, "iGraphics2D"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
  "Could not register the canvas!");
    return false;
  }

  db_width = db_height = -1;

  return true;
  
  return true;
}


// =================================================================
// FUNCTION: Open
//
// DESCRIPTION:
//	Inherited from the iGraphics3D interface
//  Opens up the Graphics3D
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  N/A
//
// RETURN VALUE:
//  bool: true on success, false on failure
// =================================================================

bool csGraphics3DGLext::Open ()
{
	if (!G2D->Open ())
	{
		Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
		// set "not opened" flag
		db_width = db_height = -1;
		return false;
	}
	return true;
}

void csGraphics3DGLext::Close()
{
  if ((db_width == db_height) && db_height == -1)
	  return;

  // kill the graphics context
  if (G2D)
	  G2D->Close ();

  db_width = db_height = -1;
}

iGraphics2D *csGraphics3DGLext::GetDriver2D ( )
{	
  return G2D;	
}

void csGraphics3DGLext::SetDimensions( int width, int height )
{
  db_width = width;
  db_height = height;
  db_asp_center_x = width >> 1; // >> 1 == / 2
  db_asp_center_y = height >> 1; // >> 1 == / 2
  frustum_valid = false;
}

int csGraphics3DGLext::GetWidth ()
{
  return db_width;
}

int csGraphics3DGLext::GetHeight()
{
  return db_height;	
}

csGraphics3DCaps *csGraphics3DGLext::GetCaps()
{	
  return &g3d_capabilities;	
}

void csGraphics3DGLext::SetPerspectiveCenter( int x,  int y )
{	
  db_asp_center_x = x;
  db_asp_center_y = y;
}

void csGraphics3DGLext::GetPerspectiveCenter( int& x, int& y )
{
  x = db_asp_center_x;
  y = db_asp_center_y;
}

void  csGraphics3DGLext::SetPerspectiveAspect( float aspect )
{	
  db_asp_ratio = aspect;	
}

float csGraphics3DGLext::GetPerspectiveAspect()
{
  return db_asp_ratio;
}


void csGraphics3DGLext::SetObjectToCamera( csReversibleTransform *o2c )
{
  object2camera = *o2c;
}

const csReversibleTransform& csGraphics3DGLext::GetObjectToCamera( )
{
  return object2camera;
}

void csGraphics3DGLext::SetClipper( iClipper2D* clipper, int cliptype )
{
  m_clipper->SetClipper(clipper);
}

iClipper2D* csGraphics3DGLext::GetClipper( )
{
  return m_clipper->GetCurClipper();
}

int csGraphics3DGLext::GetClipType( )
{
  if(m_clipper->GetCurClipper())
    return CS_CLIPPER_REQUIRED;
  else
    return CS_CLIPPER_NONE;
}

void csGraphics3DGLext::SetNearPlane (const csPlane3& pl)
{
}

void csGraphics3DGLext::ResetNearPlane( )
{
}


const csPlane3& csGraphics3DGLext::GetNearPlane ()
{
  dummynearplane = csPlane3(csVector3(0.0,0.0,0.0001), csVector3(0.0,0.0,1.0));
  return dummynearplane;
}

bool csGraphics3DGLext::HasNearPlane( )
{
  return false;
}

// Debug methods from iGraphics3D
uint32 *csGraphics3DGLext::GetZBuffAt( int x, int y )
{
  return 0;
}

float csGraphics3DGLext::GetZBuffValue( int x, int y )
{
  float zbufval;
  glReadPixels(x,y,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &zbufval);
  return zbufval;
}

// Drawing methods from iGraphics3D
bool csGraphics3DGLext::BeginDraw(int DrawFlags)
{
  if ((G2D->GetWidth() != db_width) ||  (G2D->GetHeight() != db_height))
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());

  if (DrawFlags & CSDRAW_3DGRAPHICS)
    glFlush();

  if (DrawFlags & CSDRAW_2DGRAPHICS)
  {
    // disable Z-buffer test and blending
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
  }

  if (DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    if (!G2D->BeginDraw ())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  return true;
}

void csGraphics3DGLext::FinishDraw()
{
  glFlush();
  G2D->FinishDraw ();
}

void csGraphics3DGLext::Print(csRect *area)
{
  G2D->Print (area);
}

bool csGraphics3DGLext::SetRenderState(G3D_RENDERSTATEOPTION op, long val)
{ 
  switch (op)
  {
  case G3DRENDERSTATE_ZBUFFERMODE:
      switch(val)
      {
      case CS_ZBUF_EQUAL:
	   glDepthFunc(GL_EQUAL);
	   break;
      case CS_ZBUF_FILL:
	   glDepthFunc(GL_LEQUAL);
	   break;
      case CS_ZBUF_FILLONLY:
	   glDepthFunc(GL_ALWAYS);
	   break;
      case CS_ZBUF_NONE:
	   glDepthMask(GL_FALSE);
	   break;
      case CS_ZBUF_SPECIAL:
	   break;
      case CS_ZBUF_TEST:
	   glDepthMask(GL_TRUE);
	   glDepthFunc(GL_LEQUAL);
	   break;
      case CS_ZBUF_USE:
	   break;
      }
      break;
  case G3DRENDERSTATE_DITHERENABLE:
       break;
  case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
       break;
  case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
       break;
  case G3DRENDERSTATE_TRANSPARENCYENABLE:
       break;
  case G3DRENDERSTATE_MIPMAPENABLE:
       break;
  case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
       break;
  case G3DRENDERSTATE_MMXENABLE:
       return false;
  case G3DRENDERSTATE_INTERLACINGENABLE:
       return false;
  case G3DRENDERSTATE_LIGHTINGENABLE:
       break;
  case G3DRENDERSTATE_GOURAUDENABLE:
       break;
  case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
       break;
  case G3DRENDERSTATE_EDGES:
       break;
  default:
       return false;
  }

  return true;
}

long csGraphics3DGLext::GetRenderState(G3D_RENDERSTATEOPTION op )
{
  return 0;
}

void csGraphics3DGLext::DrawPolygon(G3DPolygonDP& poly )
{
}

void csGraphics3DGLext::DrawPolygonDebug(G3DPolygonDP& poly )
{
}

void csGraphics3DGLext::DrawPolygonFX(G3DPolygonDPFX& poly )
{
}

void csGraphics3DGLext::DrawTriangleMesh(G3DTriangleMesh& mesh )
{
}

void csGraphics3DGLext::DrawPolygonMesh(G3DPolygonMesh&  mesh )
{
}

void csGraphics3DGLext::OpenFogObject(CS_ID id, csFog* fog )
{
  // Fogging is handled by GL
  return;
}

void csGraphics3DGLext::DrawFogPolygon(CS_ID id, G3DPolygonDFP& poly, int fogtype )
{
  // Fogging is handled by GL
  return;
}

void csGraphics3DGLext::CloseFogObject(CS_ID id )
{
  // Fogging is handled by GL
  return;
}

void csGraphics3DGLext::DrawLine( const csVector3& v1, const csVector3& v2, float fov, int color )
{
}

iHalo *csGraphics3DGLext::CreateHalo (float iR, float iG, float IB, unsigned char *iAlpha, int iWidth, int iHeight )
{
  return NULL;
}

void csGraphics3DGLext::DrawPixmap(iTextureHandle *hTex,
                        int sx, int sy, int sw, int sh,
                        int tx, int ty, int tw, int th, uint8 Alpha )
{
}

// Texture Cache Management
iTextureManager *csGraphics3DGLext::GetTextureManager( )
{
  return NULL;
}

void csGraphics3DGLext::DumpCache( )
{
}

void csGraphics3DGLext::ClearCache( )
{
}

void csGraphics3DGLext::RemoveFromCache(iPolygonTexture *poly_texture)
{
}

iVertexBufferManager *csGraphics3DGLext::GetVertexBufferManager ( )
{
  return vbufmgr;
}

bool csGraphics3DGLext::IsLightmapOK(iPolygonTexture* poly_texture )
{
  return true;
}

void csGraphics3DGLext::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  
  if (rep)
	  rep->ReportV (severity, "crystalspace.graphics3d.opengl", msg, arg);
  else
  {
	  csPrintfV (msg, arg);
	  csPrintf ("\n");
  }
  va_end (arg);
}

/* -----------------------------------------------------------------
 * Protected Function Definitions
 * ----------------------------------------------------------------- */

/* -----------------------------------------------------------------
 * Private Function Defintions
 * ----------------------------------------------------------------- */
