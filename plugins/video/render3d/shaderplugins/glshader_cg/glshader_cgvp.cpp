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

#include "csutil/csvector.h"
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
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/render3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"
//#include "ivideo/shader/shadervar.h"

#include "glshader_cgvp.h"

SCF_IMPLEMENT_IBASE(csShaderGLCGVP)
SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLCGVP::Activate(iShaderPass* current, csRenderMesh* mesh)
{
  csRef<iStreamSource> source = mesh->GetStreamSource ();
  for(int i = 0; i < streammap.Length(); ++i)
  {
    streammapentry* e = (streammapentry*)streammap.Get(i);
    if (!e->parameter)
      continue;
    csRef<iRenderBuffer> buf = source->GetBuffer (e->name);
    if (!buf)
      continue;
    void* ptr = buf->Lock(iRenderBuffer::CS_BUF_LOCK_RENDER);
    if (ptr)
    {
      cgGLSetParameterPointer (e->parameter, 
        source->GetComponentCount (e->name), GL_FLOAT, 0, ptr);
      cgGLEnableClientState (e->parameter);
    }
  }

  CGparameter param = cgGetFirstLeafParameter (program, CG_SOURCE);
  while (param)
  {
    const char* binding = cgGetParameterSemantic (param);
    if (!strcmp (binding, "MODELVIEW_PROJECTION_MATRIX"))
    {
      cgGLSetStateMatrixParameter (param,
        CG_GL_MODELVIEW_PROJECTION_MATRIX, 
        CG_GL_MATRIX_IDENTITY);
    } else if (!strcmp (binding, "MODELVIEW_MATRIX"))
    {
      cgGLSetStateMatrixParameter (param,
        CG_GL_MODELVIEW_MATRIX, 
        CG_GL_MATRIX_IDENTITY);
    }

    
    param = cgGetNextLeafParameter (param);
  }

  // set variables
  int i;
  for(i = 0; i < variablemap.Length(); ++i)
  {
    variablemapentry* e = (variablemapentry*)variablemap.Get(i);
    if (!e->parameter)
      continue;
    iShaderVariable* lvar = GetVariable(e->name);
    if(!lvar)
      lvar = current->GetVariable(e->name);

    if(lvar)
    {
      switch(lvar->GetType())
      {
      case iShaderVariable::INT:
        {
          int intval;
          if(lvar->GetValue(intval))
            cgGLSetParameter1f(e->parameter, (float)intval);
        }
        break;
      case iShaderVariable::VECTOR1:
        {
          float fval;
          if(lvar->GetValue(fval))
            cgGLSetParameter1f(e->parameter, (float)fval);
        }
        break;
      case iShaderVariable::VECTOR3:
        {
          csVector3 v3;
          if(lvar->GetValue(v3))
            cgGLSetParameter3f(e->parameter, v3.x, v3.y, v3.z);
        }
        break;
      }
    }
  }

  cgGLEnableProfile (cgGetProgramProfile (program));
  cgGLBindProgram (program);
}

void csShaderGLCGVP::Deactivate(iShaderPass* current, csRenderMesh* mesh)
{
  cgGLDisableProfile (cgGetProgramProfile (program));

  csRef<iStreamSource> source = mesh->GetStreamSource ();
  for(int i = 0; i < streammap.Length(); ++i)
  {
    streammapentry* e = (streammapentry*)streammap.Get(i);
    if (!e->parameter)
      continue;
    csRef<iRenderBuffer> buf = source->GetBuffer (e->name);
    if (!buf)
      continue;
    buf->Release ();
    cgGLDisableClientState (e->parameter);
  }
}

bool csShaderGLCGVP::LoadProgramStringToGL(const char* programstring)
{
  if(!programstring)
    return false;

  program = cgCreateProgram (context, CG_SOURCE,
    programstring, cgGLGetLatestProfile (CG_GL_VERTEX),
    "main", NULL);

  if (!program)
    return false;

  cgGLLoadProgram (program);

  for(int i = 0; i < variablemap.Length(); i++)
  {
    variablemapentry* e = (variablemapentry*)variablemap.Get(i);
    e->parameter = cgGetNamedParameter (program, e->cgvarname);
    if (!e->parameter)
    {
      char msg[500];
      sprintf (msg, "Variablemap warning: Variable '%s' not found in CG program.", e->cgvarname);
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glcg",
        msg, NULL);
    }
    if (!cgIsParameterReferenced (e->parameter))
      e->parameter = NULL;
  }

  for(int i = 0; i < streammap.Length(); i++)
  {
    streammapentry* e = (streammapentry*)streammap.Get(i);
    e->parameter = cgGetNamedParameter (program, e->cgvarname);
    if (!e->parameter)
    {
      char msg[500];
      sprintf (msg, "Streammap warning: Variable '%s' not found in CG program.", e->cgvarname);
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glcg",
        msg, NULL);
    }
    if (!cgIsParameterReferenced (e->parameter))
      e->parameter = NULL;
  }

  return true;
}

