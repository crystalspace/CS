/*
    Copyright (C) 2008 by Frank Richter

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
#include "csplugincommon/opengl/assumedstate.h"

namespace CS
{
  namespace PluginCommon
  {
    namespace GL
    {
      void SetAssumedState (csGLStateCache* statecache,
                             csGLExtensionManager* ext)
      {
        glClearDepth (1.0);
        glDepthRange (1.0, 0.0);
        statecache->SetPixelUnpackAlignment (1);
        // The texture format stuff generally assumes little endian
        #ifndef CS_LITTLE_ENDIAN
	        statecache->SetPixelUnpackSwapBytes (true);
        #else
	        statecache->SetPixelUnpackSwapBytes (false);
        #endif
	
        statecache->Enable_GL_VERTEX_PROGRAM_POINT_SIZE_ARB ();
        for (int i = statecache->GetNumTexCoords(); i-- > 0;)
        {
          statecache->SetCurrentTCUnit (i);
          statecache->ActivateTCUnit (csGLStateCache::activateTexEnv);
          glTexEnvi (GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
        }
        if (ext->CS_GL_ARB_point_parameters)
        {
          ext->glPointParameterfARB (GL_POINT_SIZE_MAX_ARB, 9999.0f);
          ext->glPointParameterfARB (GL_POINT_SIZE_MIN_ARB, 0.0f);
          ext->glPointParameterfARB (GL_POINT_FADE_THRESHOLD_SIZE_ARB, 1.0f);
        }
      }
    } // namespace GL
  } // namespace PluginCommon
} // namespace CS

