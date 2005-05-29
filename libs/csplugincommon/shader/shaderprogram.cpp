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

#include "cssysdef.h"

#include "csutil/databuf.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csgfx/shaderexp.h"

#include "iutil/verbositymanager.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/shaderprogram.h"

CS_LEAKGUARD_IMPLEMENT (csShaderProgram);

SCF_IMPLEMENT_IBASE(csShaderProgram)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
  SCF_IMPLEMENTS_INTERFACE(iShaderTUResolver)
SCF_IMPLEMENT_IBASE_END

csShaderProgram::csShaderProgram (iObjectRegistry* objectReg)
{
  SCF_CONSTRUCT_IBASE (0);
  InitCommonTokens (commonTokens);

  description = 0;
  csShaderProgram::objectReg = objectReg;
  synsrv = CS_QUERY_REGISTRY (objectReg, iSyntaxService);
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (objectReg, 
    "crystalspace.shared.stringset", iStringSet);
  
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (objectReg, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled("renderer.shader");
  else
    doVerbose = false;
}

csShaderProgram::~csShaderProgram ()
{
  delete[] description;
  SCF_DESTRUCT_IBASE();
}
  
void csShaderProgram::ResolveParamStatic (ProgramParam& param,
  csArray<iShaderVariableContext*> &staticContexts)
{
  if (param.name != csInvalidStringID)
  {
    for (size_t j=0; j < staticContexts.Length(); j++)
    {
      if (!param.var)
      {
	param.var = staticContexts[j]->GetVariable (param.name);
	if (param.var) break;
      }
    }
  }
}

bool csShaderProgram::ParseProgramParam (iDocumentNode* node,
  ProgramParam& param, uint types)
{
  const char* type = node->GetAttributeValue ("type");
  if (type == 0)
  {
    synsrv->Report ("crystalspace.graphics3d.shader.common",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'type' attribute");
    return false;
  }
  ProgramParamType paramType = ParamInvalid;
  if (strcmp (type, "shadervar") == 0)
  {
    const char* value = node->GetContentsValue();
    if (!value)
    {
      synsrv->Report ("crystalspace.graphics3d.shader.common",
	CS_REPORTER_SEVERITY_WARNING,
	node,
	"Node has no contents");
      return false;
    }
    param.name = strings->Request (value);
    param.valid = true;
    return true;
  }
  else if (strcmp (type, "float") == 0)
  {
    paramType = ParamFloat;
  }
  else if (strcmp (type, "vector2") == 0)
  {
    paramType = ParamVector2;
  }
  else if (strcmp (type, "vector3") == 0)
  {
    paramType = ParamVector3;
  }
  else if (strcmp (type, "vector4") == 0)
  {
    paramType = ParamVector4;
  }
  else if (strcmp (type, "matrix") == 0)
  {
    paramType = ParamMatrix;
  }
  else if (strcmp (type, "transform") == 0)
  {
    paramType = ParamTransform;
  }
  else if ((strcmp (type, "expression") == 0) || (strcmp (type, "expr") == 0))
  {
    csRef<csShaderVariable> var;
    var.AttachNew (new csShaderVariable (csInvalidStringID));
    csRef<iShaderVariableAccessor> acc = synsrv->ParseShaderVarExpr (node);
    var->SetType (csShaderVariable::VECTOR4);
    var->SetAccessor (acc);
    param.var = var;
    param.valid = true;
    return true;
  }
  else 
  {
    synsrv->Report ("crystalspace.graphics3d.shader.common",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Unknown type '%s'", type);
    return false;
  }

  if (!types & paramType)
  {
    synsrv->Report ("crystalspace.graphics3d.shader.common",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Type '%s' not supported by this parameter", type);
    return false;
  }

  param.type = paramType;

  switch (paramType)
  {
    case ParamInvalid:
      return false;
      break;
    case ParamFloat:
      {
	float x = node->GetContentsValueAsFloat ();
	param.vectorValue.Set (x, x, x, x);
      }
      break;
    case ParamVector2:
      {
	float x, y;
	const char* value = node->GetContentsValue();
	if (!value)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Node has no contents");
	  return false;
	}
	if (sscanf (value, "%f,%f", &x, &y) != 2)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Couldn't parse vector2 '%s'", value);
	  return false;
	}
	param.vectorValue.Set (x, y, 0.0f, 1.0f);
      }
      break;
    case ParamVector3:
      {
	float x, y, z;
	const char* value = node->GetContentsValue();
	if (!value)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Node has no contents");
	  return false;
	}
	if (sscanf (value, "%f,%f,%f", &x, &y, &z) != 3)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Couldn't parse vector3 '%s'", value);
	  return false;
	}
	param.vectorValue.Set (x, y, z, 1.0f);
      }
      break;
    case ParamVector4:
      {
	float x, y, z, w;
	const char* value = node->GetContentsValue();
	if (!value)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Node has no contents");
	  return false;
	}
	if (sscanf (value, "%f,%f,%f,%f", &x, &y, &z, &w) != 4)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Couldn't parse vector4 '%s'", value);
	  return false;
	}
	param.vectorValue.Set (x, y, z, w);
      }
      break;
    case ParamMatrix:
      {
	if (!synsrv->ParseMatrix (node, param.matrixValue))
	  return false;
      }
      break;
    case ParamTransform:
      {
        csRef<iDocumentNode> matrix_node = node->GetNode ("matrix");
        if (matrix_node)
        {
          csMatrix3 m;
          if (!synsrv->ParseMatrix (matrix_node, m))
            return false;
          param.transformValue.SetT2O (m);
        }
        csRef<iDocumentNode> vector_node = node->GetNode ("v");
        if (vector_node)
        {
          csVector3 v;
          if (!synsrv->ParseVector (vector_node, v))
            return false;
          param.transformValue.SetOrigin (v);
        }
      }
      break;
  }

  param.valid = true;
  return true;
}

