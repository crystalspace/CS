/*
    Copyright (C) 2004 by Jorrit Tyberghein
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

#ifndef __CS_SHADERPLUGINS_COMMON_SHADERPROGRAM_H__
#define __CS_SHADERPLUGINS_COMMON_SHADERPROGRAM_H__

/**\file
 * Base class for iShaderProgram plugins.
 */

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/leakguard.h"
#include "csutil/ref.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "iutil/strset.h"

#include "csplugincommon/shader/shaderplugin.h"

struct iDataBuffer;
struct iFile;
struct iSyntaxService;
struct iObjectRegistry;

/**\addtogroup plugincommon
 * @{ */
/**
 * Base class for iShaderProgram plugins.
 * Provides basic services such as holding and of parameter mapping
 * information, basic program data and data dumping.
 */
class CS_CRYSTALSPACE_EXPORT csShaderProgram : 
  public scfImplementation2<csShaderProgram, iShaderProgram, iShaderTUResolver>
{
protected:
  csStringHash commonTokens;
#define CS_INIT_TOKEN_TABLE_NAME InitCommonTokens
#define CS_TOKEN_ITEM_FILE \
  "csplugincommon/shader/shaderprogram.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
#undef CS_INIT_TOKEN_TABLE_NAME

protected:
  iObjectRegistry* objectReg;
  csRef<iSyntaxService> synsrv;
  csRef<iStringSet> strings;

  /**
   * Expected/accepted types for a program parameter 
   */
  enum ProgramParamType
  {
    ParamInvalid    = 0,
    ParamFloat	    = 0x0001,
    ParamVector2    = 0x0002,
    ParamVector3    = 0x0004,
    ParamVector4    = 0x0008,
    ParamMatrix	    = 0x0010,
    ParamTransform  = 0x0020,
    ParamArray      = 0x0040,
    ParamShaderExp  = 0x0080,
    
    ParamVector     = ParamFloat | ParamVector2 | ParamVector3 | ParamVector4
  };

  /**
   * Program parameter, either a SV reference or a const value 
   */
  struct ProgramParam
  {
    bool valid;
    
    // Name of SV to use (if any)
    csStringID name;
    // Reference to const value shadervar
    csRef<csShaderVariable> var;

    ProgramParam() : valid (false), name(csInvalidStringID) { }
  };

  /// Parse program parameter node
  bool ParseProgramParam (iDocumentNode* node,
    ProgramParam& param, uint types = ~0);

  /**
   * Holder of variable mapping 
   */
  struct VariableMapEntry : public csShaderVarMapping
  {
    ProgramParam mappingParam;
    intptr_t userVal;

    VariableMapEntry (csStringID s, const char* d) : 
      csShaderVarMapping (s, d)
    { 
      userVal = 0;
      mappingParam.name = s;
      mappingParam.valid = true;
    }
    VariableMapEntry (const csShaderVarMapping& other) :
      csShaderVarMapping (other.name, other.destination)
    {
      userVal = 0;
      mappingParam.name = other.name;
      mappingParam.valid = true;
    }
  };
  /// Variable mappings
  csArray<VariableMapEntry> variablemap;

  /// Program description
  csString description;

  /// iDocumentNode the program is loaded from
  csRef<iDocumentNode> programNode;
  /// File the program is loaded from (if any)
  csRef<iFile> programFile;

  /// Filename of program
  csString programFileName;
  
  /**
   * Whether the shader program should report additional information during
   * runtime.
   */
  bool doVerbose;

  /// Parse common properties and variablemapping
  bool ParseCommon (iDocumentNode* child);
  /// Get the program node
  iDocumentNode* GetProgramNode ();
  /// Get the raw program data
  csPtr<iDataBuffer> GetProgramData ();

  /// Dump all program info to output
  void DumpProgramInfo (csString& output);
  /// Dump variable mapping
  void DumpVariableMappings (csString& output);

  //@{
  /**
   * Query the value of a ProgramParam variable by reading the constant or
   * resolving the shader variable.
   */
  inline csVector4 GetParamVectorVal (const csShaderVarStack &stacks, 
    const ProgramParam &param, const csVector4& defVal)
  {
    csRef<csShaderVariable> var;
  
    var = csGetShaderVariableFromStack (stacks, param.name);
    if (!var.IsValid ())
      var = param.var;
  
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      return defVal;
  
    csVector4 v;
    var->GetValue (v);
    return v;
  }
  inline csReversibleTransform GetParamTransformVal (const csShaderVarStack &stacks, 
    const ProgramParam &param, const csReversibleTransform& defVal)
  {
    csRef<csShaderVariable> var;
  
    var = csGetShaderVariableFromStack (stacks, param.name);
    if (!var.IsValid ())
      var = param.var;
  
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      return defVal;
  
    csReversibleTransform t;
    var->GetValue (t);
    return t;
  }
  inline float GetParamFloatVal (const csShaderVarStack &stacks, 
    const ProgramParam &param, float defVal)
  {
    csRef<csShaderVariable> var;
  
    var = csGetShaderVariableFromStack (stacks, param.name);
    if (!var.IsValid ())
      var = param.var;
  
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      return defVal;
  
    float f;
    var->GetValue (f);
    return f;
  }
  //@}
public:
  CS_LEAKGUARD_DECLARE (csShaderProgram);

  csShaderProgram (iObjectRegistry* objectReg);
  virtual ~csShaderProgram ();

  virtual int ResolveTextureBinding (const char* /*binding*/)
  { return -1; }
};

/** @} */

#endif // __CS_SHADERPLUGINS_COMMON_SHADERPROGRAM_H__
