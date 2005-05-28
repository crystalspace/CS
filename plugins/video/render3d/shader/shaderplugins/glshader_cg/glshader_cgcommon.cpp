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
#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "csplugincommon/opengl/glhelper.h"
#include "glshader_cg.h"
#include "glshader_cgcommon.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGCommon);

csShaderGLCGCommon::csShaderGLCGCommon (csGLShader_CG* shaderPlug, 
					const char* type) :
  csShaderProgram (shaderPlug->object_reg), programType (type)
{
  validProgram = true;
  this->shaderPlug = shaderPlug;
  program = 0;
  cg_profile = 0;
  entrypoint = 0;
  debugFN = 0;

  InitTokenTable (xmltokens);
}

csShaderGLCGCommon::~csShaderGLCGCommon ()
{
  if (program)
    cgDestroyProgram (program);
  delete[] cg_profile;
  delete[] entrypoint;
  delete[] debugFN;
}

void csShaderGLCGCommon::Activate()
{
  cgGLEnableProfile (cgGetProgramProfile (program));
  cgGLBindProgram (program);
}

void csShaderGLCGCommon::Deactivate()
{
  cgGLDisableProfile (cgGetProgramProfile (program));
}

void csShaderGLCGCommon::SetupState (const csRenderMesh* mesh,
                                     csRenderMeshModes& modes,
                                     const csShaderVarStack &stacks)
{
  size_t i;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];

    if (RetrieveParamValue (mapping.mappingParam, stacks))
    {
      CGparameter param = (CGparameter)mapping.userVal;

      if (mapping.mappingParam.var.IsValid())
      {
	csShaderVariable* lvar = mapping.mappingParam.var;
	switch (lvar->GetType())
	{
	  case csShaderVariable::INT:
	    {
	      int intval;
	      if(lvar->GetValue(intval))
		cgGLSetParameter1f (param, (float)intval);
	    }
	    break;
	  case csShaderVariable::FLOAT:
	    {
	      float fval;
	      if(lvar->GetValue(fval))
		cgGLSetParameter1f (param, fval);
	    }
	    break;
	  case csShaderVariable::VECTOR2:
	    {
	      csVector2 v2;
	      if(lvar->GetValue(v2))
		cgGLSetParameter2f(param, v2.x, v2.y);
	    }
	    break;    
	  case csShaderVariable::COLOR:
	  case csShaderVariable::VECTOR3:
	    {
	      csVector3 v3;
	      if(lvar->GetValue(v3))
		cgGLSetParameter3f(param, v3.x, v3.y, v3.z);
	    }
	    break;
	  case csShaderVariable::VECTOR4:
	    {
	      csVector4 v4;
	      if(lvar->GetValue(v4))
		cgGLSetParameter4f(param, v4.x, v4.y, v4.z, v4.w);
	    }
	    break;
          case csShaderVariable::MATRIX:
	    {
	      csMatrix3 m;
	      if(lvar->GetValue(m))
              {
                float matrix[16];
                makeGLMatrix (m, matrix);
		cgGLSetParameter4fv(param, matrix);
              }
	    }
	    break;
          case csShaderVariable::TRANSFORM:
	    {
	      csReversibleTransform t;
	      if(lvar->GetValue(t))
              {
                float matrix[16];
                makeGLMatrix (t, matrix);
		cgGLSetParameter4fv(param, matrix);
              }
	    }
	    break;
	  default:
	    break;
	}
      }
      else
      {
	const csVector4& v = mapping.mappingParam.vectorValue;
        float matrix[16];
	switch (mapping.mappingParam.type)
	{
	  case ParamFloat:
	    cgGLSetParameter1f (param, v.x);
	    break;
	  case ParamVector2:
	    cgGLSetParameter2f (param, v.x, v.y);
	    break;
	  case ParamVector3:
	    cgGLSetParameter3f (param, v.x, v.y, v.z);
	    break;
	  case ParamVector4:
	    cgGLSetParameter4f (param, v.x, v.y, v.z, v.w);
	    break;
	  case ParamMatrix:
            makeGLMatrix (mapping.mappingParam.matrixValue, matrix);
	    cgGLSetParameter4fv (param, matrix);
	    break;
	  case ParamTransform:
            makeGLMatrix (mapping.mappingParam.transformValue, matrix);
	    cgGLSetParameter4fv (param, matrix);
	    break;
	  default:
	    break;
	}
      }
    }
  }
}

