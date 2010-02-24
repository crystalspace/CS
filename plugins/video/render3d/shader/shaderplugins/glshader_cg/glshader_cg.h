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

#ifndef __GLSHADER_CG_H__
#define __GLSHADER_CG_H__

#include "cg_common.h"

#include "csplugincommon/opengl/shaderplugin.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/blockallocator.h"
#include "csutil/leakguard.h"

#include "iutil/comp.h"
#include "ivideo/shader/shader.h"

#include "glshader_cgcommon.h"
#include "profile_limits.h"
#include "progcache.h"

struct csGLExtensionManager;

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
class StringStore;

class csGLShader_CG :
  public scfImplementationExt1<csGLShader_CG, 
			       CS::PluginCommon::ShaderProgramPluginGL,
			       iComponent>
{
private:
  static void ErrorHandler (CGcontext context, CGerror err, void* appdata);
  static void ErrorHandlerObjReg (CGcontext context, CGerror err, void* appdata);

  bool enable;
  const char* compiledProgram;
  bool doIgnoreErrors;
  
  typedef csArray<ProfileLimitsPair> ProfileLimitsArray;
  ProfileLimitsArray precacheLimits;
  void ParsePrecacheLimits (iConfigFile* config, ProfileLimitsArray& out);
  bool Precache (csShaderGLCGCommon* prog, ProfileLimitsPair& limits,
    const char* tag, iHierarchicalCache* cacheTo);
public:
  CS_LEAKGUARD_DECLARE (csGLShader_CG);
  
  using CS::PluginCommon::ShaderProgramPluginGL::ext;
  using CS::PluginCommon::ShaderProgramPluginGL::statecache;
  using CS::PluginCommon::ShaderProgramPluginGL::doVerbose;
  using CS::PluginCommon::ShaderProgramPluginGL::doVerbosePrecache;
  using CS::PluginCommon::ShaderProgramPluginGL::object_reg;
  using CS::PluginCommon::ShaderProgramPluginGL::vendor;

  CGcontext context;
  csRef<iShaderProgramPlugin> psplg;
  bool debugDump;
  char* dumpDir;

  bool enableVP, enableFP;
  ProfileLimitsPair currentLimits;
  bool strictMatchVP, strictMatchFP;
  
  csBlockAllocator<ShaderParameter> paramAlloc;
  
  csRef<iDocumentSystem> binDocSys;
  csRef<iDocumentSystem> xmlDocSys;

  ProgramCache progCache;
  StringStore* stringStore;

  csGLShader_CG (iBase *parent);
  virtual ~csGLShader_CG ();

  void Report (int severity, const char* msg, ...) CS_GNUC_PRINTF(3, 4);
  
  /**\name iShaderProgramPlugin implementation
   * @{ */
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  csPtr<iStringArray> QueryPrecacheTags (const char* type);
  bool Precache (const char* type, const char* tag,
    iBase* previous, 
    iDocumentNode* node, iHierarchicalCache* cacheTo,
    csRef<iBase>* outObj = 0);

  bool Open();
  /** @} */

  /**\name iComponent implementation
   * @{ */
  bool Initialize (iObjectRegistry* reg);
  /** @} */

  void SplitArgsString (const char* str, ArgumentArray& args);
  enum
  {
    argsAll = 0,
    argsNoConfig = 1,
    argsNoProfileLimits = 2,
    argsNoProgramType = 4,
    argsNone = argsNoConfig | argsNoProfileLimits | argsNoProgramType
  };
  void GetProfileCompilerArgs (const char* type, CGprofile profile, 
    const ProfileLimitsPair& limitsPair,
    HardwareVendor vendor, uint argsMask, ArgumentArray& args);
  static bool ProfileNeedsRouting (CGprofile profile)
  {
    return (profile >= CG_PROFILE_PS_1_1) && (profile <= CG_PROFILE_PS_1_3);
  }
  bool IsRoutedProfileSupported (CGprofile profile);
  
  void SetCompiledSource (const char* prog)
  { compiledProgram = prog; }
  void SetIgnoreErrors (bool doIgnore) { doIgnoreErrors = doIgnore; }
  void PrintCgListing (const char* listing);
  void PrintAnyListing ()
  {
    const char* listing = cgGetLastListing (context);
    if (listing && *listing && doVerbose)
    {
      PrintCgListing (listing);
    }
  }
};

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif //__GLSHADER_CG_H__
