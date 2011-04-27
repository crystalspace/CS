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

#include "ivaria/reporter.h"

#include "csutil/stringquote.h"

#include "glshader_cg.h"
#include "glshader_cgcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

bool csShaderGLCGCommon::ParseClip (iDocumentNode* node)
{
  Clip newClip;
  
  const char* space = node->GetAttributeValue ("space");
  if (space && *space)
  {
    if (strcmp (space, "eye") == 0)
      newClip.space = ShaderProgramPluginGL::ClipPlanes::Eye;
    else if (strcmp (space, "object") == 0)
      newClip.space = ShaderProgramPluginGL::ClipPlanes::Object;
    else if (strcmp (space, "world") == 0)
      newClip.space = ShaderProgramPluginGL::ClipPlanes::World;
    else
    {
      if (shaderPlug->doVerbose)
        synsrv->Report ("crystalspace.graphics3d.shader.glcg",
          CS_REPORTER_SEVERITY_WARNING,
          node,
          "Invalid %s attribute %s", CS::Quote::Single ("space"), space);
      return false;
    }
  }
  else
    newClip.space = ShaderProgramPluginGL::ClipPlanes::Object;
  
  csRef<iDocumentNode> planeNode = node->GetNode ("plane");
  if (planeNode.IsValid())
  {
    if (!ParseProgramParam (planeNode, newClip.plane,
        ParamVector4 | ParamShaderExp))
      return false;
  }
  else
  {
    newClip.plane.SetValue (csVector4 (0, 0, 1, 0));
  }
  
  newClip.distComp = 0;
  newClip.distNeg = false;
  csRef<iDocumentNode> distNode = node->GetNode ("dist");
  if (distNode.IsValid())
  {
    if (!ParseProgramParam (distNode, newClip.distance,
        ParamVector | ParamShaderExp))
      return false;
      
    const char* fullCompStr = distNode->GetAttributeValue ("comp");
    const char* compStr = fullCompStr;
    if (compStr)
    {
      if (*compStr == '-')
      {
        newClip.distNeg = true;
        compStr++;
      }
    
      if (strcmp (compStr, "x") == 0)
	newClip.distComp = 0;
      else if (strcmp (compStr, "y") == 0)
	newClip.distComp = 1;
      else if (strcmp (compStr, "z") == 0)
	newClip.distComp = 2;
      else if (strcmp (compStr, "w") == 0)
	newClip.distComp = 3;
      else
      {
	if (shaderPlug->doVerbose)
	  synsrv->Report ("crystalspace.graphics3d.shader.glcg",
	    CS_REPORTER_SEVERITY_WARNING,
	    node,
	    "Invalid %s attribute %s", CS::Quote::Single ("comp"), fullCompStr);
	return false;
      }
    }
  }
  else
  {
    newClip.distance.SetValue (0);
  }
  clips.Push (newClip);
  
  return true;
}

bool csShaderGLCGCommon::ParseVmap (iDocumentNode* node)
{
  //@@ REWRITE
  const char* destname = node->GetAttributeValue ("destination");
  if (!destname)
  {
    synsrv->Report ("crystalspace.graphics3d.shader.common",
      CS_REPORTER_SEVERITY_WARNING, node,
      "<variablemap> has no %s attribute",
      CS::Quote::Single ("destination"));
    return false;
  }
  
  bool assumeConst = node->GetAttributeValueAsBool ("assumeconst",
    false);

  const char* varname = node->GetAttributeValue ("variable");
  if (!varname)
  {
    // "New style" variable mapping
    VariableMapEntry vme (CS::InvalidShaderVarStringID, destname);
    if (!ParseProgramParam (node, vme.mappingParam,
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
  
  return true;
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
	  synsrv->Report ("crystalspace.graphics3d.shader.cg",
	    CS_REPORTER_SEVERITY_WARNING, child,
	    "<profile> is no longer supported");
          break;
        case XMLTOKEN_ENTRY:
          entrypoint = child->GetContentsValue ();
          break;
        case XMLTOKEN_COMPILERARGS:
          shaderPlug->SplitArgsString (child->GetContentsValue (), 
            compilerArgs);
          break;
	case XMLTOKEN_VARIABLEMAP:
	  if (!ParseVmap (child))
	    return false;
	  cacheKeepNodes.Push (child);
	  break;
	case XMLTOKEN_CLIP:
	  if (!ParseClip (child))
	    return false;
          cacheKeepNodes.Push (child);
	  break;
        default:
	  if (!ParseCommon (child))
	    return false;
      }
    }
  }

  cgResolve = scfQueryInterfaceSafe<iShaderProgramCG> (resolve);
  clips.ShrinkBestFit ();
  ClipsToVmap ();
  ExtractVmapConstants ();

  return true;
}

bool csShaderGLCGCommon::GetProgramNode (iDocumentNode* passProgNode)
{
  if(!passProgNode)
    return false;

  const char* progTypeNode = 0;
  switch (programType)
  {
    case progVP: progTypeNode = "cgvp"; break;
    case progFP: progTypeNode = "cgfp"; break;
  }
  csRef<iDocumentNode> variablesnode = passProgNode->GetNode (progTypeNode);
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = commonTokens.Request (value);
      switch(id)
      {
	case XMLTOKEN_PROGRAM:
	  {
	    const char* filename = child->GetAttributeValue ("file");
	    if (filename != 0)
	    {
	      programFileName = filename;
    
	      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
	      csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
	      if (!file.IsValid())
	      {
		synsrv->Report ("crystalspace.graphics3d.shader.cg",
		  CS_REPORTER_SEVERITY_WARNING, child,
		  "Could not open %s", CS::Quote::Single (filename));
		return false;
	      }
    
	      programFile = file;
	    }
	    else
	      programNode = child;
	  }
	  break;
      }
    }
  }

  return programNode.IsValid();
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
