/*
  Copyright (C) 2002-2005 by Marten Svanfeldt
			     Anders Stenberg
			     Frank Richter

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

#include "cssysdef.h"

#include "csgeom/math.h"
#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glhelper.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "glshader_cg.h"
#include "glshader_cgcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGCommon);

csShaderGLCGCommon::csShaderGLCGCommon (csGLShader_CG* shaderPlug, 
					ProgramType type) :
  scfImplementationType (this, shaderPlug->object_reg), programType (type),
  assumedConstParams (0)
{
  validProgram = true;
  this->shaderPlug = shaderPlug;
  program = 0;

  InitTokenTable (xmltokens);
}

csShaderGLCGCommon::~csShaderGLCGCommon ()
{
  if (program)
    cgDestroyProgram (program);
    
  if (assumedConstParams != 0)
  {
    for(size_t i = 0; i < assumedConstParams->GetSize (); ++i)
    {
      VariableMapEntry& mapping = assumedConstParams->Get (i);
      
      ShaderParameter* param =
	reinterpret_cast<ShaderParameter*> (mapping.userVal);
      
      FreeShaderParam (param);
    }
    delete assumedConstParams;
  }

  for(size_t i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];
    
    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    
    FreeShaderParam (param);
  }
}

void csShaderGLCGCommon::Activate()
{
  cgGLEnableProfile (programProfile);
  if (!cgGLIsProgramLoaded (program))
  {
    cgGLLoadProgram (program);
    PostCompileVmapProcess ();
  }
  cgGLBindProgram (program);
}

void csShaderGLCGCommon::Deactivate()
{
  cgGLDisableProfile (programProfile);
}

template<typename Setter>
void csShaderGLCGCommon::SetParameterValue (const Setter& setter,
                                            ShaderParameter* sparam,
                                            csShaderVariable* var)
{
  if (sparam == 0) return;
    
  CGparameter param = sparam->param;
  CGtype paramType = sparam->paramType;
  uint slot = sparam->baseSlot;
  
  switch (paramType)
  {
    case CG_INT:
      {
	int i;
	if (var->GetValue (i))
	  setter.Parameter1i (slot, param, i);
      }
      break;
    case CG_FLOAT:
      {
	float f;
	if (var->GetValue (f))
	  setter.Parameter1f (slot, param, f);
      }
      break;
    case CG_FLOAT2:
      {
	csVector2 v;
	if (var->GetValue (v))
	  setter.Parameter2fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT3:
      {
	csVector3 v;
	if (var->GetValue (v))
	  setter.Parameter3fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT4:
      {
	csVector4 v;
	if (var->GetValue (v))
	  setter.Parameter4fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT3x3:
      {
	float matrix[9];
	SVtoCgMatrix3x3 (var, matrix);
        setter.MatrixParameter3x3 (slot, param, matrix);
      }
      break;
    case CG_FLOAT4x4:
      {
	float matrix[16];
	SVtoCgMatrix4x4 (var, matrix);
	setter.MatrixParameter4x4 (slot, param, matrix);
      }
      break;
    case CG_ARRAY:
      {
	if (var->GetArraySize () == 0) 
	  break;
	  
	size_t numElements = csMin (sparam->arrayItems.GetSize(),
				    var->GetArraySize ());
	if (numElements == 0) 
	  break;
	  
	for (size_t idx = 0; idx < numElements; idx++)
	{
	  csShaderVariable *element =
	    var->GetArrayElement (idx);
	  if (element != 0)
	    SetParameterValue (setter, sparam->arrayItems[idx], element);
	}
      }
      break;
    default:
      CS_ASSERT_MSG("Don't support CG param type (yet)", false);
  }
}

void csShaderGLCGCommon::SVtoCgMatrix3x3  (csShaderVariable* var, float* matrix)
{
  if (var->GetType () == csShaderVariable::MATRIX)
  {
    csMatrix3 m;
    if (var->GetValue (m))
    {
      CS::PluginCommon::MakeGLMatrix3x3 (m, matrix);
    }
  }
  else if (var->GetType () == csShaderVariable::TRANSFORM)
  {
    csReversibleTransform t;
    if (var->GetValue (t))
    {
      CS::PluginCommon::MakeGLMatrix3x3 (t.GetO2T(), matrix);
    }
  }
  else if (var->GetType () == csShaderVariable::ARRAY)
  {
    if (var->GetArraySize () != 3)
      return;

    csVector3 v;
    for (uint idx = 0; idx < var->GetArraySize (); idx++)
    {
      csShaderVariable *element =
	var->GetArrayElement (idx);
      if (element != 0 && element->GetValue (v))
      {
	matrix[idx] = v[0]; 
	matrix[idx + 3] = v[1];
	matrix[idx + 6] = v[2];
      }
    }
  }
  else
  {
    CS_ASSERT_MSG("Can't convert all SV contents to FLOAT3x3 (yet)", false);
    memset (matrix, 0, 9 * sizeof (float));
  }
}

void csShaderGLCGCommon::SVtoCgMatrix4x4  (csShaderVariable* var, float* matrix)
{
  if (var->GetType () == csShaderVariable::MATRIX)
  {
    csMatrix3 m;
    if (var->GetValue (m))
    {
      makeGLMatrix (m, matrix);
    }
  }
  else if (var->GetType () == csShaderVariable::TRANSFORM)
  {
    csReversibleTransform t;
    if (var->GetValue (t))
    {
      makeGLMatrix (t, matrix);
    }
  }
  else if (var->GetType () == csShaderVariable::ARRAY)
  {
    if (var->GetArraySize () != 4)
      return;

    csVector4 v;
    for (uint idx = 0; idx < var->GetArraySize (); idx++)
    {
      csShaderVariable *element =
	var->GetArrayElement (idx);
      if (element != 0 && element->GetValue (v))
      {
	matrix[idx] = v[0]; 
	matrix[idx + 4] = v[1];
	matrix[idx + 8] = v[2];
	matrix[idx + 12] = v[3];
      }
    }
  }
  else
  {
    CS_ASSERT_MSG("Can't convert all SV contents to FLOAT4x4 (yet)", false);
    memset (matrix, 0, 16 * sizeof (float));
  }
}

template<typename Array, typename ParamSetter>
void csShaderGLCGCommon::ApplyVariableMapArray (const Array& array,
                                                const ParamSetter& setter,
                                                const csShaderVariableStack& stack)
{
  csRef<csShaderVariable> var;
  
  for(size_t i = 0; i < array.GetSize (); ++i)
  {
    const VariableMapEntry& mapping = array[i];
    
    var = GetParamSV (stack, mapping.mappingParam);
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    SetParameterValue (setter, param, var);
  }
}

struct SetterCg
{
  void Parameter1i (uint slot, CGparameter param, int v) const
  { cgSetParameter1i (param, v); }
  void Parameter1f (uint slot, CGparameter param, float v) const
  { cgSetParameter1f (param, v); }
  void Parameter2fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter2fv (param, v); }
  void Parameter3fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter3fv (param, v); }
  void Parameter4fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter4fv (param, v); }
  void MatrixParameter3x3 (uint slot, CGparameter param, float* v) const
  { cgGLSetMatrixParameterfc (param, v); }
  void MatrixParameter4x4 (uint slot, CGparameter param, float* v) const
  { cgGLSetMatrixParameterfc (param, v); }
};

template<GLenum Target, bool GP4Prog>
struct SetterARB : public SetterCg
{
  csGLExtensionManager* ext;

  SetterARB (csGLExtensionManager* ext) : ext (ext) {}

  void Parameter1i (uint slot, CGparameter param, int v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter1i (slot, param, v);
    else if (GP4Prog)
      ext->glProgramLocalParameterI4iNV (Target, slot, v, 0, 0, 0);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v, 0, 0, 0);
  }
  void Parameter1f (uint slot, CGparameter param, float v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter1f (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v, 0, 0, 0);
  }
  void Parameter2fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter2fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v[0], v[1], 0, 0);
  }
  void Parameter3fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter3fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v[0], v[1], v[2], 0);
  }
  void Parameter4fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter4fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fvARB (Target, slot, v);
  }
  void MatrixParameter3x3 (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::MatrixParameter4x4 (slot, param, v);
    else
    {
      float m4x4[16];
      m4x4[0] = v[0]; m4x4[1] = v[1]; m4x4[2] = v[2]; m4x4[3] = 0;
      m4x4[4] = v[3]; m4x4[5] = v[4]; m4x4[6] = v[5]; m4x4[7] = 0;
      m4x4[8] = v[6]; m4x4[9] = v[7]; m4x4[10] = v[8]; m4x4[11] = 0;
      m4x4[12] = 0; m4x4[13] = 0; m4x4[14] = 0; m4x4[15] = 1;
    
      if (ext->CS_GL_EXT_gpu_program_parameters)
        ext->glProgramLocalParameters4fvEXT (Target, slot, 4, m4x4);
      else
      {
	for (int i = 0; i < 4; i++)
	  ext->glProgramLocalParameter4fvARB (Target, slot+i, m4x4+i*4);
      }
    }
  }
  void MatrixParameter4x4 (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::MatrixParameter4x4 (slot, param, v);
    else
    {
      if (ext->CS_GL_EXT_gpu_program_parameters)
        ext->glProgramLocalParameters4fvEXT (Target, slot, 4, v);
      else
      {
	for (int i = 0; i < 4; i++)
	  ext->glProgramLocalParameter4fvARB (Target, slot+i, v+i*4);
      }
    }
  }
};

void csShaderGLCGCommon::SetupState (const CS::Graphics::RenderMesh* /*mesh*/,
                                     CS::Graphics::RenderMeshModes& /*modes*/,
                                     const csShaderVariableStack& stack)
{
  // set variables
  if ((programType == progVP) && (programProfile >= CG_PROFILE_ARBVP1))
  {
    if (programProfile >= CG_PROFILE_GPU_VP)
    {
      SetterARB<GL_VERTEX_PROGRAM_ARB, true> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
    else
    {
      SetterARB<GL_VERTEX_PROGRAM_ARB, false> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
  }
  else if ((programType == progFP) && (programProfile >= CG_PROFILE_ARBFP1))
  {
    if (programProfile >= CG_PROFILE_GPU_FP)
    {
      SetterARB<GL_FRAGMENT_PROGRAM_ARB, true> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
    else
    {
      SetterARB<GL_FRAGMENT_PROGRAM_ARB, false> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
  }
  else
  {
    SetterCg setter;
    ApplyVariableMapArray (variablemap, setter, stack);
  }
  
  /* "Assumed constant" parameters are set here b/c all needed shader 
   * vars are available */
  if (assumedConstParams != 0)
  {
    csRef<csShaderVariable> var;
  
    SetterCg setter;
    for(size_t i = 0; i < assumedConstParams->GetSize (); ++i)
    {
      VariableMapEntry& mapping = assumedConstParams->Get (i);
      
      var = GetParamSV (stack, mapping.mappingParam);
      // If var is null now we have no const nor any passed value, ignore it
      if (!var.IsValid ())
	continue;
  
      ShaderParameter* param =
	reinterpret_cast<ShaderParameter*> (mapping.userVal);
      SetParameterValue (setter, param, var);
      cgSetParameterVariability (param->param, CG_LITERAL);
      FreeShaderParam (param);
    }
    delete assumedConstParams; assumedConstParams = 0;
    cgCompileProgram (program);
    if (shaderPlug->debugDump)
      DoDebugDump();
    PostCompileVmapProcess ();
  }
}

void csShaderGLCGCommon::ResetState()
{
}

void csShaderGLCGCommon::EnsureDumpFile()
{
  if (debugFN.IsEmpty())
  {
    static int programCounter = 0;
    
    const char* progTypeStr ="";
    switch (programType)
    {
      case progVP: progTypeStr = "cgvp"; break;
      case progFP: progTypeStr = "cgfp"; break;
    }
    
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
    csString filename;
    filename << shaderPlug->dumpDir << (programCounter++) << 
      progTypeStr << ".txt";
    debugFN = filename;
    vfs->DeleteFile (debugFN);
  }
}

void csShaderGLCGCommon::FreeShaderParam (ShaderParameter* sparam)
{
  if (sparam == 0) return;
  for (size_t i = 0; i < sparam->arrayItems.GetSize(); i++)
  {
    FreeShaderParam (sparam->arrayItems[i]);
  }
  shaderPlug->paramAlloc.Free (sparam);
}

void csShaderGLCGCommon::FillShaderParam (ShaderParameter* sparam, 
                                          CGparameter param)
{
  sparam->param = param;
  sparam->paramType = cgGetParameterType (param);
  if (sparam->paramType == CG_ARRAY)
  {
    size_t arraySize = cgGetArraySize (param, 0);
    for (size_t i = arraySize; i-- > 0; )
    {
      CGparameter element = cgGetArrayParameter (param, int (i));
      ShaderParameter* newsparam = shaderPlug->paramAlloc.Alloc();
      FillShaderParam (newsparam, element);
      sparam->arrayItems.Put (i, newsparam);
    }
  }
}

void csShaderGLCGCommon::GetShaderParamSlot (ShaderParameter* sparam)
{
  if (cgGetParameterType (sparam->param) != CG_ARRAY)
  {
    CGresource rsc = cgGetParameterResource (sparam->param);
    if ((rsc == CG_C) || (rsc == CG_GLSL_UNIFORM))
    {
      sparam->baseSlot = cgGetParameterResourceIndex (sparam->param);
    }
  }
  for (size_t i = 0; i < sparam->arrayItems.GetSize(); i++)
  {
    if (sparam->arrayItems[i] != 0)
      GetShaderParamSlot (sparam->arrayItems[i]);
  }
}

void csShaderGLCGCommon::PostCompileVmapProcess ()
{
  for(size_t i = 0; i < variablemap.GetSize (); )
  {
    VariableMapEntry& mapping = variablemap[i];
    ShaderParameter* sparam =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    
    if (!PostCompileVmapProcess (sparam))
    {
      variablemap.DeleteIndex (i);
      FreeShaderParam (sparam);
      continue;
    }
    
    GetShaderParamSlot (sparam);
    ++i;
  }
}

bool csShaderGLCGCommon::PostCompileVmapProcess (ShaderParameter* sparam)
{
  CGparameter param = sparam->param;
  if (sparam->paramType == CG_ARRAY)
  {
    bool ret = false;
    for (size_t i = sparam->arrayItems.GetSize(); i-- > 0; )
    {
      if (!PostCompileVmapProcess (sparam->arrayItems[i]))
      {
        if (i == sparam->arrayItems.GetSize()-1)
          sparam->arrayItems.Truncate (i);
        else
        {
          FreeShaderParam (sparam->arrayItems[i]);
          sparam->arrayItems[i] = 0;
        }
      }
      else
      {
        ret |= true;
      }
    }
    if (ret) sparam->arrayItems.ShrinkBestFit();
    return ret;
  }
  else
    return cgIsParameterReferenced (param);
}

bool csShaderGLCGCommon::DefaultLoadProgram (
  iShaderDestinationResolverCG* cgResolve,
  const char* programStr, CGGLenum type, CGprofile maxProfile, 
  uint flags)
{
  if (!programStr || !*programStr) return false;

  size_t i;
  csString augmentedProgramStr;
  const csSet<csString>* unusedParams = &this->unusedParams;
  if (cgResolve != 0)
  {
    unusedParams = &cgResolve->GetUnusedParameters ();
  }
  csSet<csString>::GlobalIterator unusedIt (unusedParams->GetIterator());
  while (unusedIt.HasNext())
  {
    const csString& param = unusedIt.Next ();
    augmentedProgramStr.AppendFmt ("#define %s\n",
      param.GetData());
  }
  augmentedProgramStr.Append (programStr);
  programStr = augmentedProgramStr;

  CGprofile profile = CG_PROFILE_UNKNOWN;

  if (!cg_profile.IsEmpty())
    profile = cgGetProfile (cg_profile);

  if(profile == CG_PROFILE_UNKNOWN)
    profile = cgGLGetLatestProfile (type);

  if (maxProfile != CG_PROFILE_UNKNOWN)
    profile = csMin (profile, maxProfile);

  if (shaderPlug->doVerbose)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg %s program '%s': using profile %s[%d]", GetProgramType(),
      description.GetData (), cgGetProfileString (profile), profile);
  }

  ArgumentArray args;
  shaderPlug->GetProfileCompilerArgs (GetProgramType(), profile, args);
  for (i = 0; i < compilerArgs.GetSize(); i++) 
    args.Push (compilerArgs[i]);
  /* Work around Cg 2.0 bug: it emits "OPTION ARB_position_invariant;"
     AND computes result.position in the VP - doing both is verboten.
     Remedy: remove -posinv argument 
     (cgc version 2.0.0010)
   */
  if (strcmp (cgGetProfileString (profile), "gp4vp") == 0)
  {
    for (i = 0; i < args.GetSize(); ) 
    {
      if (strcmp (args[i], "-posinv") == 0)
	args.DeleteIndex (i);
      else
	i++;
    }
  }
  args.Push (0);
 
  if (program)
  {
    cgDestroyProgram (program);
  }
  shaderPlug->SetCompiledSource (programStr);
  program = cgCreateProgram (shaderPlug->context, 
    (flags & loadPrecompiled) ? CG_OBJECT : CG_SOURCE, programStr, 
    profile, !entrypoint.IsEmpty() ? entrypoint : "main", args.GetArray());
  
  if (!(flags & loadIgnoreErrors)) shaderPlug->PrintAnyListing();

  if (!program)
  {
    shaderPlug->SetCompiledSource (0);
    /*if (shaderPlug->debugDump)
    {
      EnsureDumpFile();
      WriteAdditionalDumpInfo ("Failed program source", programStr);
    }*/
    return false;
  }
  programProfile = cgGetProgramProfile (program);

  if (flags & loadApplyVmap)
  {
    SetterCg setter;
  
    i = 0;
    while (i < variablemap.GetSize ())
    {
      // Get the Cg parameter
      CGparameter param = cgGetNamedParameter (program, 
	variablemap[i].destination);
      ShaderParameter* sparam =
	reinterpret_cast<ShaderParameter*> (variablemap[i].userVal);
  
      if (!param /*||
	  (cgGetParameterType (param) != CG_ARRAY && !cgIsParameterReferenced (param))*/)
      {
	variablemap.DeleteIndex (i);
	FreeShaderParam (sparam);
	continue;
      }
      FillShaderParam (sparam, param);
      bool assumeConst = sparam->assumeConstant;
      if (assumeConst)
      {
        if (assumedConstParams == 0)
          assumedConstParams = new csSafeCopyArray<VariableMapEntry>;
	assumedConstParams->Push (variablemap[i]);
	variablemap.DeleteIndex (i);
	continue;
      }
      // Mark constants as to be folded in
      if (variablemap[i].mappingParam.IsConstant())
      {
	csShaderVariable* var = variablemap[i].mappingParam.var;
	if (var != 0)
	  SetParameterValue (setter, sparam, var);
	cgSetParameterVariability (param, CG_LITERAL);
	variablemap.DeleteIndex (i);
	FreeShaderParam (sparam);
	continue;
      }
      i++;
    }
  
    variablemap.ShrinkBestFit();
  }

  if (flags & loadIgnoreErrors) shaderPlug->SetIgnoreErrors (true);
  cgCompileProgram (program);
  if (flags & loadIgnoreErrors)
    shaderPlug->SetIgnoreErrors (false);
  else
    shaderPlug->PrintAnyListing();
  
  if (flags & loadApplyVmap)
    PostCompileVmapProcess ();

  if (flags & loadLoadToGL)
  {
    cgGetError(); // Clear error
    cgGLLoadProgram (program);
    if (!(flags & loadIgnoreErrors)) shaderPlug->PrintAnyListing();
    if ((cgGetError() != CG_NO_ERROR)
      || !cgGLIsProgramLoaded (program)) 
    {
      //if (shaderPlug->debugDump)
	//DoDebugDump();

      if (shaderPlug->doVerbose
	  && ((type == CG_GL_VERTEX) && (profile >= CG_PROFILE_ARBVP1))
	    || ((type == CG_GL_FRAGMENT) && (profile >= CG_PROFILE_ARBFP1)))
      {
	const char* err = (char*)glGetString (GL_PROGRAM_ERROR_STRING_ARB);
	shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	  "OpenGL error string: %s", err);
      }

      shaderPlug->SetCompiledSource (0);
      return false;
    }
  }

  if (shaderPlug->debugDump)
    DoDebugDump();
  
  shaderPlug->SetCompiledSource (0);

  bool result = true;
  if (programType == progFP)
  {
    int numVaryings = 0;
    CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
    while (param)
    {
      if (cgIsParameterUsed (param, program)
	&& cgIsParameterReferenced (param))
      {
	const CGenum var = cgGetParameterVariability (param);
	if (var == CG_VARYING)
	  numVaryings++;
      }
  
      param = cgGetNextLeafParameter (param);
    }
    
    /* WORKAROUND: Even NVs G80 doesn't support passing more than 16 attribs
       into an FP, yet Cg happily generates code that uses more (and GL falls 
       back to SW).
       So manually check the number of varying inputs and reject more than 16.
       
       @@@ This should be at least configurable
     */
    result = numVaryings <= 16;
  }
  if (!result && !debugFN.IsEmpty())
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
    vfs->DeleteFile (debugFN);
  }
  return result;
}