void csShaderGLCGVP::BuildTokenHash()
{
  xmltokens.Register("CGVP",XMLTOKEN_CGVP);
  xmltokens.Register("declare",XMLTOKEN_DECLARE);
  xmltokens.Register("variablemap",XMLTOKEN_VARIABLEMAP);
  xmltokens.Register("streammap",XMLTOKEN_STREAMMAP);
  xmltokens.Register("program", XMLTOKEN_PROGRAM);
}

bool csShaderGLCGVP::Load(iDataBuffer* program)
{
  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (program);
  if (error != NULL)
  { 
    csReport( object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.render3d.shader.glcg",
      "XML error '%s'!", error);
    return false;
  }
  return Load(doc->GetRoot());
}

bool csShaderGLCGVP::Load(iDocumentNode* program)
{
  if(!program)
    return false;

  BuildTokenHash();

  csRef<iRender3D> r3d = CS_QUERY_REGISTRY (object_reg, iRender3D);
  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(object_reg, iShaderManager);

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
          programstring = new char[strlen(child->GetContentsValue())+1];
          strcpy(programstring, child->GetContentsValue());
        }
        break;
      case XMLTOKEN_DECLARE:
        {
          //create a new variable
          csRef<iShaderVariable> var = 
            shadermgr->CreateVariable (child->GetAttributeValue ("name"));
          csStringID idtype = xmltokens.Request( child->GetAttributeValue("type") );
          idtype -= 100;
          var->SetType( (iShaderVariable::VariableType) idtype);
          switch(idtype)
          {
          case iShaderVariable::INT:
            var->SetValue( child->GetAttributeValueAsInt("default") );
            break;
          case iShaderVariable::VECTOR1:
            var->SetValue( child->GetAttributeValueAsFloat("default") );
            break;
          case iShaderVariable::STRING:
            var->SetValue(new scfString( child->GetAttributeValue("default")) );
            break;
          case iShaderVariable::VECTOR3:
            const char* def = child->GetAttributeValue("default");
            csVector3 v;
            sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
            var->SetValue( v );
            break;
          }
          // @@@ I'll blame Matze if this is bad :) /Anders Stenberg
          var->IncRef (); 
          variables.Put( csHashCompute(var->GetName()), var);
        }
        break;
      case XMLTOKEN_VARIABLEMAP:
        {
          //create a varable<->register mapping
          variablemapentry * map = new variablemapentry();
          const char* varname = child->GetAttributeValue("variable");
          map->name = new char[strlen(varname)+1];
          memset(map->name, 0, strlen(varname)+1); 
          memcpy(map->name, varname, strlen(varname));

          const char* cgvarname = child->GetAttributeValue("cgvar");
          map->cgvarname = new char[strlen(cgvarname)+1];
          memset(map->cgvarname, 0, strlen(cgvarname)+1); 
          memcpy(map->cgvarname, cgvarname, strlen(cgvarname));
          map->parameter = NULL;

          //save it for later
          variablemap.Push( map );
        }
        break;
      case XMLTOKEN_STREAMMAP:
        {
          //create a stream<->attribute mapping
          streammapentry * map = new streammapentry();
          map->name = r3d->GetStringContainer ()->Request (
            child->GetAttributeValue("stream"));

          const char* cgvarname = child->GetAttributeValue("cgvar");
          map->cgvarname = new char[strlen(cgvarname)+1];
          memset(map->cgvarname, 0, strlen(cgvarname)+1); 
          memcpy(map->cgvarname, cgvarname, strlen(cgvarname));
          map->parameter = NULL;

          //save it for later
          streammap.Push( map );
        }
        break;
      default:
        return false;
      }
    }
  }

  return true;
}

  
bool csShaderGLCGVP::Prepare()
{
  return LoadProgramStringToGL(programstring);
}


csBasicVector csShaderGLCGVP::GetAllVariableNames()
{
  csBasicVector res;

  csHashIterator c( &variables);
  while(c.HasNext())
  {
    res.PushSmart( (void*)((iShaderVariable*)c.Next())->GetName());
  }
  return res;
}

iShaderVariable* csShaderGLCGVP::GetVariable(const char* string)
{
  csHashIterator c(&variables, csHashCompute(string));

  if(c.HasNext())
  {
    return (iShaderVariable*)c.Next();
  }

  return NULL;
}

csPtr<iString> csShaderGLCGVP::GetProgramID()
{
  csMD5::Digest d = csMD5::Encode(programstring);
  scfString* str = new scfString();
  str->Append((const char*)d.data[0], 16);
  return csPtr<iString>(str);
}
