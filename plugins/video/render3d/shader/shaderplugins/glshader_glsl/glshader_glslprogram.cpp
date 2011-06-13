/*
  Copyright (C) 2011 by Antony Martin

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
#include "csutil/databuf.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/stringquote.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "glshader_glsl.h"
#include "glshader_glslshader.h"
#include "glshader_glslprogram.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLSLProgram);

void csShaderGLSLProgram::Activate ()
{
  shaderPlug->ext->glUseProgramObjectARB (program_id);
}

void csShaderGLSLProgram::Deactivate()
{
  // may be overkill: shader deactivation seems to be performed each time
  // an object is rendered, but this deactivation does represent an useless
  // overhead if the next object to render also uses shaders
  shaderPlug->ext->glUseProgramObjectARB (0);
}

void csShaderGLSLProgram::SetupState (const CS::Graphics::RenderMesh* /*mesh*/, 
                                      CS::Graphics::RenderMeshModes& /*modes*/,
                                      const csShaderVariableStack& stack)
{
  size_t i;
  const csGLExtensionManager* ext = shaderPlug->ext;
  csRef<csShaderVariable> var;

  // set variables
  for (i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];

    var = csGetShaderVariableFromStack (stack, mapping.name);
    if (!var.IsValid ())
      var = mapping.mappingParam.var;

    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    switch (var->GetType ())
    {
    case csShaderVariable::INT:
      {
        int v;
        if (var->GetValue (v))
          ext->glUniform1iARB (mapping.userVal, v);
      }
      break;    
    case csShaderVariable::FLOAT:
      {
        float v;
        if (var->GetValue (v))
          ext->glUniform1fARB (mapping.userVal, v);
      }
      break;    
    case csShaderVariable::VECTOR2:
      {
        csVector2 v;
        if (var->GetValue (v))
          ext->glUniform2fARB (mapping.userVal, v[0], v[1]);
      }
      break;    
    case csShaderVariable::VECTOR3:
      {
        csVector3 v;
        if (var->GetValue (v))
          ext->glUniform3fARB (mapping.userVal, v[0], v[1], v[2]);
      }
      break;    
    case csShaderVariable::VECTOR4:
      {
        csVector4 v;
        if (var->GetValue (v))
          ext->glUniform4fARB (mapping.userVal, v[0], v[1], v[2], v[3]);
      }
      break;    
    case csShaderVariable::MATRIX:
      {
        csMatrix3 m;
        if (var->GetValue (m))
        {
          float matrix[16];
          makeGLMatrix (m, matrix);
          ext->glUniformMatrix4fvARB (mapping.userVal, 1, GL_FALSE, matrix);
        }
      }
      break;
    case csShaderVariable::TRANSFORM:
      {
        csReversibleTransform t;
        if (var->GetValue (t))
        {
          float matrix[16];
          makeGLMatrix (t, matrix);
          ext->glUniformMatrix4fvARB (mapping.userVal, 1, GL_FALSE, matrix);
        }
      }
      break;
    case csShaderVariable::ARRAY:
      break;
    default:
      break;
    }
  }
}

void csShaderGLSLProgram::ResetState ()
{
}

csRef<iDataBuffer> csShaderGLSLProgram::LoadSource (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    if (!ParseCommon (child))
      return 0;
  }
  return GetProgramData ();
}

bool csShaderGLSLProgram::Load (iShaderDestinationResolver*,
                                iDocumentNode* node)
{
  if (!node)
    return false;

  csRef<iDocumentNode> program;

  program = node->GetNode ("vp");
  if (program)
  {
    vpSource = LoadSource (program);
    if (!vpSource)
      return false;
  }

  program = node->GetNode ("fp");
  if (program)
  {
    fpSource = LoadSource (program);
    if (!fpSource)
      return false;
  }

  return true;
}

bool csShaderGLSLProgram::Load (iShaderDestinationResolver*, const char* program, 
                                csArray<csShaderVarMapping> &mappings)
{
  // makes no sense for an "unified" shader to be loaded from one single source
  return false;
}


void csShaderGLSLProgram::SetupVmap ()
{
  size_t i;
  const csGLExtensionManager* ext = shaderPlug->ext;
  csRef<csShaderVariable> var;

  for (i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];

    mapping.userVal = ext->glGetUniformLocationARB (program_id,
                                                    mapping.destination.GetData ());
    if (mapping.userVal == -1)
    {
      // the uniform variable doesnt exist!
      variablemap.DeleteIndex (i);
    }
  }
}

csPtr<csShaderGLSLShader> csShaderGLSLProgram::CreateVP () const
{
  return csPtr<csShaderGLSLShader> (
    new csShaderGLSLShader (shaderPlug, "vertex", GL_VERTEX_SHADER_ARB));
}
csPtr<csShaderGLSLShader> csShaderGLSLProgram::CreateFP () const
{
  return csPtr<csShaderGLSLShader> (
    new csShaderGLSLShader (shaderPlug, "fragment", GL_FRAGMENT_SHADER_ARB));
}

bool csShaderGLSLProgram::Compile (iHierarchicalCache*, csRef<iString>* tag)
{
  int status;                   // link status

  shaderPlug->Open ();

  const csGLExtensionManager* ext = shaderPlug->ext;
  if (!ext)
    return false;

  // glCreateProgram() does not seem to be available
  program_id = ext->glCreateProgramObjectARB ();

  if (vpSource)
  {
    if (!ext->CS_GL_ARB_vertex_shader)
      return false;

    vp = CreateVP ();
    if (!vp->Compile (vpSource->GetData ()))
      return false;

    ext->glAttachObjectARB (program_id, vp->GetID ());
  }
  if (fpSource)
  {
    if (!ext->CS_GL_ARB_fragment_shader)
      return false;

    fp = CreateFP ();
    if (!fp->Compile (fpSource->GetData ()))
      return false;

    ext->glAttachObjectARB (program_id, fp->GetID ());
  }

  ext->glLinkProgramARB (program_id);
  ext->glGetObjectParameterivARB (program_id, GL_OBJECT_LINK_STATUS_ARB,
                                  &status);
  if (!status)
  {
    int size;

    ext->glGetObjectParameterivARB (program_id, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                    &size);
    csString logs((size_t)(size + 1));
    // cast hax
    ext->glGetInfoLogARB (program_id, size, NULL, (GLcharARB*)logs.GetData ());

    shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
                        "Couldn't link %s", description.GetData ());
    shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING, "Error string: %s", 
                        CS::Quote::Single (logs.GetData ()));
    return false;
  }

  // glValidateProgram() ?

  SetupVmap ();

  tag->AttachNew (new scfString ("default"));

  return true;
}


csVertexAttrib csShaderGLSLProgram::ResolveBufferDestination (const char* binding)
{
  csVertexAttrib dest = CS_VATTRIB_INVALID;
  const csGLExtensionManager* ext = shaderPlug->ext;

  if (ext)
  {
    GLint loc = ext->glGetAttribLocationARB (program_id, binding);
    dest = (csVertexAttrib)(CS_VATTRIB_0 + loc);
    // TODO: check for CS conventions about bindings between generic
    //       attributes and specific attributes like position, texcoord, etc.
  }

  return dest;
}

int csShaderGLSLProgram::ResolveTU (const char* binding)
{
  // TODO
  return 0;
}
