/*
Copyright (C) 2002 by Marten Svanfeldt
                      Anders Stenberg

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

#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/csmd5.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"
//#include "ivideo/shader/shadervar.h"

#include "glshader_cgvp.h"

SCF_IMPLEMENT_IBASE(csShaderGLCGVP)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLCGVP::Activate(csRenderMesh* mesh)
{
  int i;

  cgGLEnableProfile (cgGetProgramProfile (program));


  /*csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.renderer.stringset", iStringSet);
  csShaderVariable* var = GetVariable (strings->Request ("STANDARD_LIGHT_0_POSITION"));
  if (var)
  {
    CGparameter param = cgGetNamedParameter (program, "LightPos");
    csVector4 blah;
    var->GetValue (blah);
    if (cgIsParameter(param))
      cgGLSetParameter4f(param, blah.x, blah.y, blah.z, blah.w);
  } else printf("Not found!\n");*/

  for(i = 0; i < matrixtrackers.Length(); ++i)
  {
    cgGLSetStateMatrixParameter (
      matrixtrackers[i].parameter, 
      matrixtrackers[i].matrix, 
      matrixtrackers[i].modifier);
  }

  cgGLBindProgram (program);
}

void csShaderGLCGVP::Deactivate()
{
  cgGLDisableProfile (cgGetProgramProfile (program));
}

void csShaderGLCGVP::SetupState ( csRenderMesh *mesh, 
                                csArray<iShaderVariableContext*> &dynamicDomains)
{
  int i;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
	if (!variablemap[i].parameter)
      continue;
    csShaderVariable* lvar = GetVariable(variablemap[i].name);

    if(lvar)
    {
      switch(lvar->GetType())
      {
        case csShaderVariable::INT:
          {
            int intval;
            if(lvar->GetValue(intval))
              cgGLSetParameter1f(variablemap[i].parameter, (float)intval);
          }
          break;
        case csShaderVariable::FLOAT:
          {
            float fval;
            if(lvar->GetValue(fval))
              cgGLSetParameter1f(variablemap[i].parameter, (float)fval);
          }
          break;
        case csShaderVariable::VECTOR3:
          {
            csVector3 v3;
            if(lvar->GetValue(v3))
              cgGLSetParameter3f(variablemap[i].parameter, v3.x, v3.y, v3.z);
          }
          break;
        case csShaderVariable::VECTOR4:
          {
            csVector4 v4;
            if(lvar->GetValue(v4))
              cgGLSetParameter4f(variablemap[i].parameter,
	        v4.x, v4.y, v4.z, v4.w);
          }
          break;
        default:
	  break;
      }
    }
  }

  if (dynamicVars.Length() > 0)
  {
    for(i=0;i<dynamicDomains.Length();i++)
    {
      dynamicDomains[i]->FillVariableList(&dynamicVars);
    }
  }

  for(i = 0; i < dynamicVars.Length(); ++i)
  {
    csShaderVariable* lvar = dynamicVars.Get(i).shaderVariable;

    if(lvar)
    {
      switch(lvar->GetType())
      {
        case csShaderVariable::INT:
          {
            int intval;
            if(lvar->GetValue(intval))
              cgGLSetParameter1f(variablemap[i].parameter, (float)intval);
          }
          break;
        case csShaderVariable::FLOAT:
          {
            float fval;
            if(lvar->GetValue(fval))
              cgGLSetParameter1f(variablemap[i].parameter, (float)fval);
          }
          break;
        case csShaderVariable::VECTOR3:
          {
            csVector3 v3;
            if(lvar->GetValue(v3))
              cgGLSetParameter3f(variablemap[i].parameter, v3.x, v3.y, v3.z);
          }
          break;
        case csShaderVariable::VECTOR4:
          {
            csVector4 v4;
            if(lvar->GetValue(v4))
              cgGLSetParameter4f(variablemap[i].parameter,
	        v4.x, v4.y, v4.z, v4.w);
          }
          break;
        default:
	  break;
      }
      dynamicVars.Get (i).shaderVariable = 0;
    }
  }
}

void csShaderGLCGVP::ResetState()
{
}

bool csShaderGLCGVP::LoadProgramStringToGL(const char* programstring)
{
  if(!programstring)
    return false;

  program = cgCreateProgram (context, CG_SOURCE,
    programstring, cgGLGetLatestProfile (CG_GL_VERTEX),
    "main", 0);

  if (!program)
    return false;

  cgGLLoadProgram (program);

  return true;
}

