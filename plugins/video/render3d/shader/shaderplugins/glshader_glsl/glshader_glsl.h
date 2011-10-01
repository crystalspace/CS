/*
  Copyright (C) 2011 by Antony Martin

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

#ifndef __GLSHADER_GLSL_H__
#define __GLSHADER_GLSL_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/scfstringarray.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderGLSL)
{
  class csGLShader_GLSL : public scfImplementation2<csGLShader_GLSL, 
    iShaderProgramPlugin,
    iComponent>
  {
  private:
    bool enable;
    bool isOpen;

  public:
    csGLExtensionManager* ext;
    iObjectRegistry* object_reg;
    csRef<iGraphics3D> graph;

    CS_LEAKGUARD_DECLARE (csGLShader_GLSL);

    csGLShader_GLSL (iBase *parent);
    virtual ~csGLShader_GLSL ();

    void Report (int severity, const char* msg, ...);

    /**\name iShaderProgramPlugin implementation
    * @{ */
    virtual csPtr<iShaderProgram> CreateProgram (const char* type);

    virtual bool SupportType (const char* type);

    csPtr<iStringArray> QueryPrecacheTags (const char* type)
    {
      scfStringArray* tags = new scfStringArray;
      tags->Push ("default");
      return csPtr<iStringArray> (tags);
    }  
    bool Precache (const char* type, const char* tag,
      iBase* previous, 
      iDocumentNode* node, iHierarchicalCache* cacheTo,
      csRef<iBase>* outObj = 0) { return false; }

    void Open();
    /** @} */

    /**\name iComponent implementation
    * @{ */
    bool Initialize (iObjectRegistry* reg);
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderGLSL)

#endif //__GLSHADER_GLSL_H__

