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
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/stringreader.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
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
  csRenderMeshModes& modes,
  const csShaderVarStack &stacks)
{
  if (override.IsValid())
  {
    override->SetupState (mesh, modes, stacks);
    return;
  } 

  csShaderGLCGCommon::SetupState (mesh, modes, stacks);
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
  if (!str || (*str == 0)) return 0;
  while (strchr (separators, *str) != 0) str++;

  size_t charNum = strcspn (str, separators);
  len = charNum;
  return str;
}

static bool TokenEquals (const char* token, size_t tokenLen, const char* cmp)
{
  return (tokenLen == strlen (cmp)) && (strncmp (token, cmp, tokenLen) == 0);
}

typedef csStringFast<8> csProgVarStr;

bool csShaderGLCGVP::Compile (csArray<iShaderVariableContext*> &staticContexts)
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  CGprofile progProf = CG_PROFILE_UNKNOWN;

  // @@@ Hack: Make sure ARB_v_p is used
  if (cg_profile != 0)
    progProf = cgGetProfile (cg_profile);

  if(progProf == CG_PROFILE_UNKNOWN)
    progProf = cgGLGetLatestProfile (CG_GL_VERTEX);
  if (progProf < CG_PROFILE_ARBVP1)
  {
    delete[] cg_profile;
    cg_profile = csStrNew ("arbvp1");
  }

  if (!DefaultLoadProgram (programStr, CG_GL_VERTEX, staticContexts))
    return false;

  progProf = cgGetProgramProfile (program);
  if (progProf == CG_PROFILE_ARBVP1)
  {
    if (!shaderPlug->arbplg)
      return false;

    /* Cg 1.3 generates ARB_v_p code that the ATI drivers can't grok,
     * bah. Work around by rewriting the VP to a syntax ATI likes. */

    const char* compiledProgram = cgGetProgramString (program, 
      CG_COMPILED_PROGRAM);
        
    csStringReader reader (compiledProgram);
    csString newProgram;

    csHash<csString, csProgVarStr, csConstCharHashKeyHandler> varSemantics;

    /* Parser #blah information emitted by Cg from start of VP code, use it to
     * glean state matrix semantic for certain vars. */
    csString line;
    while (reader.GetLine (line))
    {
      size_t tokenLen;
      const char* token = GetToken (line, " ", tokenLen);
      if (!token) continue;

      if (TokenEquals (token, tokenLen, "#var"))
      {
	do
	{
	  token = GetToken (token + tokenLen, " ", tokenLen);
	  if (!token || (!TokenEquals (token, tokenLen, "float4x4")))
	    break;
	  size_t varLen;
	  const char* varName = GetToken (token + tokenLen, " ", varLen);
	  if (!varName) break;

	  csString varNameCut; 
	  varNameCut.Replace (varName, varLen);
	  CGparameter parameter = cgGetNamedParameter (program, varNameCut);
	  if (parameter == 0) break;
	  if (cgGetParameterResource (parameter) == CG_UNDEFINED)
	    break;
	  unsigned long resIdx = cgGetParameterResourceIndex (parameter);
	  csProgVarStr varStr;
	  varStr.Format ("c[%lu]", resIdx);

	  token = GetToken (varName + varLen, " ", tokenLen);
	  if (!token || (!TokenEquals (token, tokenLen, ":")))
	    break;
	  const char* semantic = GetToken (token + tokenLen, " ", tokenLen);
	  if (!semantic || (*semantic == ':'))
	  {
	    bool backSemantic = true;
	    // Backward compatibility: semantics from name
	  #define REMAP(name, to)					    \
	    if (TokenEquals (varName, varLen, name))			    \
	      varSemantics.Put (varStr, "{ " to " }");			    \
	    else
	  #define REMAP_VARIANTS(name, to)				    \
	    REMAP(name "IT", to ".invtrans")			    	    \
	    REMAP(name "T", to ".transpose")			    	    \
	    REMAP(name "I", to ".inverse")				    \
	    REMAP(name, to)
	  #define MATRIX_MAP(name, nvMatrix, arbMatrix)			    \
	    REMAP_VARIANTS (# name, arbMatrix)
	  #include "cgvp_matrixmap.inc"
	    { /* last else */ backSemantic = false; }
	  #undef REMAP
	  #undef REMAP_VARIANTS

	    if (backSemantic)
	    {
	      static bool deprecatedSemanticsWarn = false;
	      if (!deprecatedSemanticsWarn)
	      {
		deprecatedSemanticsWarn = true;
		shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY, 
		  "VP %s: matrix semantic via names are deprecated; "
		  "use Cg ': state.matrix....' semantics instead", 
		  description);
	      }
	    }
	  }
	  else if (csStrNCaseCmp (semantic, "state.matrix.", 13) == 0)
	  {
	    // Semantics binding in Cg
	    varSemantics.Put (varStr, 
	      csString().Format ("{ %s }", 
		csString ().Replace (semantic, tokenLen).GetData()).Downcase());
	  }
	}
	while (0);
      }    
      else if ((*token != '!') && (*token != '#'))
      {
	// Reached end of #blah block
	break;
      }

      newProgram << line << '\n';
    }

    // Write out our own PARAMs.
    csArray<csShaderVarMapping> mappings;
    csHash<csProgVarStr, csProgVarStr, csConstCharHashKeyHandler> progVarMap;

    CGparameter parameter = cgGetFirstLeafParameter (program, CG_PROGRAM);
    while (parameter)
    {
      CGresource resource = cgGetParameterResource (parameter);
      if (resource != CG_UNDEFINED)
      {
	CGtype paramType = cgGetParameterType (parameter);
	unsigned long resIdx = cgGetParameterResourceIndex (parameter);
	if (resource == CG_C)
	{
	  CGenum variability = cgGetParameterVariability (parameter);
	  if (variability == CG_UNIFORM)
	  {
	    // Uniform values, usually state matrix or SV binding
	    csString paramName;
	    csString semantic;
	    // Use right syntax depending on parameter type 
#define CG_DATATYPE_MACRO(name, compiler_name, enum_name, nrows, ncols)	    \
	    case enum_name:						    \
	      if (nrows == 0)						    \
	      {								    \
		paramName.Format ("p%lu", resIdx);			    \
		semantic.Format ("program.local[%lu]", resIdx);		    \
		progVarMap.Put (csString().Format ("c[%lu]", resIdx),	    \
		  paramName);						    \
	      }								    \
	      else							    \
	      {								    \
		paramName.Format ("p%lu[%d]", resIdx, nrows);		    \
		semantic.Format ("program.local[%lu..%lu]", resIdx,	    \
		  resIdx + nrows-1);					    \
		for (int r = 0; r < nrows; r++)				    \
		{							    \
		  progVarMap.Put (csString().Format ("c[%lu]", resIdx+r),   \
		    csString().Format ("p%lu[%d]", resIdx, r));		    \
		}							    \
	      }								    \
	      break;

	    switch (paramType)
	    {
#include <Cg/cg_datatypes.h>
	      default:
		CS_ASSERT_MSG ("Invalid enum", false);
	    }
#undef CG_DATATYPE_MACRO

	    csProgVarStr oldStr, newStr;
	    oldStr.Format ("c[%lu]", resIdx);
	    if (varSemantics.In (oldStr))
	      semantic = *varSemantics.GetElementPointer (oldStr);
	    newProgram.AppendFmt ("PARAM %s = %s;\n",
	      paramName.GetData(), semantic.GetData());
	  }
	  else if (variability == CG_CONSTANT)
	  {
	    // Constant value
	    int nValues;
	    const double* values;
	    csString paramName;
	    csString valueStr;
	    // Use right syntax depending on parameter type 
#define CG_DATATYPE_MACRO(name, compiler_name, enum_name, nrows, ncols)	    \
	    case enum_name:						    \
	      values = cgGetParameterValues (parameter, CG_CONSTANT,	    \
		&nValues);						    \
	      if (nrows == 0)						    \
	      {								    \
		paramName.Format ("p%lu", resIdx);			    \
		progVarMap.Put (csString().Format ("c[%lu]", resIdx),	    \
		  paramName);						    \
		if (nValues != 0)					    \
		{							    \
		  valueStr << values[0];				    \
		  for (int v = 1; v < nValues; v++)			    \
		  {							    \
		    valueStr << ", " << values[v];			    \
		  }							    \
		}							    \
	      }								    \
	      else							    \
	      {								    \
		CS_ASSERT_MSG ("Add support for matrix constants...",	    \
		  false);						    \
	      }								    \
	      break;

	    switch (paramType)
	    {
#include <Cg/cg_datatypes.h>
	      default:
		CS_ASSERT_MSG ("Invalid enum", false);
	    }
#undef CG_DATATYPE_MACRO

	    newProgram.AppendFmt ("PARAM %s = { %s };\n",
	      paramName.GetData(), valueStr.GetData());
	  }
	  else
	  {
	    CS_ASSERT_MSG ("Add support for more Cg variabilities...", false);
	  }
	}
      }
      parameter = cgGetNextLeafParameter (parameter);
    }

    // Kill PARAM statement emitted by Cg
    while (line[line.Length() - 1] != ';')
    {
      if (!reader.GetLine (line)) break;
    }

    // Copy remaining lines
    while (reader.GetLine (line))
    {
      size_t tokenLen;
      const char* token = GetToken (line, " ", tokenLen);
      if (!token) continue;

      do
      {
	if (*token != '#')
	{
	  // Process a line of code
	  csHash<csProgVarStr, csProgVarStr, csConstCharHashKeyHandler>::GlobalIterator
	    mapIt (progVarMap.GetIterator());
	  while (mapIt.HasNext())
	  {
	    csProgVarStr oldStr;
	    csProgVarStr newStr = mapIt.Next (oldStr);
	    line.ReplaceAll (oldStr, newStr);
	  }
	}
      } while(0);

      newProgram << line << '\n';
    }
    WriteAdditionalDumpInfo ("VP after processing", newProgram);

    // Generate variable map for ARB plugin
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
	csStringFast<20> regnum;
	regnum.Format ("register %lu", cgGetParameterResourceIndex (parameter));
        mappings.Push (csShaderVarMapping (variablemap[i].name, regnum));
      }
    }
    variablemap.DeleteAll();

    override = shaderPlug->arbplg->CreateProgram ("vp");
    if (!override)
      return false;

    if (!override->Load (0, newProgram, mappings))
      return false;

    return override->Compile (staticContexts);
  }
  else if (progProf < CG_PROFILE_ARBVP1)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING, 
      "VP %s: profile '%s' not officially supported", 
      description, cgGetProfileString (progProf));
  }

  return true;
}

