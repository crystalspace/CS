/*
  Copyright (C) 2011 by Frank Richter

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

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "instancing.h"

#include "csplugincommon/opengl/glhelper.h"

#include "gl_render3d.h"
#include "profilescope.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  InstancingHelper::InstancingHelper (csGLGraphics3D* g3d,
				      size_t numInstances,
				      size_t paramNum,
				      const csVertexAttrib* paramsTargets,
				      csShaderVariable** const * params)
   : g3d (g3d), instParamNum (paramNum),   
     targets (paramsTargets), allParams (params),
     instNum (0), numInstances (numInstances)
  {
  }

  void InstancingHelper::SetupNextInstance ()
  {
    ProfileScope _profile (g3d, "Setup instance"); // @@@ Worthwhile to measure?
    
    csShaderVariable* const* params = allParams[instNum];
  
    csVector4 v;
    float matrix[16];
    bool uploadMatrix;
    for (size_t n = 0; n < instParamNum; n++)
    {
      csShaderVariable* param = params[n];
      csVertexAttrib target = targets[n];
      switch (param->GetType())
      {
      case csShaderVariable::MATRIX:
	{
	  csMatrix3 m;
	  params[n]->GetValue (m);
	  makeGLMatrix (m, matrix, target != CS_IATTRIB_OBJECT2WORLD);
	  uploadMatrix = true;
	}
	break;
      case csShaderVariable::TRANSFORM:
	{
	  csReversibleTransform tf;
	  params[n]->GetValue (tf);
	  makeGLMatrix (tf, matrix, target != CS_IATTRIB_OBJECT2WORLD);
	  uploadMatrix = true;
	}
	break;
      default:
	uploadMatrix = false;
      }
      if (uploadMatrix)
      {
	switch (target)
	{
	case CS_IATTRIB_OBJECT2WORLD:
	  {
	    g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    glMultMatrixf (matrix);
	  }
	  break;
	default:
	  if (g3d->ext->CS_GL_ARB_multitexture)
	  {
	    if ((target >= CS_VATTRIB_TEXCOORD0) 
	      && (target <= CS_VATTRIB_TEXCOORD7))
	    {
	      // numTCUnits is type GLint, while target is an enumerated
	      // type. These are not necessarily the same.
	      size_t maxN = csMin (3,csVertexAttrib(g3d->GetNumTCUnits()) - (target - CS_VATTRIB_TEXCOORD0));
	      GLenum tu = GL_TEXTURE0 + (target - CS_VATTRIB_TEXCOORD0);
	      for (size_t n = 0; n < maxN; n++)
	      {
		g3d->ext->glMultiTexCoord4fvARB (tu + (GLenum)n, &matrix[n*4]);
	      }
	    }
	  }
	  if (g3d->ext->glVertexAttrib4fvARB)
	  {
	    if (CS_VATTRIB_IS_GENERIC (target))
	    {
	      size_t maxN = csMin (3, CS_VATTRIB_GENERIC_LAST - target + 1);
	      GLenum attr = (target - CS_VATTRIB_GENERIC_FIRST);
	      for (size_t n = 0; n < maxN; n++)
	      {
		g3d->ext->glVertexAttrib4fvARB (attr + (GLenum)n, &matrix[n*4]);
	      }
	    }
	  }
	}
      }
      else
      {
	params[n]->GetValue (v);
	switch (target)
	{
	case CS_VATTRIB_WEIGHT:
	  if (g3d->ext->CS_GL_EXT_vertex_weighting)
	    g3d->ext->glVertexWeightfvEXT (v.m);
	  break;
	case CS_VATTRIB_NORMAL:
	  glNormal3fv (v.m);
	  break;
	case CS_VATTRIB_PRIMARY_COLOR:
	  glColor4fv (v.m);
	  break;
	case CS_VATTRIB_SECONDARY_COLOR:
	  if (g3d->ext->CS_GL_EXT_secondary_color)
	    g3d->ext->glSecondaryColor3fvEXT (v.m);
	  break;
	case CS_VATTRIB_FOGCOORD:
	  if (g3d->ext->CS_GL_EXT_fog_coord)
	    g3d->ext->glFogCoordfvEXT (v.m);
	  break;
	case CS_IATTRIB_OBJECT2WORLD:
	  {
	    g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	  }
	  break;
	default:
	  if (g3d->ext->CS_GL_ARB_multitexture)
	  {
	    if ((target >= CS_VATTRIB_TEXCOORD0) 
	      && (target <= CS_VATTRIB_TEXCOORD7))
	    {
	      g3d->ext->glMultiTexCoord4fvARB (
		GL_TEXTURE0 + (target - CS_VATTRIB_TEXCOORD0), 
		v.m);
	    }
	  }
	  else if (target == CS_VATTRIB_TEXCOORD)
	  {
	    glTexCoord4fv (v.m);
	  }
	  if (g3d->ext->glVertexAttrib4fvARB)
	  {
	    if (CS_VATTRIB_IS_GENERIC (target))
	    {
	      g3d->ext->glVertexAttrib4fvARB (target - CS_VATTRIB_0,
		v.m);
	    }
	  }
	}
      }
    }
  }
  
  void InstancingHelper::TeardownInstance ()
  {
    for (size_t n = 0; n < instParamNum; n++)
    {
      csVertexAttrib target = targets[n];
      switch (target)
      {
      case CS_IATTRIB_OBJECT2WORLD:
	{
	  g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	  glPopMatrix ();
	}
	break;
      default:
	/* Nothing to do */
	break;
      }
    }
    
    instNum++;
  }
  
  void InstancingHelper::DrawAllInstances (GLenum mode, GLuint start, GLuint end,
					   GLsizei count, GLenum type, const GLvoid* indices)
  {
    while (instNum < numInstances)
    {
      SetupNextInstance ();
      g3d->glDrawRangeElements (mode, start, end, count, type, indices);
      TeardownInstance ();
    }
  }
}
CS_PLUGIN_NAMESPACE_END(gl3d)
