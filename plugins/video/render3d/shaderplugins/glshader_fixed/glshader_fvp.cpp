/*
Copyright (C) 2002 by Mårten Svanfeldt
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

#include "video/canvas/openglcommon/glextmanager.h"

#include "glshader_fvp.h"

SCF_IMPLEMENT_IBASE(csGLShaderFVP)
SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csGLShaderFVP::Activate(iShaderPass* current, csRenderMesh* mesh)
{
  //enable it
}

void csGLShaderFVP::Deactivate(iShaderPass* current)
{
}

void csGLShaderFVP::SetupState (iShaderPass *current, csRenderMesh *mesh)
{
  int i;

  // set variables
  /*for(i = 0; i < variablemap.Length(); ++i)
  {
    csShaderVariable* lvar = GetVariable(variablemap[i].name);

    if(lvar)
    {
      switch(lvar->GetType())
      {
      case csShaderVariable::INT:
        {
          int intval;
          if(lvar->GetValue(intval))
            ext->glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 
            variablemap[i].registernum,
            (float)intval, (float)intval, (float)intval, (float)intval);
        }
        break;
      case csShaderVariable::FLOAT:
        {
          float fval;
          if(lvar->GetValue(fval))
            ext->glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 
            variablemap[i].registernum,
            fval, fval, fval, fval);
        }
        break;
      case csShaderVariable::VECTOR3:
        {
          csVector3 v3;
          if(lvar->GetValue(v3))
            ext->glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 
            variablemap[i].registernum,
            v3.x, v3.y, v3.z, 1);
        }
        break;
      case csShaderVariable::VECTOR4:
        {
          csVector4 v4;
          if(lvar->GetValue(v4))
            ext->glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 
            variablemap[i].registernum,
            v4.x, v4.y, v4.z, v4.w);
        }
        break;
      default:
	break;
      }
    }
  }*/
  /*csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.renderer.stringset", iStringSet);
  csShaderVariable* lvar = GetVariable(strings->Request (
    "STANDARD_TIME"));*/
  
}

void csGLShaderFVP::ResetState ()
{
}

void csGLShaderFVP::BuildTokenHash()
{
  xmltokens.Register("fixedvp",XMLTOKEN_FIXEDVP);
  xmltokens.Register("declare",XMLTOKEN_DECLARE);
  xmltokens.Register("variablemap",XMLTOKEN_VARIABLEMAP);
  xmltokens.Register("program", XMLTOKEN_PROGRAM);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
}

bool csGLShaderFVP::Load(iDataBuffer* program)
{
  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (program);
  if (error != 0)
  { 
    csReport( object_reg, CS_REPORTER_SEVERITY_ERROR, 
      "crystalspace.graphics3d.shader.fixed", "XML error '%s'!", error);
    return false;
  }
  return Load(doc->GetRoot());
}

bool csGLShaderFVP::Load(iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(object_reg, iShaderManager);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.renderer.stringset", iStringSet);

  csRef<iDocumentNode> variablesnode = program->GetNode("fixedvp");
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
      /*case XMLTOKEN_PROGRAM:
        {
          //save for later loading
          programstring = new char[strlen(child->GetContentsValue())+1];
          strcpy(programstring, child->GetContentsValue());
        }
          break;
      case XMLTOKEN_DECLARE:
        {
          //create a new variable
          csRef<csShaderVariable> var = 
            shadermgr->CreateVariable (
            strings->Request(child->GetAttributeValue ("name")));

          // @@@ Will leak! Should do proper refcounting.
          var->IncRef ();

          csStringID idtype = xmltokens.Request( child->GetAttributeValue("type") );
          idtype -= 100;
          var->SetType( (csShaderVariable::VariableType) idtype);
          switch(idtype)
          {
          case csShaderVariable::INT:
            var->SetValue( child->GetAttributeValueAsInt("default") );
            break;
          case csShaderVariable::FLOAT:
            var->SetValue( child->GetAttributeValueAsFloat("default") );
            break;
          case csShaderVariable::STRING:
            var->SetValue(new scfString( child->GetAttributeValue("default")) );
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

          variablemap[i].registernum = 
            child->GetAttributeValueAsInt("register");
        }
        break;*/
      default:
        break;
        //return false;
      }
    }
  }

  return true;
}

  
bool csGLShaderFVP::Prepare()
{
  return true;
}

csPtr<iString> csGLShaderFVP::GetProgramID()
{
  csMD5::Digest d = csMD5::Encode("blah"); // @@@ Should make a real ID
  scfString* str = new scfString();
  str->Append((const char*)d.data, 16);
  return csPtr<iString>(str);
}
