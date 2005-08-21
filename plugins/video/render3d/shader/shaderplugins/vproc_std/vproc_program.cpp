/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlight.h"
#include "csgfx/shadervar.h"

#include "imap/services.h"
#include "iutil/document.h"
#include "ivideo/rendermesh.h"

#include "vproc_program.h"

CS_LEAKGUARD_IMPLEMENT (csVProcStandardProgram);

SCF_IMPLEMENT_IBASE_EXT(csVProcStandardProgram)
SCF_IMPLEMENT_IBASE_EXT_END


csVProcStandardProgram::csVProcStandardProgram (csVProc_Std *plug)
  : csShaderProgram (plug->objreg), shaderPlugin (plug), 
  lightMixMode (LIGHTMIXMODE_NONE), 
  colorMixMode (LIGHTMIXMODE_NONE), 
  numLights (0), useAttenuation (true),
  positionBuffer (CS_BUFFER_POSITION),
  normalBuffer (CS_BUFFER_NORMAL),
  colorBuffer (CS_BUFFER_COLOR_UNLIT)
{
  InitTokenTable (tokens);
}

csVProcStandardProgram::~csVProcStandardProgram ()
{
}

void csVProcStandardProgram::Activate ()
{
}

void csVProcStandardProgram::Deactivate ()
{
}

void csVProcStandardProgram::SetupState (const csRenderMesh* mesh,
                                         csRenderMeshModes& modes,
                                         const csShaderVarStack &stacks)
{
  if (numLights > 0)
  {
    int lightsActive = 0;
    csStringID id;
    id = shaderPlugin->lsvCache.GetDefaultSVId (
      csLightShaderVarCache::varLightCount);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (lightsActive);
    if (lightsActive == 0) return;

    iRenderBuffer *vbuf = GetBuffer (positionBuffer, modes, stacks);
    iRenderBuffer *nbuf = GetBuffer (normalBuffer, modes, stacks);
    iRenderBuffer *cbuf = GetBuffer (colorBuffer, modes, stacks);

    if (vbuf == 0 || nbuf == 0) return;

    //copy output
    csRef<iRenderBuffer> clbuf = 
      csRenderBuffer::CreateRenderBuffer (vbuf->GetElementCount (), CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);

    // tempdata
    csColor *tmpColor = (csColor*) clbuf->Lock (CS_BUF_LOCK_NORMAL);

    size_t elementCount = vbuf->GetElementCount ();
    memset (tmpColor, 0, sizeof(csColor) * elementCount);

    csReversibleTransform object2world;
    csGetShaderVariableFromStack (stacks, 
      shaderPlugin->string_object2world)->GetValue (object2world);

    if (lightMixMode == LIGHTMIXMODE_NONE)
    {
      //only calculate last, other have no effect
      csLightProperties light (csMin((size_t)lightsActive, numLights)-1,
	shaderPlugin->lsvCache, stacks);
      iVertexLightCalculator *calc = 
	shaderPlugin->GetLightCalculator (light, useAttenuation);
      calc->CalculateLighting (light, object2world, elementCount,
        csVertexListWalker<csVector3> (vbuf->Lock (CS_BUF_LOCK_READ),
	  elementCount, vbuf->GetStride ()),
        csVertexListWalker<csVector3> (nbuf->Lock (CS_BUF_LOCK_READ),
	  elementCount, nbuf->GetStride ()), 
        tmpColor);
    }
    else
    {
      LightMixmode useMixMode = LIGHTMIXMODE_ADD;
      for (size_t i = 0; i < (csMin((size_t)lightsActive, numLights)); i++)
      {
	csLightProperties light (i, shaderPlugin->lsvCache, stacks);
        iVertexLightCalculator *calc = 
	  shaderPlugin->GetLightCalculator (light, useAttenuation);

        switch (useMixMode)
        {
        case LIGHTMIXMODE_ADD:
          {
            calc->CalculateLightingAdd (light, object2world, elementCount,
              csVertexListWalker<csVector3> (vbuf->Lock (CS_BUF_LOCK_READ), 
		elementCount, vbuf->GetStride ()),
              csVertexListWalker<csVector3> (nbuf->Lock (CS_BUF_LOCK_READ),
		elementCount, nbuf->GetStride ()), 
              tmpColor);
            break;
          }
        case LIGHTMIXMODE_MUL:
          {
            calc->CalculateLightingMul (light, object2world, elementCount,
              csVertexListWalker<csVector3> (vbuf->Lock (CS_BUF_LOCK_READ),
		elementCount, vbuf->GetStride ()),
              csVertexListWalker<csVector3> (nbuf->Lock (CS_BUF_LOCK_READ),
	        elementCount, nbuf->GetStride ()), 
              tmpColor);
            break;
          }
        case LIGHTMIXMODE_NONE:
	  break;
        }
	useMixMode = lightMixMode;
      }
    }

    if (cbuf && (colorMixMode != LIGHTMIXMODE_NONE))
    {
      csVertexListWalker<csColor> cbufWalker (cbuf->Lock (CS_BUF_LOCK_READ),
	elementCount, vbuf->GetStride ());
      
      if (colorMixMode == LIGHTMIXMODE_ADD)
      {
	for (size_t i = 0; i < elementCount; i++)
	  tmpColor[i] += cbufWalker[i];
      }
      else
      {
	for (size_t i = 0; i < elementCount; i++)
	  tmpColor[i] *= cbufWalker[i];
      }

      cbuf->Release ();
    }

    float finalLightFactorReal = GetParamFloatVal (stacks, finalLightFactor,
      1.0f);
    for (size_t i = 0; i < elementCount; i++)
      tmpColor[i] *= finalLightFactorReal;

    vbuf->Release ();
    nbuf->Release ();
    clbuf->Release ();
    modes.buffers->SetRenderBuffer (CS_BUFFER_COLOR, clbuf);
  }
}

