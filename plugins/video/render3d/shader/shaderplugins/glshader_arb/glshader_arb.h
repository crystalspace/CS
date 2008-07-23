/*
Copyright (C) 2002 by Marten Svanfeldt
                      Anders Stenberg

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

#ifndef __GLSHADER_ARB_H__
#define __GLSHADER_ARB_H__

#include "iutil/comp.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "ivideo/shader/shader.h"
#include "csutil/leakguard.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"

class csGLShader_ARB : public scfImplementation2<csGLShader_ARB, 
						 iShaderProgramPlugin,
					         iComponent>
{
private:
  bool enable;
  bool isOpen;

public:
  csGLExtensionManager* ext;
  iObjectRegistry* object_reg;

  CS_LEAKGUARD_DECLARE (csGLShader_ARB);

  csGLShader_ARB (iBase *parent);
  virtual ~csGLShader_ARB ();

  /**\name iShaderProgramPlugin implementation
   * @{ */
  virtual csPtr<iShaderProgram> CreateProgram (const char* type);

  virtual bool SupportType(const char* type);
  
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

#endif //__GLSHADER_ARB_H__

