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

#include "video/canvas/openglcommon/glextmanager.h"

#include "glshader_arb.h"
#include "glshader_avp.h"

SCF_IMPLEMENT_IBASE(csShaderGLAVP)
SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLAVP::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (shaderPlug->object_reg, severity, 
    "crystalspace.graphics3d.shader.glarb", msg, args);
  va_end (args);
}

void csShaderGLAVP::Activate ()
{
  //enable it
  glEnable(GL_VERTEX_PROGRAM_ARB);
  shaderPlug->ext->glBindProgramARB (GL_VERTEX_PROGRAM_ARB, program_num);
}

void csShaderGLAVP::Deactivate()
{
  glDisable (GL_VERTEX_PROGRAM_ARB);
}

void csShaderGLAVP::SetupState (csRenderMesh *mesh, 
				csArray<iShaderVariableContext*> &dynamicDomains)
{
  int i;
  const csGLExtensionManager* ext = shaderPlug->ext;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
    csShaderVariable* lvar = variablemap[i].ref;

    if(lvar)
    {
      csVector4 v4;
      if (lvar->GetValue (v4))
      {
        ext->glProgramLocalParameter4fvARB (GL_VERTEX_PROGRAM_ARB, 
          variablemap[i].registernum, &v4.x);
      }
    }
  }

  if (dynamicVars.Length() > 0)
  {
    dynamicVars.PrepareFill ();
    for(i=0;i<dynamicDomains.Length();i++)
    {
      dynamicDomains[i]->FillVariableList (&dynamicVars);
    }
  }

  for(i = 0; i < dynamicVars.Length(); ++i)
  {
    csShaderVariableProxy& proxy = dynamicVars.Get(i);
    csShaderVariable* lvar = proxy.shaderVariable;

    if(lvar)
    {
      csVector4 v4;
      if (lvar->GetValue (v4))
      {
        ext->glProgramLocalParameter4fvARB (GL_VERTEX_PROGRAM_ARB, 
          (int)proxy.userData, &v4.x);
      }
      proxy.shaderVariable = 0;
    }
  }
}

void csShaderGLAVP::ResetState ()
{
}

bool csShaderGLAVP::LoadProgramStringToGL (const char* programstring)
{
  if (!shaderPlug->ext)
    return false;

  const csGLExtensionManager* ext = shaderPlug->ext;

  if(!ext->CS_GL_ARB_vertex_program)
    return false;

  if(!programstring)
    return false;

  //step to first !!
  int stringlen = strlen(programstring);
  int i=0;
  while (*programstring != '!' && i<stringlen)
  {
    ++programstring;
    ++i;
  }

  ext->glGenProgramsARB(1, &program_num);
  ext->glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program_num);
  
  ext->glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, 
    strlen(programstring), (void*) programstring);

  const GLubyte * programErrorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

  GLint errorpos;
  glGetIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
  if(errorpos != -1)
  {
    CS_ALLOC_STACK_ARRAY (char, errorStart, strlen (programstring) + 1);
    strcpy (errorStart, programstring);

    char* start = errorStart + errorpos;
    while (start > errorStart)
    {
      if (*(start - 1) == '\n')
      {
	break;
      }
      start--;
    }

    char* end = strchr (start, '\n');
    if (end)
      *(end-1) = 0;

    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Couldn't load vertex program \"%s\"", description);
    Report (CS_REPORTER_SEVERITY_WARNING, "Program error at: \"%s\"", start);
    Report (CS_REPORTER_SEVERITY_WARNING, "Error string: '%s'", 
      programErrorString);
    return false;
  }
  else
  {
    if ((programErrorString != 0) && (*programErrorString != 0))
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Warning for vertex program \"%s\": '%s'", description, 
	programErrorString);
    }
  }

  return true;
}

void csShaderGLAVP::BuildTokenHash()
{
  xmltokens.Register ("arbvp", XMLTOKEN_ARBVP);
  xmltokens.Register ("declare", XMLTOKEN_DECLARE);
  xmltokens.Register ("variablemap", XMLTOKEN_VARIABLEMAP);
  xmltokens.Register ("program", XMLTOKEN_PROGRAM);
  xmltokens.Register ("description", XMLTOKEN_DESCRIPTION);

  // Note: to avoid collision between the XMLTOKENs and the SV types,
  // the XMLTOKENs start with 100
  xmltokens.Register ("integer", csShaderVariable::INT);
  xmltokens.Register ("float", csShaderVariable::FLOAT);
  xmltokens.Register ("vector3", csShaderVariable::VECTOR3);
}


bool csShaderGLAVP::Load (iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    shaderPlug->object_reg, "crystalspace.shared.stringset", iStringSet);

  csRef<iDocumentNode> variablesnode = program->GetNode("arbvp");
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
              new csShaderVariable (strings->Request(child->GetAttributeValue ("name"))));

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
          break;
        default:
          return false;
      }
    }
  }

  return true;
}

/*
bool csShaderGLAVP::Prepare (iShaderPass *pass)
{
  //compile variables
  variablemapentry tempEntry;
  csShaderVariableProxy tempProx;
  csArray<variablemapentry> newStat;
  csShaderVariable *var;
  int i;

  for (i = 0; i < variablemap.Length (); i++)
  {
    var = GetVariable(variablemap[i].name);
    if (!var)
      var = pass->GetVariableRecursive (variablemap[i].name);
    if (var)
    {
      //static
      tempEntry = variablemap[i];
      tempEntry.ref = var;
      newStat.Push (tempEntry);
    }
    else
    {
      //dynamic
      tempProx.Name = variablemap[i].name;
      tempProx.userData = variablemap[i].registernum;
      dynamicVars.InsertSorted (tempProx);
    }
  }
  variablemap.Empty ();
  newStat.TransferTo (variablemap);

  return LoadProgramStringToGL(programstring);
}
*/

bool csShaderGLAVP::Compile(csArray<iShaderVariableContext*> &staticDomains)
{
  shaderPlug->Open ();

  variablemapentry tempEntry;
  csShaderVariableProxy tempProx;
  csArray<variablemapentry> newStat;
  csShaderVariable *var;
  int i,j;

  for (i = 0; i < variablemap.Length (); i++)
  {
    var = GetVariable(variablemap[i].name);
    if (!var)
    {
      for (j=0;j<staticDomains.Length();j++)
      {
        var = staticDomains[j]->GetVariable (variablemap[i].name);
        if (var) break;
      }
    }
    if (var)
    {
      //static
      tempEntry = variablemap[i];
      tempEntry.ref = var;
      newStat.Push (tempEntry);
    }
    else
    {
      //dynamic
      tempProx.Name = variablemap[i].name;
      tempProx.userData = (void*)variablemap[i].registernum;
      dynamicVars.InsertSorted (tempProx);
    }
  }
  variablemap.Empty ();
  newStat.TransferTo (variablemap);

  return LoadProgramStringToGL (programstring);
}