bool csShaderProgram::RetrieveParamValue (ProgramParam& param, 
  const csShaderVarStack& stacks)
{
  if (!param.valid) return false;
  csRef<csShaderVariable> var = param.var;
  if (!var && (param.name != csInvalidStringID) &&
      param.name < (csStringID)stacks.Length () && 
      stacks[param.name].Length () > 0)
  {
    var = stacks[param.name].Top ();
  }
  if (var)
  {
    const csShaderVariable::VariableType varType = var->GetType();
    if (varType == csShaderVariable::MATRIX)
      var->GetValue (param.matrixValue);
    else if (varType == csShaderVariable::TRANSFORM)
      var->GetValue (param.transformValue);
    else
      var->GetValue (param.vectorValue);
    switch (varType)
    {
      case csShaderVariable::INT:
      case csShaderVariable::FLOAT:
	param.type = ParamFloat;
	break;
      case csShaderVariable::VECTOR2:
	param.type = ParamVector2;
	break;    
      case csShaderVariable::COLOR:
      case csShaderVariable::VECTOR3:
	param.type = ParamVector3;
	break;
      case csShaderVariable::VECTOR4:
	param.type = ParamVector4;
	break;
      case csShaderVariable::MATRIX:
	param.type = ParamMatrix;
	break;
      case csShaderVariable::TRANSFORM:
	param.type = ParamTransform;
	break;
      default:
	param.type = ParamInvalid;
    }
  }

  return ((var != 0) || (param.name == csInvalidStringID));
}

