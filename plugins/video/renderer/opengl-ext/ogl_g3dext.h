#ifndef _OGL_G3DEXT_HPP_
#define _OGL_G3DEXT_HPP_
/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: ogl_g3dext.hpp
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
 *
 * CVS/RCS ID:
 *    $Id$
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */

/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */

// Look for the CS OPENGL PATH
#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

/* -----------------------------------------------------------------
 * Preprocessor Defines and Enumeration Types
 * ----------------------------------------------------------------- */

#include "csgeom/transfrm.h"
#include "gl_states_ext.h"
#include "clipperobj.h"

/* -----------------------------------------------------------------
 * Public Class Declarations
 * ----------------------------------------------------------------- */
class csGraphics3DGLext;
struct iObjectRegistry;
struct iClipper2D;
struct iTextureManager;

/* -----------------------------------------------------------------
 * Public Class Definitions
 * ----------------------------------------------------------------- */
class csGraphics3DGLext : public iGraphics3D
{

  // -------------------------------------------------------
  // Public Constructors and Destructors
  // -------------------------------------------------------
  public:
    SCF_DECLARE_IBASE;

    // Public Constructor
    csGraphics3DGLext(iBase*);
    // Public Destructor
    virtual ~csGraphics3DGLext();

  // -------------------------------------------------------
  // Public Members and Methods
  // -------------------------------------------------------
  public:
    // Access to the configuration file
    csConfigAccess config;

    // The System Interface
    iObjectRegistry *object_reg;

    // Initialization inherited from iComponent
    virtual bool Initialize (iObjectRegistry *);

    // Inherited from the iGraphics3D interface
    virtual bool Open ();
    virtual void Close();

    virtual iGraphics2D *GetDriver2D ( );

    virtual void SetDimensions( int width, int height );

    virtual int GetWidth ();
    virtual int GetHeight();

    virtual csGraphics3DCaps *GetCaps();

    virtual void SetPerspectiveCenter( int x,  int y );
    virtual void GetPerspectiveCenter( int& x, int& y );

    virtual void  SetPerspectiveAspect( float aspect );
    virtual float GetPerspectiveAspect( );

    virtual void SetObjectToCamera( csReversibleTransform *o2c );
    virtual const csReversibleTransform& GetObjectToCamera( );

    virtual void SetClipper( iClipper2D* clipper, int cliptype );
    virtual iClipper2D* GetClipper( );
    virtual int GetClipType( );

    virtual void SetNearPlane (const csPlane3& pl);
    virtual void ResetNearPlane( );
    virtual const csPlane3& GetNearPlane ();
    virtual bool HasNearPlane( );

    // Debug methods from iGraphics3D
    virtual uint32 *GetZBuffAt( int x, int y );
    virtual float   GetZBuffValue( int x, int y );
    
    // Drawing methods from iGraphics3D
    virtual bool BeginDraw(int DrawFlags);
    virtual void FinishDraw( );
    virtual void Print(csRect *area);

    virtual bool SetRenderState( G3D_RENDERSTATEOPTION op, long val);
    virtual long GetRenderState( G3D_RENDERSTATEOPTION op );

    virtual void DrawPolygon     ( G3DPolygonDP& poly );
    virtual void DrawPolygonDebug( G3DPolygonDP& poly );
    virtual void DrawPolygonFX   ( G3DPolygonDPFX& poly );

    virtual void DrawTriangleMesh( G3DTriangleMesh& mesh );
    virtual void DrawPolygonMesh ( G3DPolygonMesh&  mesh );

    virtual void OpenFogObject (CS_ID id, csFog* fog );
    virtual void DrawFogPolygon(CS_ID id, G3DPolygonDFP& poly, int fogtype );
    virtual void CloseFogObject(CS_ID id );

    virtual void DrawLine( const csVector3& v1, const csVector3& v2,
                           float fov, int color );
    virtual iHalo *CreateHalo (float iR, float iG, float IB,
                               unsigned char *iAlpha, int iWidth, int iHeight );

    virtual void DrawPixmap(iTextureHandle *hTex,
                            int sx, int sy, int sw, int sh,
                            int tx, int ty, int tw, int th, uint8 Alpha );

    // Texture Cache Management
    virtual iTextureManager *GetTextureManager( );
    virtual void DumpCache( );
    virtual void ClearCache( );
    virtual void RemoveFromCache(iPolygonTexture *poly_texture);

    virtual iVertexBufferManager *GetVertexBufferManager ( );
    virtual bool IsLightmapOK(iPolygonTexture* poly_texture );

  // -------------------------------------------------------
  // Protected Functions
  // -------------------------------------------------------

    void Report (int severity, const char* msg, ...);
protected:
    // The Vertex Buffer ... (diffrent Implementations will be chosen
    // depending on the Graphics card (atm there's only a standard one)
    iVertexBufferManager *vbufmgr; 
    // The 2D Graphics renderer
    csRef<iGraphics2D> G2D;

    // recalc frustum ?
    bool frustum_valid;

    // dummy nearplane
    csPlane3 dummynearplane;

    bool stencil_init;
    bool planes_init;

    // Current object 2 Camera transform
    csReversibleTransform object2camera;

    // 3D Rendering capabilities
    csGraphics3DCaps g3d_capabilities;

    // Width and Height of Drawing Buffer
    int db_width;
    int db_height;

    // Perspective Center of the Drawing Buffer
    int db_asp_center_x;
    int db_asp_center_y;

    // Aspect Ratio
    float db_asp_ratio;

    // These are the states of various glEnable/glDisable elements
    // It is less expensive to check these than to glEnable of glDisable
    // them if they are already Enabled/Disabled

    gl_states_ext m_glstates;    

    // Clipper Stuff
    iClipperObject *m_clipper;
};

#endif /* _OGL_G3DEXT_HPP_ */
