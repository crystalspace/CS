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
  csRef<csShaderVariable> var;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];
    
    var = csGetShaderVariableFromStack (stacks, mapping.name);
    if (!var.IsValid ())
      var = mapping.mappingParam.var;

    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    CGparameter param = (CGparameter)mapping.userVal;
    
    switch (var->GetType ())
    {
    case csShaderVariable::INT:
    case csShaderVariable::FLOAT:
      {
        float f;
        if (var->GetValue (f))
          cgGLSetParameter1f (param, f);
      }
      break;
    case csShaderVariable::VECTOR2:
      {
        csVector2 v;
        if (var->GetValue (v))
          cgGLSetParameter2f (param, v.x, v.y);
      }
      break;
    case csShaderVariable::COLOR:
    case csShaderVariable::VECTOR3:
      {
        csVector3 v;
        if (var->GetValue (v))
          cgGLSetParameter3f (param, v.x, v.y, v.z);
      }
      break;
    case csShaderVariable::VECTOR4:
      {
        csVector4 v;
        if (var->GetValue (v))
          cgGLSetParameter4f (param, v.x, v.y, v.z, v.w);
      }
      break;
    case csShaderVariable::MATRIX:
      {
        csMatrix3 m;
        if (var->GetValue (m))
        {
          float matrix[16];
          makeGLMatrix (m, matrix);
          cgGLSetMatrixParameterfc (param, matrix);
        }
      }
      break;
    case csShaderVariable::TRANSFORM:
      {
        csReversibleTransform t;
        if (var->GetValue (t))
        {
          float matrix[16];
          makeGLMatrix (t, matrix);
          cgGLSetMatrixParameterfc (param, matrix);
        }
      }
      break;
    case csShaderVariable::ARRAY:
      {
        //Array..
        if (cgGetParameterType (param) != CG_ARRAY)
          break;

        if (var->GetArraySize () == 0) 
          break;

        uint numElements = csMin ((uint)cgGetArraySize (param, 0), 
                                  (uint)var->GetArraySize ());

        if (numElements == 0) 
          break;

        // Do type-specific packing depending on first var
        const csShaderVariable *cvar = var->GetArrayElement (0);
        
        CS_ALLOC_STACK_ARRAY (float, tmpArr, numElements * 16);
        uint idx = 0;

        switch (cvar->GetType ())
        {
        case csShaderVariable::INT:
        case csShaderVariable::FLOAT:
          {
            float f;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (f))
                tmpArr[idx] = f;
            }
            cgGLSetParameterArray1f (param, 0, numElements, tmpArr);
          }
          break;
        case csShaderVariable::VECTOR2:
          {
            csVector2 v;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (v))
              {
                tmpArr[2*idx+0] = v.x;
                tmpArr[2*idx+1] = v.y;
              }
            }
            cgGLSetParameterArray2f (param, 0, numElements, tmpArr);
          }
          break;
        case csShaderVariable::COLOR:
        case csShaderVariable::VECTOR3:
          {
            csVector3 v;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (v))
              {
                tmpArr[3*idx+0] = v.x;
                tmpArr[3*idx+1] = v.y;
                tmpArr[3*idx+2] = v.z;
              }
            }
            cgGLSetParameterArray3f (param, 0, numElements, tmpArr);
          }
          break;
        case csShaderVariable::VECTOR4:
          {
            csVector4 v;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (v))
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
        case csShaderVariable::MATRIX:
          {
            csMatrix3 m;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (m))
              {
                makeGLMatrix (m, &tmpArr[16*idx]);
              }
            }
            cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
          }
          break;
        case csShaderVariable::TRANSFORM:
          {
            csReversibleTransform t;
            for (idx = 0; idx < numElements; i++)
            {
              if (var->GetArrayElement (i)->GetValue (t))
              {
                makeGLMatrix (t, &tmpArr[16*idx]);
              }
            }
            cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
          }
          break;
	default:
	  break;
        }
        break;
      }
    default:
      break;
    }
  }
}

void csShaderGLCGCommon::ResetState()
{
}

bool csShaderGLCGCommon::DefaultLoadProgram (const char* programStr, 
  CGGLenum type)
{
  size_t i;

  CGprofile profile = CG_PROFILE_UNKNOWN;

  if (cg_profile != 0)
    profile = cgGetProfile (cg_profile);

  if(profile == CG_PROFILE_UNKNOWN)
    profile = cgGLGetLatestProfile (type);

  if (shaderPlug->doVerbose)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg program '%s': using profile %s", description.GetData (), 
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

    if (!param || 
        !(cgIsParameterReferenced (param) && cgGetParameterType (param) != CG_ARRAY))
    {
      variablemap.DeleteIndex (i);
      continue;
    }
    variablemap[i].userVal = (intptr_t)param;
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
