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

#include "glshader_cgfp.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGFP);

void csShaderGLCGFP::Activate()
{
  if (pswrap)
  {
    pswrap->Activate ();
    return;
  } 

  csShaderGLCGCommon::Activate();
}

void csShaderGLCGFP::Deactivate()
{
  if (pswrap)
  {
    pswrap->Deactivate ();
    return;
  } 

  csShaderGLCGCommon::Deactivate();
}

void csShaderGLCGFP::SetupState (const csRenderMesh* mesh,
                                 csRenderMeshModes& modes,
                                 const csShaderVarStack &stacks)
{
  if (pswrap)
  {
    pswrap->SetupState (mesh, modes, stacks);
    return;
  } 

  csShaderGLCGCommon::SetupState (mesh, modes, stacks);
}

void csShaderGLCGFP::ResetState()
{
  if (pswrap)
  {
    pswrap->ResetState ();
    return;
  } 
  csShaderGLCGCommon::ResetState();
}

bool csShaderGLCGFP::Compile (csArray<iShaderVariableContext*> &staticContexts)
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  size_t i;

  // See if we want to wrap through the PS plugin
  // (psplg will be 0 if wrapping isn't wanted)
  if (shaderPlug->psplg)
  {
    program = cgCreateProgram (shaderPlug->context, CG_SOURCE,
      programStr, CG_PROFILE_PS_1_3, entrypoint ? entrypoint : "main", 0);

    if (!program)
      return false;

    pswrap = shaderPlug->psplg->CreateProgram ("fp");

    if (!pswrap)
      return false;

    csArray<csShaderVarMapping> mappings;
    
    for (i = 0; i < variablemap.Length (); i++)
    {
      // Get the Cg parameter
      CGparameter parameter = cgGetNamedParameter (
	program, variablemap[i].destination);
      // Check if it's found, and just skip it if not.
      if (!parameter)
        continue;
      if (!cgIsParameterReferenced (parameter))
	continue;
      // Make sure it's a C-register
      CGresource resource = cgGetParameterResource (parameter);
      if (resource == CG_C)
      {
        // Get the register number, and create a mapping
        char regnum[3];
        sprintf (regnum, "c%lu", cgGetParameterResourceIndex (parameter));
        mappings.Push (csShaderVarMapping (variablemap[i].name, regnum));
      }
    }

    if (pswrap->Load (0, cgGetProgramString (program, CG_COMPILED_PROGRAM), 
      mappings))
    {
      return pswrap->Compile (staticContexts);
    }
    else
    {
      return false;
    }
  }
  else
  {
    return DefaultLoadProgram (programStr, CG_GL_FRAGMENT, staticContexts);
  }

  return true;
}

int csShaderGLCGFP::ResolveTextureBinding (const char* binding)
{
  int newTU = -1;
  if (program)
  {
    CGparameter parameter = cgGetNamedParameter (program, binding);
    if (parameter)
    {
      if ((cgGetParameterBaseResource (parameter) == CG_TEXUNIT0) ||
	(cgGetParameterBaseResource (parameter) == CG_TEX0))
      {
	newTU = cgGetParameterResourceIndex (parameter);
      }
    }
  }

  return newTU;
}