void csShaderGLCGCommon::DoDebugDump ()
{
  csString output;
  DumpProgramInfo (output);
  output << "CG program type: " << programType << "\n";
  output << "CG profile: " << cgGetProgramString (program, 
    CG_PROGRAM_PROFILE) << "\n";
  output << "CG entry point: " << (entrypoint ? entrypoint : "main") << 
    "\n";
  output << "CG program valid: " << validProgram << "\n";
  output << "\n";

  output << "Variable mappings:\n";
  DumpVariableMappings (output);
  output << "\n";

  CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
  while (param)
  {
    output << "Parameter: " << cgGetParameterName (param) << "\n";
    output << " Type: " << 
      cgGetTypeString (cgGetParameterNamedType (param)) << "\n";
    output << " Direction: " <<
      cgGetEnumString (cgGetParameterDirection (param)) << "\n";
    output << " Semantic: " << cgGetParameterSemantic (param) << "\n";
    const CGenum var = cgGetParameterVariability (param);
    output << " Variability: " << cgGetEnumString (var) << "\n";
    output << " Resource: " <<
      cgGetResourceString (cgGetParameterResource (param)) << "\n";
    output << " Resource index: " <<
      cgGetParameterResourceIndex (param) << "\n";
    // Cg 2.0 seems to not like CG_DEFAULT for uniforms
    if (/*(var == CG_UNIFORM) || */(var == CG_CONSTANT))
    {
      int nValues;
      const double* values = cgGetParameterValues (param, 
	(var == CG_UNIFORM) ? CG_DEFAULT : CG_CONSTANT, &nValues);
      if (nValues != 0)
      {
	output << " Values:";
	for (int v = 0; v < nValues; v++)
	{
	  output << ' ' << values[v];
	}
	output << "\n";
      }
    }
    if (!cgIsParameterUsed (param, program)) output << "  not used\n";
    if (!cgIsParameterReferenced (param)) output << "  not referenced\n";

    param = cgGetNextLeafParameter (param);
  }
  output << "\n";

  output << "Program source:\n";
  output << cgGetProgramString (program, CG_PROGRAM_SOURCE);
  output << "\n";

  output << "Compiled program:\n";
  output << cgGetProgramString (program, CG_COMPILED_PROGRAM);
  output << "\n";

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
  EnsureDumpFile();

  csRef<iFile> debugFile = vfs->Open (debugFN, VFS_FILE_APPEND);
  if (!debugFile)
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not write '%s'", debugFN.GetData());
  }
  else
  {
    debugFile->Write (output.GetData(), output.Length ());
    csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, 
      "crystalspace.graphics3d.shader.glcg",
      "Dumped Cg program info to '%s'", debugFN.GetData());
  }
}

