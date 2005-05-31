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
#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glhelper.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_arb.h"
#include "glshader_afp.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLAFP);

SCF_IMPLEMENT_IBASE_EXT(csShaderGLAFP)
SCF_IMPLEMENT_IBASE_EXT_END

void csShaderGLAFP::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (shaderPlug->object_reg, severity, 
    "crystalspace.graphics3d.shader.glarb", msg, args);
  va_end (args);
}

void csShaderGLAFP::Activate ()
{
  //enable it
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
  shaderPlug->ext->glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program_num);
}

void csShaderGLAFP::Deactivate()
{
  glDisable (GL_FRAGMENT_PROGRAM_ARB);
}

void csShaderGLAFP::SetupState (const csRenderMesh *mesh, 
                                csRenderMeshModes& modes,
	                        const csShaderVarStack &stacks)
{
  size_t i;
  const csGLExtensionManager* ext = shaderPlug->ext;
  csRef<csShaderVariable> var;

  // set variables
  for(i = 0; i < variablemap.Length(); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];

    var = csGetShaderVariableFromStack (stacks, mapping.name);
    if (!var.IsValid ())
      var = mapping.mappingParam.var;

    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    switch (var->GetType ())
    {
    case csShaderVariable::INT:
    case csShaderVariable::FLOAT:
    case csShaderVariable::VECTOR2:
    case csShaderVariable::COLOR:
    case csShaderVariable::VECTOR3:
    case csShaderVariable::VECTOR4:
      {
        csVector4 v;
        if (var->GetValue (v))
          ext->glProgramLocalParameter4fvARB (GL_VERTEX_PROGRAM_ARB, mapping.userVal, &v.x);
      }
      break;    
    case csShaderVariable::MATRIX:
      {
        csMatrix3 m;
        if(var->GetValue(m))
        {
          float matrix[16];
          makeGLMatrix (m, matrix);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal, matrix[0], matrix[4], matrix[8], matrix[12]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+1, matrix[1], matrix[5], matrix[9], matrix[13]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+2, matrix[2], matrix[6], matrix[10], matrix[14]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+3, matrix[3], matrix[7], matrix[11], matrix[15]);
        }
      }
      break;
    case csShaderVariable::TRANSFORM:
      {
        csReversibleTransform t;
        if(var->GetValue(t))
        {
          float matrix[16];
          makeGLMatrix (t, matrix);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal, matrix[0], matrix[4], matrix[8], matrix[12]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+1, matrix[1], matrix[5], matrix[9], matrix[13]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+2, matrix[2], matrix[6], matrix[10], matrix[14]);
          ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
            mapping.userVal+3, matrix[3], matrix[7], matrix[11], matrix[15]);
        }
      }
      break;
    case csShaderVariable::ARRAY:
      {
        csRef<csShaderVariable> cvar;
        if (var->GetArraySize () == 0) break;

        cvar = var->GetArrayElement (0);

        switch (cvar->GetType ())
        {
        case csShaderVariable::INT:
        case csShaderVariable::FLOAT:
        case csShaderVariable::VECTOR2:
        case csShaderVariable::COLOR:
        case csShaderVariable::VECTOR3:
        case csShaderVariable::VECTOR4:
          {
            csVector4 v;
            for (uint i = 0; i < var->GetArraySize (); i++)
            {
              cvar = var->GetArrayElement (i);
              if (cvar->GetValue (v))
                ext->glProgramLocalParameter4fvARB (GL_VERTEX_PROGRAM_ARB, mapping.userVal+i, &v.x);
            }
          }
          break;    
        case csShaderVariable::MATRIX:
          {
            csMatrix3 m;
            for (uint i = 0; i< var->GetArraySize (); i++)
            {
              cvar = var->GetArrayElement (i);
              if(cvar->GetValue(m))
              {
                float matrix[16];
                makeGLMatrix (m, matrix);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+0, matrix[0], matrix[4], matrix[8], matrix[12]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+1, matrix[1], matrix[5], matrix[9], matrix[13]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+2, matrix[2], matrix[6], matrix[10], matrix[14]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+3, matrix[3], matrix[7], matrix[11], matrix[15]);
              }
            }
          }
          break;
        case csShaderVariable::TRANSFORM:
          {
            csReversibleTransform t;
            for (uint i = 0; i< var->GetArraySize (); i++)
            {
              cvar = var->GetArrayElement (i);
              if(cvar->GetValue(t))
              {
                float matrix[16];
                makeGLMatrix (t, matrix);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+0, matrix[0], matrix[4], matrix[8], matrix[12]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+1, matrix[1], matrix[5], matrix[9], matrix[13]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+2, matrix[2], matrix[6], matrix[10], matrix[14]);
                ext->glProgramLocalParameter4fARB (GL_VERTEX_PROGRAM_ARB,
                  mapping.userVal+i*4+3, matrix[3], matrix[7], matrix[11], matrix[15]);
              }
            }
          }
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }
  }
}

void csShaderGLAFP::ResetState ()
{
}

bool csShaderGLAFP::LoadProgramStringToGL ()
{
  if (!shaderPlug->ext)
    return false;

  const csGLExtensionManager* ext = shaderPlug->ext;

  if(!ext->CS_GL_ARB_fragment_program)
    return false;

  //step to first !!
  csRef<iDataBuffer> data = GetProgramData();
  if (!data)
    return false;

  const char* programstring = (char*)data->GetData ();
  size_t stringlen = data->GetSize ();

  size_t i=0;
  while (*programstring != '!' && (i < stringlen))
  {
    ++programstring;
    ++i;
  }
  stringlen -= i;

  ext->glGenProgramsARB(1, &program_num);
  ext->glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program_num);
  
  ext->glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, 
    (GLsizei)stringlen, (void*) programstring);

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
      "Couldn't load fragment program \"%s\"", description);
    Report (CS_REPORTER_SEVERITY_WARNING, "Program error at: \"%s\"", start);
    Report (CS_REPORTER_SEVERITY_WARNING, "Error string: '%s'", 
      programErrorString);
    return false;
  }
  else
  {
    if (doVerbose && (programErrorString != 0) && (*programErrorString != 0))
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Warning for fragment program \"%s\": '%s'", description, 
	programErrorString);
    }
  }

  return true;
}

bool csShaderGLAFP::Load(iShaderTUResolver*, iDocumentNode* program)
{
  if(!program)
    return false;

  csRef<iDocumentNode> variablesnode = program->GetNode("arbfp");
  if (variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      if (!ParseCommon (child))
	return false;
    }
  }

  return true;
}

bool csShaderGLAFP::Compile()
{
  shaderPlug->Open ();

  for (size_t i = 0; i < variablemap.Length ();)
  {
    int dest;
    if (sscanf (variablemap[i].destination, "register %d", &dest) != 1)
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Unknown variable destination %s", 
	variablemap[i].destination.GetData());
      variablemap.DeleteIndex (i);
      continue;
    }

    variablemap[i].userVal = dest;
    i++;
  }

  variablemap.ShrinkBestFit();

  return LoadProgramStringToGL ();
}
