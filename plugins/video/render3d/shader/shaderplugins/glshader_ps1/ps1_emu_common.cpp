/*
Copyright (C) 2002 by John Harger

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

#include "glshader_ps1.h"
#include "ps1_emu_common.h"

SCF_IMPLEMENT_IBASE(csShaderGLPS1_Common)
SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLPS1_Common::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (shaderPlug->object_reg, severity, 
    "crystalspace.graphics3d.shader.glps1", msg, args);
  va_end (args);
}

void csShaderGLPS1_Common::BuildTokenHash()
{
  xmltokens.Register ("ps1fp", XMLTOKEN_PS1FP);
  xmltokens.Register ("declare", XMLTOKEN_DECLARE);
  xmltokens.Register ("variablemap", XMLTOKEN_VARIABLEMAP);
  xmltokens.Register ("program", XMLTOKEN_PROGRAM);
  xmltokens.Register ("description", XMLTOKEN_DESCRIPTION);

  // Note: to avoid collision between the XMLTOKENs and the SV types,
  // the XMLTOKENs start with 100
  xmltokens.Register ("integer", csShaderVariable::INT);
  xmltokens.Register ("float", csShaderVariable::FLOAT);
  xmltokens.Register ("vector3", csShaderVariable::VECTOR3);
  xmltokens.Register ("vector4", csShaderVariable::VECTOR4);
}

bool csShaderGLPS1_Common::Load(iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iDocumentNode> variablesnode = program->GetNode("ps1fp");
  if (variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_PROGRAM:
          //save for later loading
          programstring = csStrNew (child->GetContentsValue ());
          break;
        case XMLTOKEN_DESCRIPTION:
	        description = csStrNew (child->GetContentsValue ());
	        break;
        case XMLTOKEN_DECLARE:
          {
            //create a new variable
            csRef<csShaderVariable> var = csPtr<csShaderVariable>(
              new csShaderVariable (strings->Request(
	      	child->GetAttributeValue ("name"))));

            // @@@ Will leak! Should do proper refcounting.
            var->IncRef ();

            csStringID idtype = xmltokens.Request (
	    	      child->GetAttributeValue("type") );
            var->SetType ((csShaderVariable::VariableType) idtype);
            switch(idtype)
            {
              case csShaderVariable::INT:
                var->SetValue (child->GetAttributeValueAsInt ("default"));
                break;
              case csShaderVariable::FLOAT:
                var->SetValue (child->GetAttributeValueAsFloat ("default"));
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

            variablemap[i].registernum = 
              child->GetAttributeValueAsInt("register");
          }
          break;
        default:
          return false;
      }
    }
  }

  return true;
}

bool csShaderGLPS1_Common::Load (const char* program, 
                            csArray<varmapping> &mappings)
{
  programstring = csStrNew (program);

  for (int m=0; m<mappings.Length (); ++m)
  {
    variablemap.Push (variablemapentry ());
    int i = variablemap.Length ()-1;

    variablemap[i].name = mappings[m].source;

    variablemap[i].registernum = atoi (mappings[m].destination);
  }

  return true;
}


bool csShaderGLPS1_Common::Compile(
	csArray<iShaderVariableContext*> &staticContexts)
{
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

  return LoadProgramStringToGL(programstring);
}

