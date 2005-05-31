/*
  Copyright (C) 2002 by Anders Stenberg
                        Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __GLSHADER_CGVP_H__
#define __GLSHADER_CGVP_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"

#include <Cg/cg.h>
/* WIN32is used in an "#if" inside <cgGL.h>, however, it is sometimes defined
 * without value. */
#ifdef WIN32
#undef WIN32
#define WIN32 1
#endif 
#include <Cg/cgGL.h>

#include "glshader_cgcommon.h"

class csShaderGLCGVP : public csShaderGLCGCommon
{
private:
  struct CGMatrixTrackerEntry
  {
    CGGLenum cgMatrix;
    CGGLenum cgTransform;
    CGparameter cgParameter;
  };

  struct NVMatrixTrackerEntry
  {
    GLenum nvMatrix;
    GLenum nvTransform;
    GLuint nvParameter;
  };

  bool cgTrackMatrices;
  csArray<CGMatrixTrackerEntry> cgMatrixTrackers;
  bool nvTrackMatrices;
  csArray<NVMatrixTrackerEntry> nvMatrixTrackers;
  csRef<iShaderProgram> override;
public:
  CS_LEAKGUARD_DECLARE (csShaderGLCGVP);

  csShaderGLCGVP (csGLShader_CG* shaderPlug) : 
    csShaderGLCGCommon (shaderPlug, "cgvp"), cgTrackMatrices(false), 
    nvTrackMatrices(false) { }

  /// Sets this program to be the one used when rendering
  virtual void Activate ();
  /// Compile a program
  virtual bool Compile();
};


#endif //__GLSHADER_CGVP_H__
