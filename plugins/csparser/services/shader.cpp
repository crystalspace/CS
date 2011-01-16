/*
    Copyright (C) 2008 by Frank Richter

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

#include "syntxldr.h"

#include "iengine/engine.h"
#include "imap/ldrctxt.h"
#include "iutil/document.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "ivideo/shader/shader.h"

#include "csgfx/shaderexp.h"
#include "csgfx/shaderexpaccessor.h"
#include "csgfx/shadervar.h"
#include "cstool/vfsdirchange.h"
#include "csutil/scanstr.h"
#include "csutil/stringconv.h"
#include "csutil/stringquote.h"
#include "csutil/xmltiny.h"

CS_PLUGIN_NAMESPACE_BEGIN(SyntaxService)
{

bool csTextSyntaxService::ParseShaderVar (iLoaderContext* ldr_context,
    	iDocumentNode* node, csShaderVariable& var,
      iStringArray* failedTextures)
{
  csRef<iLoaderContext> engineLoaderContext;
  if (ldr_context == 0)
  {
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
    if (engine.IsValid())
      engineLoaderContext = engine->CreateLoaderContext (0);
    ldr_context = engineLoaderContext;
  }

  const char *name = node->GetAttributeValue("name");
  if (name != 0)
  {
    var.SetName (strings->Request (name));
  }
  const char *type = node->GetAttributeValue("type");
  if (!type)
  {
    Report (
      "crystalspace.syntax.shadervariable",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Invalid shadervar type specified.");
    return false;
  }
  csStringID idtype = xmltokens.Request (type);
  switch (idtype)
  {
    case XMLTOKEN_INT:
    case XMLTOKEN_INTEGER:
      var.SetValue (node->GetContentsValueAsInt ());
      break;
    case XMLTOKEN_FLOAT:
      var.SetValue (node->GetContentsValueAsFloat ());
      break;
    case XMLTOKEN_VECTOR2:
      {
        const char* def = node->GetContentsValue ();
        csVector2 v (0.0f, 0.0f);
        csScanStr (def, "%f,%f", &v.x, &v.y);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR3:
      {
        const char* def = node->GetContentsValue ();
        csVector3 v (0);
        csScanStr (def, "%f,%f,%f", &v.x, &v.y, &v.z);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR4:
      {
        const char* def = node->GetContentsValue ();
        csVector4 v (0);
        csScanStr (def, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_TEXTURE:
      {
        if (!ldr_context) break;
        csRef<iTextureWrapper> tex;
        // @@@ This should be done in a better way...
        //  @@@ E.g. lazy retrieval of the texture with an accessor?
        const char* texname = node->GetContentsValue ();
        tex = ldr_context->FindTexture (texname);

        if(failedTextures)
        {
          // Check for failed texture load.
          int i = 0;
          while(!tex)
          {
            if(failedTextures->GetSize() != 0 &&
              !strcmp(failedTextures->Get(i), texname))
            {
              Report (
                "crystalspace.syntax.shadervariable",
                CS_REPORTER_SEVERITY_WARNING,
                node,
                "Texture %s not found.", CS::Quote::Single (texname));
            }

            if(i >= (int)(failedTextures->GetSize()-1))
            {
              tex = ldr_context->FindTexture (texname);
              i = 0;
            }
            else
            {
              i++;
            }
          }
        }
        else if(!tex)
        {
          Report (
            "crystalspace.syntax.shadervariable",
            CS_REPORTER_SEVERITY_WARNING,
            node,
            "Texture %s not found.", CS::Quote::Single (texname));
        }

        var.SetValue (tex);
      }
      break;
    case XMLTOKEN_LIBEXPR:
      {
	const char* exprname = node->GetAttributeValue ("exprname");
	if (!exprname)
	{
	  Report ("crystalspace.syntax.shadervariable.expression",
		CS_REPORTER_SEVERITY_ERROR,
		node, "%s attribute missing for shader expression!",
		CS::Quote::Single ("exprname"));
	  return false;
	}
	csRef<iShaderManager> shmgr = csQueryRegistry<iShaderManager> (
		object_reg);
	if (shmgr)
	{
          iShaderVariableAccessor* acc = shmgr->GetShaderVariableAccessor (
	    exprname);
	  if (!acc)
	  {
	    Report ("crystalspace.syntax.shadervariable.expression",
		  CS_REPORTER_SEVERITY_ERROR,
		  node, "Can't find expression with name %s!", exprname);
	    return false;
	  }
	  var.SetAccessor (acc);
	  /* Set a type to avoid a premature accessor call (many shader 
	     expressions may utilize variables set in the shader which
	     might not be available when the type is tried to be determined) */
	  var.SetType (csShaderVariable::VECTOR4);
	}
      }
      break;
    case XMLTOKEN_EXPR:
    case XMLTOKEN_EXPRESSION:
      {
	csRef<iShaderVariableAccessor> acc = ParseShaderVarExpr (node);
	if (!acc.IsValid())
	  return false;
	var.SetAccessor (acc);
	/* Set a type to avoid a premature accessor call (many shader 
	    expressions may utilize variables set in the shader which
	    might not be available when the type is tried to be determined) */
        var.SetType (csShaderVariable::VECTOR4);
      }
      break;
    case XMLTOKEN_ARRAY:
      {
        csRef<iDocumentNodeIterator> varNodes = node->GetNodes ("shadervar");

        int varCount = 0;
        while (varNodes->HasNext ()) 
        {
          varCount++;
          varNodes->Next ();
        }

        var.SetType (csShaderVariable::ARRAY);
        var.SetArraySize (varCount);

        varCount = 0;
        varNodes = node->GetNodes ("shadervar");
        while (varNodes->HasNext ()) 
        {
          csRef<iDocumentNode> varNode = varNodes->Next ();
          csRef<csShaderVariable> elementVar = 
            csPtr<csShaderVariable> (new csShaderVariable (CS::InvalidShaderVarStringID));
          var.SetArrayElement (varCount, elementVar);
          ParseShaderVar (ldr_context, varNode, *elementVar, failedTextures);
          varCount++;
        }
      }
      break;
    default:
      Report (
        "crystalspace.syntax.shadervariable",
        CS_REPORTER_SEVERITY_WARNING,
        node,
	"Invalid shadervar type %s.", CS::Quote::Single (type));
      return false;
  }

  return true;
}

