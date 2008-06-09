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

static const int assumeConstFlag = 0x80000000;

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
      
      shaderPlug->paramAlloc.Free (param);
    }
    delete assumedConstParams;
  }

  for(size_t i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];
    
    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    
    shaderPlug->paramAlloc.Free (param);
  }
}

void csShaderGLCGCommon::Activate()
{
  cgGLEnableProfile (programProfile);
  if (!cgGLIsProgramLoaded (program)) cgGLLoadProgram (program);
  cgGLBindProgram (program);
}

void csShaderGLCGCommon::Deactivate()
{
  cgGLDisableProfile (programProfile);
}

void csShaderGLCGCommon::SetParameterValue (ShaderParameter* sparam,
                                            csShaderVariable* var)
{
  CGparameter param = sparam->param;
  CGtype paramType = sparam->paramType;
  
  switch (paramType)
  {
    case CG_INT:
      {
	int i;
	if (var->GetValue (i))
	  cgGLSetParameter1f (param, i);
      }
    case CG_FLOAT:
      {
	float f;
	if (var->GetValue (f))
	  cgGLSetParameter1f (param, f);
      }
    case CG_FLOAT2:
      {
	csVector2 v;
	if (var->GetValue (v))
	  cgGLSetParameter2f (param, v.x, v.y);
      }
      break;
    case CG_FLOAT3:
      {
	csVector3 v;
	if (var->GetValue (v))
	  cgGLSetParameter3f (param, v.x, v.y, v.z);
      }
      break;
    case CG_FLOAT4:
      {
	csVector4 v;
	if (var->GetValue (v))
	  cgGLSetParameter4f (param, v.x, v.y, v.z, v.w);
      }
      break;
    case CG_FLOAT4x4:
      {
	float matrix[16];
	SVtoCgMatrix4x4 (var, matrix);
	cgGLSetMatrixParameterfc (param, matrix);
      }
      break;
    case CG_ARRAY:
      {
	CGtype innerType = sparam->arrayInnerType;
	if (var->GetArraySize () == 0) 
	  break;

	uint numElements = csMin ((uint)cgGetArraySize (param, 0), 
				  sparam->arraySize);

	if (numElements == 0) 
	  break;

	const csShaderVariable *cvar = var->GetArrayElement (0);
	if (cvar == 0)
	  break;
	
	CS_ALLOC_STACK_ARRAY (float, tmpArr, numElements * 16);
	uint idx = 0;

	switch (innerType)
	{
	  case CG_FLOAT:
	    {
	      float f;
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0 && element->GetValue (f))
		  tmpArr[idx] = f;
	      }
	      cgGLSetParameterArray1f (param, 0, numElements, tmpArr);
	    }
	    break;
	  case CG_FLOAT2:
	    {
	      csVector2 v;
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0 && element->GetValue (v))
		{
		  tmpArr[2*idx+0] = v.x;
		  tmpArr[2*idx+1] = v.y;
		}
	      }
	      cgGLSetParameterArray2f (param, 0, numElements, tmpArr);
	    }
	    break;
	  case CG_FLOAT3:
	    {
	      csVector3 v;
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0 && element->GetValue (v))
		{
		  tmpArr[3*idx+0] = v.x;
		  tmpArr[3*idx+1] = v.y;
		  tmpArr[3*idx+2] = v.z;
		}
	      }
	      cgGLSetParameterArray3f (param, 0, numElements, tmpArr);
	    }
	    break;
	  case CG_FLOAT4:
	    {
	      csVector4 v;
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0 && element->GetValue (v))
		{
		  tmpArr[4*idx+0] = v.x;
		  tmpArr[4*idx+1] = v.y;
		  tmpArr[4*idx+2] = v.z;
		  tmpArr[4*idx+3] = v.w;
		}
	      }
	      cgGLSetParameterArray4f (param, 0, numElements, tmpArr);
	    }
	    break;
	  case CG_FLOAT3x3:
	    {
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0)
		{
		  SVtoCgMatrix3x3 (element, &tmpArr[9*idx]);
		}
	      }
	      cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
	    }
	    break;
	  case CG_FLOAT4x4:
	    {
	      for (idx = 0; idx < numElements; idx++)
	      {
		csShaderVariable *element =
		  var->GetArrayElement (idx);
		if (element != 0)
		{
		  SVtoCgMatrix4x4 (element, &tmpArr[16*idx]);
		}
	      }
	      cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
	    }
	    break;
	  default:
	    CS_ASSERT_MSG("Don't support CG param type (yet)", false);
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

void csShaderGLCGCommon::SetupState (const CS::Graphics::RenderMesh* /*mesh*/,
                                     CS::Graphics::RenderMeshModes& /*modes*/,
                                     const csShaderVariableStack& stack)
{
  size_t i;
  csRef<csShaderVariable> var;

  // set variables
  for(i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];
    
    var = GetParamSV (stack, mapping.mappingParam);
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    SetParameterValue (param, var);
  }
  
  /* "Assumed constant" parameters are set here b/c all needed shader 
   * vars are available */
  if (assumedConstParams != 0)
  {
    for(i = 0; i < assumedConstParams->GetSize (); ++i)
    {
      VariableMapEntry& mapping = assumedConstParams->Get (i);
      
      var = GetParamSV (stack, mapping.mappingParam);
      // If var is null now we have no const nor any passed value, ignore it
      if (!var.IsValid ())
	continue;
  
      ShaderParameter* param =
	reinterpret_cast<ShaderParameter*> (mapping.userVal);
      SetParameterValue (param, var);
      cgSetParameterVariability (param->param, CG_LITERAL);
      shaderPlug->paramAlloc.Free (param);
    }
    delete assumedConstParams; assumedConstParams = 0;
    cgCompileProgram (program);
    if (shaderPlug->debugDump)
      DoDebugDump();
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
    i = 0;
    while (i < variablemap.GetSize ())
    {
      // Get the Cg parameter
      CGparameter param = cgGetNamedParameter (program, 
	variablemap[i].destination);
  
      if (!param ||
	  (cgGetParameterType (param) != CG_ARRAY && !cgIsParameterReferenced (param)))
      {
	variablemap.DeleteIndex (i);
	continue;
      }
      ShaderParameter* sparam =
	reinterpret_cast<ShaderParameter*> (variablemap[i].userVal);
      sparam->param = param;
      sparam->paramType = cgGetParameterType (param);
      if (sparam->paramType == CG_ARRAY)
      {
        sparam->arrayInnerType = cgGetArrayType (param);
        sparam->arraySize = cgGetArraySize (param, 0);
      }
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
	  SetParameterValue (sparam, var);
	cgSetParameterVariability (param, CG_LITERAL);
	variablemap.DeleteIndex (i);
	shaderPlug->paramAlloc.Free (sparam);
	continue;
      }
      i++;
    }
  
    variablemap.ShrinkBestFit();
  }

  if (assumedConstParams == 0)
  {
    if (flags & loadIgnoreErrors) shaderPlug->SetIgnoreErrors (true);
    cgCompileProgram (program);
    if (flags & loadIgnoreErrors)
      shaderPlug->SetIgnoreErrors (false);
    else
      shaderPlug->PrintAnyListing();
  }

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

  const char* progTypeNode;
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
