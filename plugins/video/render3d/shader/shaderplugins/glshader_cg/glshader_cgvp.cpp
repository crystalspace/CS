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

#include "csutil/bitarray.h"
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

#include "csplugincommon/opengl/glextmanager.h"

#include "glshader_cgvp.h"
#include "glshader_cg.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGVP);

void csShaderGLCGVP::Activate()
{
  if (override.IsValid())
  {
    override->Activate();
    return;
  }

  csShaderGLCGCommon::Activate();

  if (cgTrackMatrices)
  {
    for (size_t i = 0; i < cgMatrixTrackers.Length(); ++i)
    {
      const CGMatrixTrackerEntry& mt = cgMatrixTrackers[i];

      cgGLSetStateMatrixParameter(mt.cgParameter, mt.cgMatrix, 
	mt.cgTransform);
    }
  }
  if (nvTrackMatrices)
  {
    for (size_t i = 0; i < nvMatrixTrackers.Length(); ++i)
    {
      const NVMatrixTrackerEntry& mt = nvMatrixTrackers[i];

      shaderPlug->ext->glTrackMatrixNV (GL_VERTEX_PROGRAM_NV,
	mt.nvParameter, mt.nvMatrix, mt.nvTransform);
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

const CGGLenum CGMatrixMODELVIEW_PROJECTION = 
  CG_GL_MODELVIEW_PROJECTION_MATRIX;
const CGGLenum CGMatrixMODELVIEW = CG_GL_MODELVIEW_MATRIX;
const CGGLenum CGMatrixPROJECTION = CG_GL_PROJECTION_MATRIX;

const GLenum NVMatrixMODELVIEW_PROJECTION = GL_MODELVIEW_PROJECTION_NV;
const GLenum NVMatrixMODELVIEW = GL_MODELVIEW;
const GLenum NVMatrixPROJECTION = GL_PROJECTION;

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
	  
	  if (((size_t)varIndex >= remap.Length()) || (remap[varIndex] == 0)) break;

	  line.Format ("PARAM c%d[4] = { %s };", varIndex, remap[varIndex]);
	}
      }
      while (0);

      newProgram << line << '\n';
    }
    WriteAdditionalDumpInfo ("VP after processing", newProgram);

    override = shaderPlug->arbplg->CreateProgram ("vp");
    if (!override)
      return false;

    csArray<csShaderVarMapping> mappings;

    for (size_t i = 0; i < variablemap.Length (); i++)
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
    if (!shaderPlug->doNVVPrealign)
    {
      CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
      while (param)
      {
	const char* pname = cgGetParameterName (param);
      #define TRACKERENTRY(Matrix, Modifier)		  \
	CGMatrixTrackerEntry map;			  \
	map.cgMatrix = CGMatrix ## Matrix;		  \
	map.cgTransform = Modifier;			  \
	map.cgParameter = param;			  \
	cgMatrixTrackers.Push (map);
      #define NAMEDTRACKERENTRY(Name, Matrix, Modifier)	  \
	if (!strcmp (pname, Name))			  \
	{						  \
	  TRACKERENTRY (Matrix, Modifier)		  \
	}						  \
	else
      #define NAMEDENTRIES(Basename, Matrix)		  \
	NAMEDTRACKERENTRY (Basename, Matrix,		  \
	  CG_GL_MATRIX_IDENTITY)			  \
	NAMEDTRACKERENTRY (Basename "I", Matrix,	  \
	  CG_GL_MATRIX_INVERSE)				  \
	NAMEDTRACKERENTRY (Basename "T", Matrix,	  \
	  CG_GL_MATRIX_TRANSPOSE)			  \
	NAMEDTRACKERENTRY (Basename "IT", Matrix,	  \
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

      cgTrackMatrices = (cgMatrixTrackers.Length() != 0);
    }
    else
    {
      /*
      * Realign float4x4 parameters on 4-register boundaries, keep all other
      * variables were they are.
      * @@@ PROBLEM: In complex VPs, it could happen that not enough registers
      * are available for both aligning and keeping other regs were they are.
      * Currently, this will likely result in load failure. Should be handled
      * more gracefully.
      */

      const char* compiledProgram = cgGetProgramString (program, 
	CG_COMPILED_PROGRAM);
          
      csStringReader reader (compiledProgram);
      csString newProgram;

      GLint numTemps = 0;
      if (shaderPlug->ext->CS_GL_ARB_vertex_program)
      {
	// VP2.0+ are more or less additions to ARB_v_p.
	// So query the param count from there.
	shaderPlug->ext->glGetProgramivARB (GL_VERTEX_PROGRAM_ARB, 
	  GL_MAX_PROGRAM_PARAMETERS_ARB, &numTemps);
      }
      else
      {
	// Else: Guess.
	numTemps = (progProf == CG_PROFILE_VP20) ? 96 : 256;
      }
      csBitArray usedSlots (numTemps);
      csArray<NVMatrixTrackerEntry> matrixParams;

      // Pass 1: collect which params we need to realign.
      csString line;
      bool nextPass = false;
      while (reader.GetLine (line) && !nextPass)
      {
	size_t tokenLen;
	const char* token = GetToken (line, " ", tokenLen);
	if (!token) continue;

	do
	{
	  if (TokenEquals (token, tokenLen, "#var"))
	  {
	    token = GetToken (token + tokenLen, " ", tokenLen);
	    if (!token)
	      break;
	    bool isMatrix = TokenEquals (token, tokenLen, "float4x4");
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
	    if (sscanf (token, "c[%d]", &varIndex) != 1)
	      break;

	    if (isMatrix)
	    {
	      // Align matrices on 4-register boundaries.
	    #define TRACKERENTRY(Matrix, Modifier)		  \
	      NVMatrixTrackerEntry map;				  \
	      map.nvMatrix = NVMatrix ## Matrix;		  \
	      map.nvTransform = Modifier;			  \
	      map.nvParameter = varIndex;			  \
	      matrixParams.Push (map);
	    #define NAMEDTRACKERENTRY(Name, Matrix, Modifier)	  \
	      if (TokenEquals (varName, varLen, Name))		  \
	      {							  \
		TRACKERENTRY (Matrix, Modifier)			  \
	      }							  \
	      else
	    #define NAMEDENTRIES(Basename, Matrix)		  \
	      NAMEDTRACKERENTRY (Basename, Matrix,		  \
		GL_IDENTITY_NV)					  \
	      NAMEDTRACKERENTRY (Basename "I", Matrix,	  	  \
		GL_INVERSE_NV)					  \
	      NAMEDTRACKERENTRY (Basename "T", Matrix,	  	  \
		GL_TRANSPOSE_NV)				  \
	      NAMEDTRACKERENTRY (Basename "IT", Matrix,	  	  \
		GL_INVERSE_TRANSPOSE_NV)			  

	    #define MATRIX_MAP(name, nvMatrix, arbMatrix)	  \
	      NAMEDENTRIES (# name, nvMatrix)
	    #include "cgvp_matrixmap.inc"
	      { 
		/* last else */ 
		CGparameter param = cgGetNamedParameter (program, 
		  varName);
		if (cgIsParameterReferenced (param))
		{
		  for (int b = 0; b < 4; b++)
		    usedSlots.SetBit (varIndex);
		}
	      }
	    }
	    else
	    {
	      csString nameOnly; nameOnly.Append (varName, varLen);
	      CGparameter param = cgGetNamedParameter (program, 
		nameOnly);
	      if (cgIsParameterReferenced (param))
		usedSlots.SetBit (varIndex);
	    }
	  }
	  else if (TokenEquals (token, tokenLen, "#const"))
	  {
	    size_t varLen;
	    const char* constName = GetToken (token + tokenLen, " ", varLen);
	    if (!constName) break;

	    int varIndex;
	    if (sscanf (constName, "c[%d]", &varIndex) != 1)
	      break;

	    usedSlots.SetBit (varIndex);
	  }
	  else if (*token != '#')
	  {
	    const char* lineData = line.GetData();

	    if (strncmp (lineData, "!!", 2) == 0)
	      break;
	    else
	      // The end of the header with the var infos and so on.
	      nextPass = true;
	  }
	} while(0);
      }

      // Pass 2: Realigning, rewriting of the VP code.
      if (nextPass)
      {
	reader.Reset();

	csHash<int, int> constRemap;

	for (size_t m = 0; m < matrixParams.Length(); m++)
	{
	  NVMatrixTrackerEntry mte = matrixParams[m];
	  int newIndex = 0;
	  bool found = false;
	  // Align matrices on 4-register boundaries.
	  while (newIndex < numTemps)
	  {
	    if (!usedSlots.AreSomeBitsSet (newIndex, 4))
	    {
	      for (int b = 0; b < 4; b++)
	      {
		usedSlots.SetBit (newIndex + b);
		constRemap.Put (mte.nvParameter + b, newIndex + b);
	      }
	      found = true;
	      break;
	    }
	    newIndex += 4;
	  }
	  if (!found) return false;

	  mte.nvParameter = newIndex;
	  nvMatrixTrackers.Push (mte);
	}

	csString line;
	while (reader.GetLine (line))
	{
	  size_t tokenLen;
	  const char* token = GetToken (line, " ", tokenLen);
	  if (!token) continue;

	  do
	  {
	    if (*token != '#')
	    {
	      const char* lineData = line.GetData();

	      if (strncmp (lineData, "!!", 2) == 0)
		break;

	      // Process a line of code

	      csString newLine;

	      while (lineData != 0)
	      {
		const char* param = strstr (lineData, "c[");
		if (param != 0)
		{
		  param += 2;
		  newLine.Append (lineData, param - lineData);
		  int varIndex;
		  if (sscanf (param, "%d", &varIndex) != 1)
		  {
		    newLine.Replace (line);
		    break;
		  }
		  if (constRemap.In (varIndex))
		    newLine << constRemap.Get (varIndex, -1);
		  else
		    newLine << varIndex;
		  lineData = strchr (param, ']');
		}
		else
		{
		  newLine << lineData;
		  lineData = 0;
		}
	      }

	      line.Replace (newLine);
	    }
	  } while(0);

	  newProgram << line << '\n';
	}

	nvTrackMatrices = (nvMatrixTrackers.Length() != 0);

	WriteAdditionalDumpInfo ("VP after processing", newProgram);
      }

      // Now overwrite the VP program with our manipulated version.
      cgGLEnableProfile (progProf);
      cgGLUnbindProgram (progProf);

      shaderPlug->ext->glLoadProgramNV (GL_VERTEX_PROGRAM_NV, 
	cgGLGetProgramID (program), newProgram.Length(), 
	(GLubyte*)newProgram.GetData());

      cgGLDisableProfile (progProf);
    }
  }

  return true;
}

