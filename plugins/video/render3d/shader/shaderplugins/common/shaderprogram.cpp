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

#include "ivaria/reporter.h"

#include "shaderprogram.h"

SCF_IMPLEMENT_IBASE(csShaderProgram)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

csShaderProgram::csShaderProgram (iObjectRegistry* objectReg)
{
  SCF_CONSTRUCT_IBASE (0);
  InitCommonTokens (commonTokens);

  csShaderProgram::objectReg = objectReg;
  synsrv = CS_QUERY_REGISTRY (objectReg, iSyntaxService);
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (objectReg, 
    "crystalspace.shared.stringset", iStringSet);
}

csShaderProgram::~csShaderProgram ()
{
  SCF_DESTRUCT_IBASE();
}

bool csShaderProgram::ParseCommon (iDocumentNode* child)
{
  const char* value = child->GetValue ();
  csStringID id = commonTokens.Request (value);
  switch (id)
  {
    case XMLTOKEN_VARIABLEMAP:
      {
	const char* varname = child->GetAttributeValue ("variable");
	if (!varname)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING, child,
	    "<variablemap> has no 'variable' attribute");
	  return false;
	}

	const char* destname = child->GetAttributeValue ("destination");
	if (!destname)
	{
	  synsrv->Report ("crystalspace.graphics3d.shader.common",
	    CS_REPORTER_SEVERITY_WARNING, child,
	    "<variablemap> has no 'destination' attribute");
	  return false;
	}

	variablemap.Push (VariableMapEntry (strings->Request (varname),
	  destname));
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
      description = child->GetContentsValue();
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

    const char* err = doc->Parse (programFile);
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

  for (int i = 0; i < variablemap.Length (); i++)
  {
    // Check if we've got it locally
    var = svcontext.GetVariable(variablemap[i].name);
    if (!var)
    {
      // If not, check the static contexts
      for (int j = 0; j < staticContexts.Length(); j++)
      {
	var = staticContexts[j]->GetVariable (variablemap[i].name);
	if (var) break;
      }
    }
    if (var)
    {
      // We found it, so we add it as a static mapping
      variablemap[i].statlink = var;
    }
  }
}
