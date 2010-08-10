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

#include "cg_common.h"
#include "progcache.h"

#include "csplugincommon/opengl/shaderplugin.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"

using namespace CS::PluginCommon;

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

  class csGLShader_CG;
  struct ProfileLimits;
  struct ProfileLimitsPair;

  struct iShaderProgramCG : public virtual iBase
  {
    SCF_INTERFACE(iShaderProgramCG, 0,0,1);

    virtual const csSet<csString>& GetUnusedParameters () = 0;
  };
  
  struct ShaderParameter
  {
    bool assumeConstant;
    CGparameter param;
    uint baseSlot;
    CGtype paramType;
    csArray<ShaderParameter*> arrayItems;
    
    ShaderParameter() : assumeConstant (false), param (0),
      baseSlot ((uint)~0),
      paramType ((CGtype)0) {}
  };
  
  class ParamValueCache;

class csShaderGLCGCommon : public scfImplementationExt1<csShaderGLCGCommon,
                                                        csShaderProgram,
                                                        iShaderProgramCG>
{
protected:
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/glshader_cg/glshader_cg.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  csRef<csGLShader_CG> shaderPlug;

  CGprogram program;
  CGprofile programProfile;
  bool programPositionInvariant;
  csString entrypoint;
  csRefArray<iDocumentNode> cacheKeepNodes;

  // Wrapped PS1.x shader program
  csRef<iShaderProgram> pswrap;
  
  enum ProgramType
  {
    progVP, progFP
  };
  ProgramType programType;
  ArgumentArray compilerArgs;
  csRef<iShaderProgramCG> cgResolve;
  csSet<csString> unusedParams;
  
  struct Clip
  {
    ShaderProgramPluginGL::ClipPlanes::ClipSpace space;
    
    ProgramParam plane;
    ProgramParam distance;
    int distComp;
    bool distNeg;
  };
  csArray<Clip> clips;
  // Magic SV names
  enum
  {
    svClipPackedDist0 = 0xffff41,
    svClipPackedDist1 = 0xffff42,
    svClipPlane = 0xffff23 // must be lowest of the 'magic' values
  };
  csRef<csShaderVariable> clipPackedDists[2];
  csRef<csShaderVariable> clipPlane[6];
  bool ParseClip (iDocumentNode* node);
  bool ParseVmap (iDocumentNode* node);

  csString debugFN;
  void EnsureDumpFile();

  void FreeShaderParam (ShaderParameter* sparam);
  void FillShaderParam (ShaderParameter* sparam, CGparameter param);
  void GetShaderParamSlot (ShaderParameter* sparam);
  /**
   * Go over variablemaps and fetch Cg parameters for them
   */
  void GetParamsFromVmap();
  //@{
  /**
   * Set properties for a mapped parameter which can only be determined
   * after compilation
   */
  void GetPostCompileParamProps ();
  bool GetPostCompileParamProps (ShaderParameter* sparam);
  //@}

  void PrecacheClear();

  enum
  {
    loadLoadToGL = 1,
    loadIgnoreErrors = 2,
    loadApplyVmap = 4,
    loadIgnoreConfigProgramOpts = 8,
    loadFlagUnusedV2FForInit = 16
  };
  bool DefaultLoadProgram (iShaderProgramCG* cgResolve,
    const char* programStr, ProgramType type,
    const ProfileLimitsPair& customLimits, 
    uint flags = loadLoadToGL | loadApplyVmap | loadFlagUnusedV2FForInit);
  csString GetAugmentedProgram (const char* programStr,
    bool initializeUnusedV2F = false);
  void DoDebugDump ();
  void DebugDumpParam (csString& output, CGparameter param);
  void WriteAdditionalDumpInfo (const char* description, const char* content);
  const char* GetProgramType()
  {
    switch (programType)
    {
      case progVP: return "vertex";
      case progFP: return "fragment";
    }
    return 0;
  }
  
  void ClipsToVmap ();
  void OutputClipPreamble (csString& str);
  void WriteClipApplications (csString& str);
  
  void CollectUnusedParameters (csSet<csString>& unusedParams,
				const csStringArray& unusedCandidates);
  template<typename Setter>
  void SetParameterValue (const Setter& setter,
    ShaderParameter* sparam, csShaderVariable* var);
  void SetParameterValueCg (ShaderParameter* sparam, csShaderVariable* var);
  
  void SVtoCgMatrix3x3 (csShaderVariable* var, float* matrix);
  void SVtoCgMatrix4x4 (csShaderVariable* var, float* matrix);

  template<typename Array, typename ParamSetter>
  void ApplyVariableMapArray (const Array& array, const ParamSetter& setter,
    const csShaderVariableStack& stack);
  void ApplyVariableMapArrays (const csShaderVariableStack& stack);
  
  bool WriteToCacheWorker (iHierarchicalCache* cache, const ProfileLimits& limits,
    const ProfileLimitsPair& limitsPair, const char* tag, 
    const ProgramObject& program, csString& failReason);
  bool WriteToCache (iHierarchicalCache* cache, const ProfileLimits& limits,
    const ProfileLimitsPair& limitsPair, const char* tag,
    const ProgramObject& program);
  bool WriteToCache (iHierarchicalCache* cache, const ProfileLimits& limits,
    const ProfileLimitsPair& limitsPair, const char* tag);
  
  bool GetProgramNode (iDocumentNode* passProgNode);

  bool LoadProgramWithPS1 ();
public:
  CS_LEAKGUARD_DECLARE (csShaderGLCGCommon);

  csShaderGLCGCommon (csGLShader_CG* shaderPlug, ProgramType type);
  virtual ~csShaderGLCGCommon ();

  virtual bool Precache (const ProfileLimitsPair& limitsPair,
    const char* tag, iHierarchicalCache* cache) = 0;
    
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
    const csShaderVariableStack& stack);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid () { return program != 0;} 

  /// Loads from a document-node
  virtual bool Load (iShaderDestinationResolver*, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderDestinationResolver*, const char*, 
    csArray<csShaderVarMapping>&)
  { return false; }

  const csSet<csString>& GetUnusedParameters ()
  { return unusedParams; }
  
  iShaderProgram::CacheLoadResult LoadFromCache (
    iHierarchicalCache* cache, iBase* previous, iDocumentNode* programNode,
    csRef<iString>* failReason = 0, csRef<iString>* tag = 0,
    ProfileLimitsPair* cacheLimits = 0);
  virtual iShaderProgram::CacheLoadResult LoadFromCache (
    iHierarchicalCache* cache, iBase* previous, iDocumentNode* programNode,
    csRef<iString>* failReason = 0, csRef<iString>* tag = 0)
  { return LoadFromCache (cache, previous, programNode, failReason, tag, 0); }
};

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif //__GLSHADER_CGCOMMON_H__
