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

#include "glshader_cgfp.h"

SCF_IMPLEMENT_IBASE(csShaderGLCGFP)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLCGFP::Activate()
{
  cgGLEnableProfile (cgGetProgramProfile (program));

  cgGLBindProgram (program);
}

void csShaderGLCGFP::Deactivate()
{
  cgGLDisableProfile (cgGetProgramProfile (program));
}

void csShaderGLCGFP::SetupState (csRenderMesh* mesh,
  const CS_SHADERVAR_STACK &stacks)
{
  int i;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
    // Check if it's statically linked
    csRef<csShaderVariable> lvar = variablemap[i].statlink;
    // If not, we check the stack
    if (!lvar && variablemap[i].name < (csStringID)stacks.Length ()
      && stacks[variablemap[i].name].Length () > 0)
      lvar = stacks[variablemap[i].name].Top ();

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
}

void csShaderGLCGFP::ResetState()
{
}

bool csShaderGLCGFP::Compile(csArray<iShaderVariableContext*> &staticContexts)
{
  shaderPlug->Open ();

  csShaderVariable *var;
  int i,j;

  for (i = 0; i < variablemap.Length (); i++)
  {
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
      variablemap[i].statlink = var;
    }
  }

  return LoadProgramStringToGL (programstring);
}

bool csShaderGLCGFP::LoadProgramStringToGL(const char* programstring)
{
  if(!programstring)
    return false;

  CGprofile profile = CG_PROFILE_UNKNOWN;

  if(cg_profile.Length ())
    profile = cgGetProfile (cg_profile.GetData ());

  if(profile == CG_PROFILE_UNKNOWN)
    profile = cgGLGetLatestProfile (CG_GL_FRAGMENT);

  program = cgCreateProgram (shaderPlug->context, CG_SOURCE,
    programstring, profile, "main", 0);

  if (!program)
    return false;

  cgGLLoadProgram (program);

  return true;
}

void csShaderGLCGFP::BuildTokenHash()
{
  xmltokens.Register("CGFP",XMLTOKEN_CGFP);
  xmltokens.Register("declare",XMLTOKEN_DECLARE);
  xmltokens.Register("variablemap", XMLTOKEN_VARIABLEMAP);
  xmltokens.Register("program", XMLTOKEN_PROGRAM);
  xmltokens.Register("profile", XMLTOKEN_PROFILE);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
  xmltokens.Register("vector4", 100+csShaderVariable::VECTOR4);
}

bool csShaderGLCGFP::Load(iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(
  	shaderPlug->object_reg, iShaderManager);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	shaderPlug->object_reg, "crystalspace.shared.stringset", iStringSet);

  csRef<iDocumentNode> variablesnode = program->GetNode("cgfp");
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
            programstring = csStrNew (child->GetContentsValue());
          }
          break;
        case XMLTOKEN_PROFILE:
          cg_profile = child->GetContentsValue ();
          break;
        case XMLTOKEN_DECLARE:
          {
            //create a new variable
            csRef<csShaderVariable> var = 
	            csPtr<csShaderVariable>(new csShaderVariable ( strings->
							     Request (child->
								      GetAttributeValue ("name"))));

            // @@@ Will leak! Should do proper refcounting.
            var->IncRef ();

            csStringID idtype = xmltokens.Request (
	    	      child->GetAttributeValue("type") );
            idtype -= 100;
            var->SetType ((csShaderVariable::VariableType) idtype);
            switch(idtype)
            {
              case csShaderVariable::INT:
                var->SetValue (child->GetAttributeValueAsInt("default"));
                break;
              case csShaderVariable::FLOAT:
                var->SetValue (child->GetAttributeValueAsFloat("default"));
                break;
              case csShaderVariable::VECTOR3:
              {
                const char* def = child->GetAttributeValue("default");
                csVector3 v;
                sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
                var->SetValue( v );
                break;
              }
              case csShaderVariable::VECTOR4:
              {
                const char* def = child->GetAttributeValue("default");
                csVector4 v;
                sscanf(def, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
                var->SetValue( v );
                break;
              }
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