void csShaderGLCGVP::BuildTokenHash()
{
  xmltokens.Register("CGVP",XMLTOKEN_CGVP);
  xmltokens.Register("declare",XMLTOKEN_DECLARE);
  xmltokens.Register("variablemap",XMLTOKEN_VARIABLEMAP);
  xmltokens.Register("program", XMLTOKEN_PROGRAM);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
}

bool csShaderGLCGVP::Load(iDataBuffer* program)
{
  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (program);
  if (error != 0)
  { 
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.shader.glcg", "XML error '%s'!", error);
    return false;
  }
  return Load(doc->GetRoot());
}

bool csShaderGLCGVP::Load(iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(
  	object_reg, iShaderManager);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	object_reg, "crystalspace.renderer.stringset", iStringSet);


  csRef<iDocumentNode> variablesnode = program->GetNode("cgvp");
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
        case XMLTOKEN_PROGRAM:
          {
            //save for later loading
            programstring = csStrNew (child->GetContentsValue ());
          }
          break;
        case XMLTOKEN_DECLARE:
          {
            //create a new variable
            csRef<csShaderVariable> var = shadermgr->CreateVariable (
              strings->Request(child->GetAttributeValue ("name")));

            // @@@ Will leak! Should do proper refcounting.
            var->IncRef ();

            csStringID idtype = xmltokens.Request (
	    	child->GetAttributeValue("type"));
            idtype -= 100;
            var->SetType( (csShaderVariable::VariableType) idtype);
            switch(idtype)
            {
              case csShaderVariable::INT:
                var->SetValue (child->GetAttributeValueAsInt("default"));
                break;
              case csShaderVariable::FLOAT:
                var->SetValue (child->GetAttributeValueAsFloat("default"));
                break;
              case csShaderVariable::STRING:
                var->SetValue (new scfString (child->GetAttributeValue (
			"default")));
                break;
              case csShaderVariable::VECTOR3:
                const char* def = child->GetAttributeValue("default");
                csVector3 v;
                sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
                var->SetValue( v );
                break;
            }
            AddVariable (var);
          }
          break;
        case XMLTOKEN_VARIABLEMAP:
          {
            variablemap.Push (variablemapentry ());
            int i = variablemap.Length ()-1;

            variablemap[i].name = strings->Request (
              child->GetAttributeValue("variable"));

            const char* cgvarname = child->GetAttributeValue("cgvar");
            variablemap[i].cgvarname = new char[strlen(cgvarname)+1];
            memset(variablemap[i].cgvarname, 0, strlen(cgvarname)+1); 
            memcpy(variablemap[i].cgvarname, cgvarname, strlen(cgvarname));
            variablemap[i].parameter = 0;
          }
          break;
        default:
          return false;
      }
    }
  }

  return true;
}

  
bool csShaderGLCGVP::Prepare(iShaderPass *pass)
{
  if (!LoadProgramStringToGL(programstring))
    return false;

  for(int i = 0; i < variablemap.Length(); i++)
  {
    variablemap[i].parameter = cgGetNamedParameter (
      program, variablemap[i].cgvarname);
    if (!variablemap[i].parameter)
    {
      char msg[500];
      sprintf (msg, 
        "Variablemap warning: Variable '%s' not found in CG program.", 
        variablemap[i].cgvarname);
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.graphics3d.shader.glcg", msg, 0);
    }
    if (!cgIsParameterReferenced (variablemap[i].parameter))
      variablemap[i].parameter = 0;
  }

  CGparameter param = cgGetFirstLeafParameter (program, CG_SOURCE);
  while (param)
  {
    const char* binding = cgGetParameterSemantic (param);
    if (!strcmp (binding, "MV_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_MATRIX;
      map.modifier = CG_GL_MATRIX_IDENTITY;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "I_MV_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "T_MV_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_MATRIX;
      map.modifier = CG_GL_MATRIX_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "IT_MV_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "MVP_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_IDENTITY;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "I_MVP_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "T_MVP_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "IT_MVP_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_MODELVIEW_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "P_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_IDENTITY;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "I_P_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "T_P_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    else if (!strcmp (binding, "IT_P_MATRIX"))
    {
      matrixtrackerentry map;
      map.matrix = CG_GL_PROJECTION_MATRIX;
      map.modifier = CG_GL_MATRIX_INVERSE_TRANSPOSE;
      map.parameter = param;
      matrixtrackers.Push (map);
    }
    
    param = cgGetNextLeafParameter (param);
  }
  return true;
}