void csShaderGLCGCommon::ResetState()
{
}

bool csShaderGLCGCommon::DefaultLoadProgram (const char* programStr, 
  CGGLenum type, csArray<iShaderVariableContext*> &staticContexts)
{
  csShaderVariable *var;
  size_t i, j;

  CGprofile profile = CG_PROFILE_UNKNOWN;

  if (cg_profile != 0)
    profile = cgGetProfile (cg_profile);

  if(profile == CG_PROFILE_UNKNOWN)
    profile = cgGLGetLatestProfile (type);

  if (shaderPlug->doVerbose)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg program '%s': using profile %s", description, 
      cgGetProfileString (profile));
  }

  program = cgCreateProgram (shaderPlug->context, CG_SOURCE,
    programStr, profile, entrypoint ? entrypoint : "main", 
    GetProfileCompilerArgs (profile));

  if (!program)
    return false;

  cgGLLoadProgram (program);

  if (shaderPlug->debugDump)
    DoDebugDump();

  i = 0;
  while (i < variablemap.Length ())
  {
    // Get the Cg parameter
    CGparameter param = cgGetNamedParameter (program, 
      variablemap[i].destination);
    // Check if it's found, and just skip it if not.
    if (!param)
    {
      i++;
      continue;
    }

    if (!cgIsParameterReferenced (param))
    {
      variablemap.DeleteIndex (i);
      continue;
    }
    variablemap[i].userVal = (intptr_t)param;

    // Check if we've got it locally
    var = svcontext.GetVariable(variablemap[i].name);
    if (!var)
    {
      // If not, check the static contexts
      for (j=0;j<staticContexts.Length();j++)
      {
        var = staticContexts[j]->GetVariable (variablemap[i].name);
        if (var) break;
      }
    }
    if (var)
    {
      // We found it, so we add it as a static mapping
      variablemap[i].mappingParam.var = var; // statlink = var;
    }
    i++;
  }

  variablemap.ShrinkBestFit();

  return true;
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
    if ((var == CG_UNIFORM) || (var == CG_CONSTANT))
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

    param = cgGetNextLeafParameter (param);
  }
  output << "\n";

  output << "Program source:\n";
  output << cgGetProgramString (program, CG_PROGRAM_SOURCE);
  output << "\n";

  output << "Compiled program:\n";
  output << cgGetProgramString (program, CG_COMPILED_PROGRAM);
  output << "\n";

  if (!debugFN)
  {
    static int programCounter = 0;
    csString filename;
    filename << shaderPlug->dumpDir << (programCounter++) << programType << 
      ".txt";
    debugFN = csStrNew (filename);
  }

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objectReg, iVFS);
  if (!vfs->WriteFile (debugFN, output.GetData(), output.Length()))
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not write '%s'", debugFN);
  }
  else
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, 
      "crystalspace.graphics3d.shader.glcg",
      "Dumped Cg program info to '%s'", debugFN);
  }
}

void csShaderGLCGCommon::WriteAdditionalDumpInfo (const char* description, 
						  const char* content)
{
  if (!shaderPlug->debugDump || !debugFN) return;

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objectReg, iVFS);
  csRef<iDataBuffer> oldDump = vfs->ReadFile (debugFN, true);

  csString output ((char*)oldDump->GetData());
  output << description << ":\n";
  output << content;
  output << "\n";
  if (!vfs->WriteFile (debugFN, output.GetData(), output.Length()))
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not augment '%s'", debugFN);
  }
}

bool csShaderGLCGCommon::Load (iShaderTUResolver*, iDocumentNode* program)
{
  if(!program)
    return false;

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(
  	shaderPlug->object_reg, iShaderManager);

  csRef<iDocumentNode> variablesnode = program->GetNode (programType);
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
	  delete[] cg_profile;
          cg_profile = csStrNew (child->GetContentsValue ());
          break;
        case XMLTOKEN_ENTRY:
	  delete[] entrypoint;
          entrypoint = csStrNew (child->GetContentsValue ());
          break;
        default:
	  if (!ParseCommon (child))
	    return false;
      }
    }
  }

  return true;
}