bool csShaderProgram::ParseCommon (iDocumentNode* child)
{
  const char* value = child->GetValue ();
  csStringID id = commonTokens.Request (value);
  switch (id)
  {
    case XMLTOKEN_VARIABLEMAP:
      {
	const char* destname = child->GetAttributeValue ("destination");
	if (!destname)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING, child,
	    "<variablemap> has no 'destination' attribute");
	  return false;
	}

	const char* varname = child->GetAttributeValue ("variable");
	if (!varname)
	{
	  // "New style" variable mapping
	  VariableMapEntry vme (csInvalidStringID, destname);
	  if (!ParseProgramParam (child, vme.mappingParam,
	    ParamFloat | ParamVector2 | ParamVector3 | ParamVector4))
	    return false;
	  variablemap.Push (vme);
	}
	else
	{
	  // "Classic" variable mapping
	  variablemap.Push (VariableMapEntry (strings->Request (varname),
	    destname));
	}
      }
      break;
    case XMLTOKEN_PROGRAM:
      {
	const char* filename = child->GetAttributeValue ("file");
	if (filename != 0)
	{
	  programFileName = filename;

	  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objectReg, iVFS);
	  csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
	  if (!file.IsValid())
	  {
	    synsrv->Report ("crystalspace.graphics3d.shader.common",
	      CS_REPORTER_SEVERITY_WARNING, child,
	      "Could not open '%s'", filename);
	    return false;
	  }

	  programFile = file;
	}
	else
	  programNode = child;
      }
      break;

    case XMLTOKEN_DESCRIPTION:
      delete[] description;
      description = csStrNew (child->GetContentsValue());
      break;
    case XMLTOKEN_SHADERVAR:
      {
        const char* varname = child->GetAttributeValue ("name");
	if (!varname)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING, child,
	    "<shadervar> without name");
	  return false;
	}
	csRef<csShaderVariable> var;
	var.AttachNew (new csShaderVariable (strings->Request (varname)));

        if (!synsrv->ParseShaderVar (child, *var))
        {
	  return false;
        }
	svcontext.AddVariable (var);
      }
      break;
    default:
      synsrv->ReportBadToken (child);
      return false;
  }
  return true;
}

iDocumentNode* csShaderProgram::GetProgramNode ()
{
  if (programNode.IsValid ())
    return programNode;

  if (programFile.IsValid ())
  {
    csRef<iDocumentSystem> docsys = CS_QUERY_REGISTRY (objectReg, 
      iDocumentSystem);
    if (!docsys)
      docsys.AttachNew (new csTinyDocumentSystem ());
    csRef<iDocument> doc (docsys->CreateDocument ());

    const char* err = doc->Parse (programFile, true);
    if (err != 0)
    {
      csReport (objectReg,
	CS_REPORTER_SEVERITY_WARNING, 
	"crystalspace.graphics3d.shader.common",
	"Error parsing %s: %s", programFileName.GetData(), err);
      return 0;
    }
    programNode = doc->GetRoot ();
    programFile = 0;
    return programNode;
  }

  return 0;
}

csPtr<iDataBuffer> csShaderProgram::GetProgramData ()
{
  if (programFile.IsValid())
  {
    return programFile->GetAllData ();
  }

  if (programNode.IsValid())
  {
    char* data = csStrNew (programNode->GetContentsValue ());

    csRef<iDataBuffer> newbuff;
    newbuff.AttachNew (new csDataBuffer (data, strlen (data)));
    return csPtr<iDataBuffer> (newbuff);
  }

  return 0;
}

void csShaderProgram::ResolveStaticVars (
  csArray<iShaderVariableContext*> &staticContexts)
{
  csShaderVariable *var;

  for (size_t i = 0; i < variablemap.Length (); i++)
  {
    // Check if we've got it locally
    var = svcontext.GetVariable(variablemap[i].name);
    if (!var)
    {
      // If not, check the static contexts
      for (size_t j = 0; j < staticContexts.Length(); j++)
      {
	var = staticContexts[j]->GetVariable (variablemap[i].name);
	if (var) break;
      }
    }
    if (var)
    {
      // We found it, so we add it as a static mapping
      //variablemap[i].statlink = var;
      variablemap[i].mappingParam.var = var;
    }
  }
}

void csShaderProgram::DumpProgramInfo (csString& output)
{
  output << "Program description: " << 
    (description ? description : "<none>") << "\n";
  output << "Program file name: " << programFileName << "\n";
}

void csShaderProgram::DumpVariableMappings (csString& output)
{
  for (size_t v = 0; v < variablemap.Length(); v++)
  {
    const VariableMapEntry& vme = variablemap[v];

    output << strings->Request (vme.name);
    output << '(' << vme.name << ") -> ";
    output << vme.destination << ' ';
    output.AppendFmt ("%jx", (intmax_t)vme.userVal);
    output << ' ';
    output << '\n'; 
  }
}