void csVProcStandardProgram::ResetState ()
{
}

bool csVProcStandardProgram::Compile ()
{
  return true;
}

bool csVProcStandardProgram::Load (iShaderTUResolver* tuResolve, 
				   const char* program, 
                                   csArray<csShaderVarMapping>& mappings)
{
  return false;
}
				    
bool csVProcStandardProgram::ParseLightMixMode (iDocumentNode* child, 
						LightMixmode& mixmode)
{
  const char *str = child->GetContentsValue ();
  if (str)
  {
    if (strcasecmp (str, "none"))
      mixmode = LIGHTMIXMODE_NONE;
    else if (strcasecmp (str, "add"))
      mixmode = LIGHTMIXMODE_ADD;
    else if (strcasecmp (str, "multiply"))
      mixmode = LIGHTMIXMODE_MUL;
    else
    {
      synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
	child, "Invalid light mix mode '%s'", str);
      return false;
    }
    return true;
  }
  return false;
}

bool csVProcStandardProgram::ParseBufferName (iDocumentNode* child, 
					      BufferName& name)
{
  const char* str = child->GetContentsValue();
  if (!str) 
  {
    synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
      child, "Expected buffer name");
    return false;
  }
  name.defaultName = csRenderBuffer::GetBufferNameFromDescr (str);

  if (name.defaultName == CS_BUFFER_NONE)
    name.userName = strings->Request (str);

  return true;
}

bool csVProcStandardProgram::Load (iShaderTUResolver* tuResolve, iDocumentNode* program)
{
  if (!program)
    return false;

  csRef<iDocumentNode> pnode = program->GetNode("vproc_std");
  if(pnode)
  {
    csRef<iDocumentNodeIterator> it = pnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = tokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_LIGHTS:
	numLights = child->GetContentsValueAsInt ();
	break;
      case XMLTOKEN_FINALFACTOR:
	if (!ParseProgramParam (child, finalLightFactor,
	  ParamFloat | ParamShaderExp))
	  return false;
	break;
      case XMLTOKEN_ATTENUATION:
	if (!synsrv->ParseBool (child, useAttenuation, true))
	  return false;
	break;
      case XMLTOKEN_LIGHTMIXMODE:
	if (!ParseLightMixMode (child, lightMixMode))
	  return false;
	break;
      case XMLTOKEN_COLORMIXMODE:
	if (!ParseLightMixMode (child, colorMixMode))
	  return false;
	break;
      case XMLTOKEN_POSITIONBUFFER:
	if (!ParseBufferName (child, positionBuffer))
	  return false;
	break;
      case XMLTOKEN_NORMALBUFFER:
	if (!ParseBufferName (child, normalBuffer))
	  return false;
	break;
      case XMLTOKEN_COLORBUFFER:
	if (!ParseBufferName (child, colorBuffer))
	  return false;
	break;
      default:
        {
          switch (commonTokens.Request (value))
          {
          case XMLTOKEN_PROGRAM:
          case XMLTOKEN_VARIABLEMAP:
            // Don't want those
            synsrv->ReportBadToken (child);
            return false;
            break;
          default:
            if (!ParseCommon (child))
              return false;
          }
        }
      }
    }
  }
  else
  {
    synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
      pnode, "<vproc_std> node missing");
  }

  return true;
}

iRenderBuffer* csVProcStandardProgram::GetBuffer (const BufferName& name,
  csRenderMeshModes& modes, const csShaderVarStack &stacks)
{
  if (name.defaultName != CS_BUFFER_NONE)
    return modes.buffers->GetRenderBuffer (name.defaultName);

  if ((stacks.Length() > name.userName) && (stacks[name.userName]))
  {
    iRenderBuffer* buf;
    stacks[name.userName]->GetValue (buf);
    return buf;
  }
  return 0;
}
