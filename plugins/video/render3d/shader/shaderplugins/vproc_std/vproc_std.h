/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_VPROC_VPROC_STD_H__
#define __CS_VPROC_VPROC_STD_H__

#include "iutil/comp.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/leakguard.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "iengine/light.h"

struct iVertexLightCalculator;

CS_PLUGIN_NAMESPACE_BEGIN(VProc_std)
{

class csVProc_Std : public scfImplementation2<csVProc_Std, 
					      iShaderProgramPlugin,
					      iComponent>
{
public:
  CS_LEAKGUARD_DECLARE (csVProc_Std);

  csVProc_Std (iBase *parent);
  virtual ~csVProc_Std ();

  iVertexLightCalculator* GetLightCalculator (
    const csLightProperties& light, bool useAttenuation = true);

  /**\name iShaderProgramPlugin implementation
   * @{ */
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  void Open();

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
  /** @} */

  /**\name iComponent implementation
   * @{ */
  bool Initialize (iObjectRegistry* reg);
  /** @} */

  iObjectRegistry *objreg;
  csRef<iShaderManager> shaderManager;
  CS::ShaderVarStringID string_object2world;
  CS::ShaderVarStringID string_world2camera;
  csLightShaderVarCache lsvCache;
private:
  bool isOpen;

  int GetMatrixIndex (csLightType type, csLightAttenuationMode attn) const
  {
    return type*5 + attn;
  }
  iVertexLightCalculator *lightCalculatorMatrix[16];
};

}
CS_PLUGIN_NAMESPACE_END(VProc_std)

#endif //__CS_VPROC_VPROC_STD_H__

