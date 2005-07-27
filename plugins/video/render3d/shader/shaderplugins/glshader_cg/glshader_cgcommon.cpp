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
    CGtype paramType = cgGetParameterType (param);
    
    switch (paramType)
    {
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
          if (var->GetType () == csShaderVariable::MATRIX)
          {
            csMatrix3 m;
            if (var->GetValue (m))
            {
              float matrix[16];
              makeGLMatrix (m, matrix);
              cgGLSetMatrixParameterfc (param, matrix);
            }
          }
          else if (var->GetType () == csShaderVariable::TRANSFORM)
          {
            csReversibleTransform t;
            if (var->GetValue (t))
            {
              float matrix[16];
              makeGLMatrix (t, matrix);
              cgGLSetMatrixParameterfc (param, matrix);
            }
          }
          else
          {
            CS_ASSERT_MSG("Can't convert all SV contents to FLOAT4x4 (yet)", false);
          }
        }
        break;
      case CG_ARRAY:
        {
          CGtype innerType = cgGetArrayType (param);
          if (var->GetArraySize () == 0) 
            break;

          uint numElements = csMin ((uint)cgGetArraySize (param, 0), 
                                    (uint)var->GetArraySize ());

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
            case CG_FLOAT4x4:
              {
                if (var->GetType () == csShaderVariable::MATRIX)
                {
                  csMatrix3 m;
                  for (idx = 0; idx < numElements; idx++)
                  {
                    csShaderVariable *element =
                      var->GetArrayElement (idx);
                    if (element != 0 && element->GetValue (m))
                    {
                      makeGLMatrix (m, &tmpArr[16*idx]);
                    }
                  }
                  cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
                }
                else if (var->GetType () == csShaderVariable::TRANSFORM)
                {
                  csReversibleTransform t;
                  for (idx = 0; idx < numElements; idx++)
                  {
                    csShaderVariable *element =
                      var->GetArrayElement (idx);
                    if (element != 0 && element->GetValue (t))
                    {
                      makeGLMatrix (t, &tmpArr[16*idx]);
                    }
                  }
                  cgGLSetMatrixParameterArrayfc (param, 0, numElements, tmpArr);
                }
                else
                {
                  CS_ASSERT_MSG("Can't convert all SV contents to FLOAT4x4 (yet)", false);
                }
              }
              break;
            default:
              CS_ASSERT_MSG("Don't support CG param type (yet)", false);
          }
        }
      default:
        CS_ASSERT_MSG("Don't support CG param type (yet)", false);
    }
  }
}

void csShaderGLCGCommon::ResetState()
{
}

bool csShaderGLCGCommon::DefaultLoadProgram (const char* programStr, 
  CGGLenum type, bool compiled, bool doLoad)
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

  program = cgCreateProgram (shaderPlug->context, 
    compiled ? CG_OBJECT : CG_SOURCE, programStr, 
    profile, entrypoint ? entrypoint : "main", 
    GetProfileCompilerArgs (profile));

  if (!program)
    return false;

  if (shaderPlug->debugDump)
    DoDebugDump();

  if (doLoad) cgGLLoadProgram (program);

  i = 0;
  while (i < variablemap.Length ())
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

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objectReg, iVFS);
  if (!debugFN)
  {
    static int programCounter = 0;
    csString filename;
    filename << shaderPlug->dumpDir << (programCounter++) << programType << 
      ".txt";
    debugFN = csStrNew (filename);
    vfs->DeleteFile (debugFN);
  }

  csRef<iFile> debugFile = vfs->Open (debugFN, VFS_FILE_APPEND);
  if (!debugFile)
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not write '%s'", debugFN);
  }
  else
  {
    debugFile->Write (output.GetData(), output.Length());
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