void csShaderGLCGCommon::WriteAdditionalDumpInfo (const char* description, 
						  const char* content)
{
  if (!shaderPlug->debugDump || !debugFN) return;

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
  csRef<iDataBuffer> oldDump = vfs->ReadFile (debugFN, true);

  csString output (oldDump ? (char*)oldDump->GetData() : 0);
  output << description << ":\n";
  output << content;
  output << "\n";
  if (!vfs->WriteFile (debugFN, output.GetData(), output.Length ()))
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not augment '%s'", debugFN.GetData());
  }
}

bool csShaderGLCGCommon::Load (iShaderDestinationResolver* resolve, 
			       iDocumentNode* program)
{
  if(!program)
    return false;

  csRef<iShaderManager> shadermgr = 
  	csQueryRegistry<iShaderManager> (shaderPlug->object_reg);

  const char* progTypeNode = 0;
  switch (programType)
  {
    case progVP: progTypeNode = "cgvp"; break;
    case progFP: progTypeNode = "cgfp"; break;
  }
  csRef<iDocumentNode> variablesnode = program->GetNode (progTypeNode);
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_PROFILE:
          cg_profile = child->GetContentsValue ();
          break;
        case XMLTOKEN_ENTRY:
          entrypoint = child->GetContentsValue ();
          break;
        case XMLTOKEN_COMPILERARGS:
          shaderPlug->SplitArgsString (child->GetContentsValue (), 
            compilerArgs);
          break;
	case XMLTOKEN_VARIABLEMAP:
	  {
	    //@@ REWRITE
	    const char* destname = child->GetAttributeValue ("destination");
	    if (!destname)
	    {
	      synsrv->Report ("crystalspace.graphics3d.shader.common",
		CS_REPORTER_SEVERITY_WARNING, child,
		"<variablemap> has no 'destination' attribute");
	      return false;
	    }
	    
	    bool assumeConst = child->GetAttributeValueAsBool ("assumeconst",
	      false);
    
	    const char* varname = child->GetAttributeValue ("variable");
	    if (!varname)
	    {
	      // "New style" variable mapping
	      VariableMapEntry vme (CS::InvalidShaderVarStringID, destname);
	      if (!ParseProgramParam (child, vme.mappingParam,
		ParamFloat | ParamVector2 | ParamVector3 | ParamVector4))
		return false;
	      ShaderParameter* sparam = shaderPlug->paramAlloc.Alloc();
	      sparam->assumeConstant = assumeConst;
	      vme.userVal = reinterpret_cast<intptr_t> (sparam);
	      variablemap.Push (vme);
	    }
	    else
	    {
	      // "Classic" variable mapping
	      CS::Graphics::ShaderVarNameParser nameParse (varname);
	      VariableMapEntry vme (
		stringsSvName->Request (nameParse.GetShaderVarName()),
		destname);
	      for (size_t n = 0; n < nameParse.GetIndexNum(); n++)
	      {
		vme.mappingParam.indices.Push (nameParse.GetIndexValue (n));
	      }
	      ShaderParameter* sparam = shaderPlug->paramAlloc.Alloc();
	      sparam->assumeConstant = assumeConst;
	      vme.userVal = reinterpret_cast<intptr_t> (sparam);
	      variablemap.Push (vme);
	    }
	  }
	  break;
        default:
	  if (!ParseCommon (child))
	    return false;
      }
    }
  }

  cgResolve = scfQueryInterfaceSafe<iShaderDestinationResolverCG> (resolve);

  return true;
}

void csShaderGLCGCommon::CollectUnusedParameters (csSet<csString>& unusedParams)
{
  CGparameter cgParam = cgGetFirstLeafParameter (program, CG_PROGRAM);
  while (cgParam)
  {
    if (!cgIsParameterUsed (cgParam, program))
    {
      csString param (cgGetParameterName (cgParam));
      for (size_t j = 0; j < param.Length(); j++)
      {
        if ((param[j] == '.') || (param[j] == '[') || (param[j] == ']'))
          param[j] = '_';
      }
      csString s;
      s.Format ("PARAM_%s_UNUSED", param.GetData());
      unusedParams.Add (s);
    }

    cgParam = cgGetNextLeafParameter (cgParam);
  }
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