csRef<iShaderVariableAccessor> csTextSyntaxService::ParseShaderVarExpr (
  iDocumentNode* node)
{
  csRef<iDocumentNode> exprNode;
  csRef<iDocumentNodeIterator> nodeIt = node->GetNodes();
  while (nodeIt->HasNext())
  {
    csRef<iDocumentNode> child = nodeIt->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    exprNode = child;
    break;
  }

  if (!exprNode)
  {
    Report ("crystalspace.syntax.shadervariable.expression",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Can't find expression node");
    return 0;
  }

  csShaderExpression* expression = new csShaderExpression (object_reg);
  if (!expression->Parse (exprNode))
  {
    Report ("crystalspace.syntax.shadervariable.expression",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Error parsing expression: %s", expression->GetError());
    delete expression;
    return 0;
  }
  csRef<csShaderVariable> var;
  var.AttachNew (new csShaderVariable (CS::InvalidShaderVarStringID));
  csRef<csShaderExpressionAccessor> acc;
  acc.AttachNew (new csShaderExpressionAccessor (object_reg, expression));
  return acc;
}

bool csTextSyntaxService::WriteShaderVar (iDocumentNode* node,
					  csShaderVariable& var)
{
  const char* name = strings->Request (var.GetName ());
  if (name != 0) node->SetAttribute ("name", name);
  switch (var.GetType ())
  {
    case csShaderVariable::INT:
      {
        node->SetAttribute ("type", "integer");
        int val;
        var.GetValue (val);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsInt (val);
      }
      break;
    case csShaderVariable::FLOAT:
      {
        node->SetAttribute ("type", "float");
        float val;
        var.GetValue (val);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (val);
      }
      break;
    case csShaderVariable::VECTOR2:
      {
        node->SetAttribute ("type", "vector2");
        csString val;
        csVector2 vec;
        var.GetValue (vec);
        val.Format ("%s,%s",
		    CS::Utility::ftostr (vec.x).GetData(),
		    CS::Utility::ftostr (vec.y).GetData());
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::VECTOR3:
      {
        node->SetAttribute ("type", "vector3");
        csString val;
        csVector3 vec;
        var.GetValue (vec);
        val.Format ("%s,%s,%s",
		    CS::Utility::ftostr (vec.x).GetData(),
		    CS::Utility::ftostr (vec.y).GetData(),
		    CS::Utility::ftostr (vec.z).GetData());
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::VECTOR4:
      {
        node->SetAttribute ("type", "vector4");
        csString val;
        csVector4 vec;
        var.GetValue (vec);
        val.Format ("%s,%s,%s,%s",
		    CS::Utility::ftostr (vec.x).GetData(),
		    CS::Utility::ftostr (vec.y).GetData(),
		    CS::Utility::ftostr (vec.z).GetData(),
		    CS::Utility::ftostr (vec.w).GetData());
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::TEXTURE:
      {
        node->SetAttribute ("type", "texture");
        iTextureWrapper* val;
        var.GetValue (val);
        if (val)
          node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val->QueryObject ()->GetName ());
      }
      break;
    default:
      break;
  };
  return true;
}

csRef<iShader> csTextSyntaxService::ParseShaderRef (
    iLoaderContext* ldr_context, iDocumentNode* node)
{
  // @@@ FIXME: unify with csLoader::ParseShader()?
  static const char* msgid = "crystalspace.syntax.shaderref";

  const char* shaderName = node->GetAttributeValue ("name");
  if (shaderName == 0)
  {
    ReportError (msgid, node, "no %s attribute",
		 CS::Quote::Single ("name"));
    return 0;
  }

  csRef<iShaderManager> shmgr = csQueryRegistry<iShaderManager> (object_reg);
  csRef<iShader> shader = shmgr->GetShader (shaderName);
  if (shader.IsValid()) return shader;

  const char* shaderFileName = node->GetAttributeValue ("file");
  if (shaderFileName != 0)
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csVfsDirectoryChanger dirChanger (vfs);
    csString filename (shaderFileName);
    csRef<iFile> shaderFile = vfs->Open (filename, VFS_FILE_READ);

    if(!shaderFile)
    {
      Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
	"Unable to open shader file %s!", CS::Quote::Single (shaderFileName));
      return 0;
    }

    csRef<iDocumentSystem> docsys =
      csQueryRegistry<iDocumentSystem> (object_reg);
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());
    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    const char* err = shaderDoc->Parse (shaderFile, false);
    if (err != 0)
    {
      Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
	"Could not parse shader file %s: %s",
	CS::Quote::Single (shaderFileName), err);
      return 0;
    }
    csRef<iDocumentNode> shaderNode = 
      shaderDoc->GetRoot ()->GetNode ("shader");
    dirChanger.ChangeTo (filename);
    
    shader = ParseShader (ldr_context, shaderNode);
    if (shader.IsValid())
      shader->SetFileName (filename);
    return shader;
  }

  return 0;
}

