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
#include "csutil/stringreader.h"
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

#include "glshader_cgvp.h"
#include "glshader_cg.h"

void csShaderGLCGVP::Activate()
{
  if (override.IsValid())
  {
    override->Activate();
    return;
  }

  csShaderGLCGCommon::Activate();

  if (nvTrackMatrices)
  {

    for(int i = 0; i < matrixtrackers.Length(); ++i)
    {
      matrixtrackerentry& mt = matrixtrackers[i];

      cgGLSetStateMatrixParameter(mt.cgParameter,mt.nvMatrix,mt.nvTransform);
    }
  }
}

void csShaderGLCGVP::Deactivate()
{
  if (override.IsValid())
  {
    override->Deactivate();
    return;
  }

  csShaderGLCGCommon::Deactivate();
}

void csShaderGLCGVP::SetupState (const csRenderMesh* mesh,
  const csShaderVarStack &stacks)
{
  if (override.IsValid())
  {
    override->SetupState (mesh, stacks);
    return;
  } 

  csShaderGLCGCommon::SetupState (mesh, stacks);
}

void csShaderGLCGVP::ResetState()
{
  if (override.IsValid())
  {
    override->ResetState ();
    return;
  } 
  csShaderGLCGCommon::ResetState();
}

static const char* GetToken (const char* str, const char* separators,
			     size_t& len)
{
  if (!str) return 0;
  while (strchr (separators, *str) != 0) str++;

  size_t charNum = strcspn (str, separators);
  len = charNum;
  return str;
}

static bool TokenEquals (const char* token, size_t tokenLen, const char* cmp)
{
  return (tokenLen == strlen (cmp)) && (strncmp (token, cmp, tokenLen) == 0);
}



bool csShaderGLCGVP::Compile(csArray<iShaderVariableContext*> &staticContexts)
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  if (!DefaultLoadProgram (programStr, CG_GL_VERTEX, staticContexts))
    return false;

  CGprofile progProf = cgGetProgramProfile (program);
  if (progProf == CG_PROFILE_ARBVP1)
  {
    if (!shaderPlug->arbplg)
      return false;

    // Change the ARB VP to use state.matrix.... built-ins for ModelView etc.
    const char* compiledProgram = cgGetProgramString (program, 
      CG_COMPILED_PROGRAM);
        
    csStringReader reader (compiledProgram);
    csString newProgram;

    csArray<const char*> remap;

    csString line;
    while (reader.GetLine (line))
    {
      size_t tokenLen;
      const char* token = GetToken (line, " ", tokenLen);
      if (!token) continue;

      do
      {
	if (TokenEquals (token, tokenLen, "#var"))
	{
	  token = GetToken (token + tokenLen, " ", tokenLen);
	  if (!token || (!TokenEquals (token, tokenLen, "float4x4")))
	    break;
	  size_t varLen;
	  const char* varName = GetToken (token + tokenLen, " ", varLen);
	  if (!varName) break;

	  token = GetToken (varName + varLen, " ", tokenLen);
	  if (!token || (!TokenEquals (token, tokenLen, ":")))
	    break;
	  token = GetToken (token + tokenLen, " ", tokenLen);
	  if (!token || (!TokenEquals (token, tokenLen, ":")))
	    break;
	  token = GetToken (token + tokenLen, " ", tokenLen);
	  if (!token) break;

	  int varIndex;
	  if (sscanf (token, "c[%d], 4", &varIndex) != 1)
	    break;

	#define REMAP(name, to)						    \
	  if (TokenEquals (varName, varLen, name))			    \
	    remap.GetExtend (varIndex) = to;				    \
	  else
	#define REMAP_VARIANTS(name, to)				    \
	  REMAP(name "IT", to ".invtrans")			    	    \
	  REMAP(name "T", to ".transpose")			    	    \
	  REMAP(name "I", to ".inverse")				    \
	  REMAP(name, to)
	#define MATRIX_MAP(name, nvMatrix, arbMatrix)			    \
	  REMAP_VARIANTS (# name, arbMatrix)
	#include "cgvp_matrixmap.inc"
	  { /* last else */ }
	#undef REMAP
	#undef REMAP_VARIANTS
	}
	else if (TokenEquals (token, tokenLen, "PARAM"))
	{
	  size_t varLen;
	  const char* varName = GetToken (token + tokenLen, " ", varLen);
	  if (!varName) break;

	  int varIndex;
	  if (sscanf (varName, "c%d[4]", &varIndex) != 1)
	    break;
	  
	  if ((varIndex >= remap.Length()) || (remap[varIndex] == 0)) break;

	  line.Format ("PARAM c%d[4] = { %s };", varIndex, remap[varIndex]);
	}
      }
      while (0);

      newProgram << line << '\n';
    }

    override = shaderPlug->arbplg->CreateProgram ("vp");
    if (!override)
      return false;

    csArray<csShaderVarMapping> mappings;

    for (int i = 0; i < variablemap.Length (); i++)
    {
      // Get the Cg parameter
      CGparameter parameter = (CGparameter)variablemap[i].userPtr;
      // Check if it's found, and just skip it if not.
      // Make sure it's a C-register
      CGresource resource = cgGetParameterResource (parameter);
      if (resource == CG_C)
      {
        // Get the register number, and create a mapping
        char regnum[20];
        sprintf (regnum, "register %lu", cgGetParameterResourceIndex (parameter));
        mappings.Push (csShaderVarMapping (variablemap[i].name, regnum));
      }
    }
    variablemap.DeleteAll();

    if (!override->Load (newProgram, mappings))
      return false;

    return override->Compile (staticContexts);
  }
  else
  {
    CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
    while (param)
    {
      const char* pname = cgGetParameterName (param);
    #define TRACKERENTRY(Matrix, Modifier)		  \
      matrixtrackerentry map;				  \
      map.nvMatrix = (CGGLenum)Matrix;				  \
      map.nvTransform = (CGGLenum)Modifier;			  \
      map.cgParameter = param; \
      matrixtrackers.Push (map);
    #define NAMEDTRACKERENTRY(Name, Matrix, Modifier)	  \
      if (!strcmp (pname, Name))			  \
      {							  \
	TRACKERENTRY (Matrix, Modifier)			  \
      }							  \
      else
    #define NAMEDENTRIES(Basename, Matrix)		  \
      NAMEDTRACKERENTRY (Basename, Matrix,		  \
	CG_GL_MATRIX_IDENTITY)					  \
      NAMEDTRACKERENTRY (Basename "I", Matrix,	  	  \
	CG_GL_MATRIX_INVERSE)					  \
      NAMEDTRACKERENTRY (Basename "T", Matrix,	  	  \
	CG_GL_MATRIX_TRANSPOSE)				  \
      NAMEDTRACKERENTRY (Basename "IT", Matrix,	  	  \
	CG_GL_MATRIX_INVERSE_TRANSPOSE)			  

    #define MATRIX_MAP(name, nvMatrix, arbMatrix)	  \
      NAMEDENTRIES (# name, nvMatrix)
    #include "cgvp_matrixmap.inc"
      { /* last else */ }

    #undef NAMEDENTRIES
    #undef NAMEDTRACKERENTRY
    #undef TRACKERENTRY

      param = cgGetNextLeafParameter (param);
    }

    nvTrackMatrices = true;
  }

  return true;
}

