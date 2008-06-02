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

#ifndef __GLSHADER_CGCOMMON_H__
#define __GLSHADER_CGCOMMON_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"

#include "glshader_cg.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

  struct iShaderDestinationResolverCG : public virtual iBase
  {
    SCF_INTERFACE(iShaderDestinationResolverCG, 0,0,1);

    virtual const csArray<csString>& GetUnusedParameters () = 0;
  };

class csShaderGLCGCommon : public scfImplementationExt1<csShaderGLCGCommon,
                                                        csShaderProgram,
                                                        iShaderDestinationResolverCG>
{
protected:
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/glshader_cg/glshader_cg.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  csGLShader_CG* shaderPlug;

  CGprogram program;
  CGprofile programProfile;
  csString cg_profile;
  csString entrypoint;

  bool validProgram;

  const char* programType;
  ArgumentArray compilerArgs;
  csRef<iShaderDestinationResolverCG> cgResolve;
  csArray<csString> unusedParams;

  csString debugFN;

  bool DefaultLoadProgram (iShaderDestinationResolverCG* cgResolve,
    const char* programStr, CGGLenum type, 
    CGprofile maxProfile, bool compiled = false, bool doLoad = true);
  void DoDebugDump ();
  void WriteAdditionalDumpInfo (const char* description, const char* content);
  virtual const char* GetProgramType() = 0;
  void CollectUnusedParameters ();
  void SetParameterValue (CGparameter param, csShaderVariable* var);
  
  void SVtoCgMatrix3x3 (csShaderVariable* var, float* matrix);
  void SVtoCgMatrix4x4 (csShaderVariable* var, float* matrix);
public:
  CS_LEAKGUARD_DECLARE (csShaderGLCGCommon);

  csShaderGLCGCommon (csGLShader_CG* shaderPlug, const char* type);
  virtual ~csShaderGLCGCommon ();

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const CS::Graphics::RenderMesh* mesh,
    CS::Graphics::RenderMeshModes& modes,
    const iShaderVarStack* stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid () { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load (iShaderDestinationResolver*, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderDestinationResolver*, const char*, 
    csArray<csShaderVarMapping>&)
  { return false; }

  const csArray<csString>& GetUnusedParameters ()
  { return unusedParams; }
};

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif //__GLSHADER_CGCOMMON_H__