csRef<iShader> csTextSyntaxService::ParseShader (
    iLoaderContext* ldr_context, iDocumentNode* node)
{
  // @@@ FIXME: unify with csLoader::ParseShader()?
  static const char* msgid = "crystalspace.syntax.shader";

  csRef<iShaderManager> shaderMgr = csQueryRegistry<iShaderManager> (object_reg);
  const char* name = node->GetAttributeValue ("name");
  if ((!ldr_context || ldr_context->CheckDupes ()) && name)
  {
    iShader* shader = shaderMgr->GetShader (name);
    if (shader)
    {
      return shader;
    }
  }

  const char* type = node->GetAttributeValue ("compiler");
  if (type == 0)
    type = node->GetAttributeValue ("type");
  if (type == 0)
  {
    ReportError (msgid, node,
      "%s attribute is missing!", CS::Quote::Single ("compiler"));
    return 0;
  }
  csRef<iShaderCompiler> shcom = shaderMgr->GetCompiler (type);
  if (!shcom.IsValid()) 
  {
    ReportError (msgid, node,
      "Could not get shader compiler %s", CS::Quote::Single (type));
    return 0;
  }
  csRef<iShader> shader = shcom->CompileShader (ldr_context, node);
  if (shader.IsValid())
  {
    shaderMgr->RegisterShader (shader);
  }
  else 
    return 0;
  return shader;
}

}
CS_PLUGIN_NAMESPACE_END(SyntaxService)
