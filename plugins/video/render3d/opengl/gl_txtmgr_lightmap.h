/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004 by Frank Richter

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

#ifndef __CS_GL_TXTMGR_LIGHTMAP_H__
#define __CS_GL_TXTMGR_LIGHTMAP_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csgeom/csrect.h"
#include "csutil/leakguard.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

#include "ivideo/txtmgr.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLSuperLightmap;
class csGLTextureHandle;
class csGLTextureManager;

/**
 * A single lightmap on a super lightmap.
 */
class csGLRendererLightmap : 
  public scfImplementation1<csGLRendererLightmap, 
			    iRendererLightmap>
{
private:
  friend class csGLSuperLightmap;
  friend class csGLGraphics3D;

  /// Texture coordinates (in pixels)
  csRect rect;
  /// The SLM this lightmap is a part of.
  csRef<csGLSuperLightmap> slm;
public:
  CS_LEAKGUARD_DECLARE (csGLRendererLightmap);

  void DecRef();

  csGLRendererLightmap ();
  virtual ~csGLRendererLightmap ();

  /// Return the LM texture coords, in pixels.
  virtual void GetSLMCoords (int& left, int& top, 
    int& width, int& height);

  /// Set the light data.
  virtual void SetData (csRGBcolor* data);

  /// Set the size of a light cell.
  virtual void SetLightCellSize (int size);
};

/**
 * An OpenGL super lightmap.
 */
class csGLSuperLightmap : 
  public scfImplementation1<csGLSuperLightmap,
			    iSuperLightmap>
{
private:
  friend class csGLRendererLightmap;

  /// Number of lightmaps on this SLM.
  int numRLMs;

  csRef<csGLTextureHandle> th;

  /// Actually create the GL texture.
  void CreateTexture ();
  /**
   * Free a lightmap. Will also delete the GL texture if no LMs are on 
   * this SLM.
   */
  void FreeRLM (csGLRendererLightmap* rlm);
public:
  CS_LEAKGUARD_DECLARE (csGLSuperLightmap);

  /// GL texture handle
  GLuint texHandle;
  /// Dimensions of this SLM
  int w, h;
  /// Remove the GL texture.
  void DeleteTexture ();

  /// The texture manager that created this SLM.
  csGLTextureManager* txtmgr;

  void DecRef();

  csGLSuperLightmap (csGLTextureManager* txtmgr, int width, int height);
  virtual ~csGLSuperLightmap ();

  /// Add a lightmap.
  virtual csPtr<iRendererLightmap> RegisterLightmap (int left, int top, 
    int width, int height);

  /// Dump the contents onto an image.
  virtual csPtr<iImage> Dump ();

  virtual iTextureHandle* GetTexture ();
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_TXTMGR_LIGHTMAP_H__

