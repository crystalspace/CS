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

#include "ivideo/rendermesh.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlight.h"
#include "csgfx/shadervar.h"
#include "vproc_program.h"

CS_LEAKGUARD_IMPLEMENT (csVProcStandardProgram);

SCF_IMPLEMENT_IBASE_EXT(csVProcStandardProgram)
SCF_IMPLEMENT_IBASE_EXT_END


csVProcStandardProgram::csVProcStandardProgram (csVProc_Std *plug)
  : csShaderProgram (plug->objreg), shaderPlugin (plug), lightMixMode (LIGHTMIXMODE_NONE), 
  finalLightFactor (1.0f), numLights (0), useAttenuation (true)
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
    const csArray<iLight*> lights = shaderPlugin->shaderManager->GetActiveLights ();
    if (lights.Length () == 0) return;

    iRenderBuffer *vbuf = modes.buffers->GetRenderBuffer (CS_BUFFER_POSITION);
    iRenderBuffer *cbuf = modes.buffers->GetRenderBuffer (CS_BUFFER_COLOR_UNLIT);
    iRenderBuffer *nbuf = modes.buffers->GetRenderBuffer (CS_BUFFER_NORMAL);

    if (vbuf == 0 || nbuf == 0) return;

    //copy output
    csRef<iRenderBuffer> clbuf = csRenderBuffer::CreateRenderBuffer (vbuf->GetElementCount (), CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);

    // tempdata
    csColor *tmpColor = (csColor*) clbuf->Lock (CS_BUF_LOCK_NORMAL);


    if (lightMixMode == LIGHTMIXMODE_ADD || lightMixMode == LIGHTMIXMODE_MUL)
    {
      if (cbuf) //copy over colors
        memcpy (tmpColor, cbuf->Lock (CS_BUF_LOCK_READ), vbuf->GetElementCount ()*sizeof(csColor));
      else
        memset (tmpColor, 0, sizeof(csColor)*vbuf->GetElementCount ());
    }

    csReversibleTransform object2world;
    stacks[shaderPlugin->string_object2world].Top()->GetValue (object2world);

    iLight* light = 0;
    size_t elementCount = vbuf->GetElementCount ();

    if (lightMixMode == LIGHTMIXMODE_NONE)
    {
      //only calculate last, other have no effect
      light = lights.Get (csMin(lights.Length (), numLights)-1);
      iVertexLightCalculator *calc = shaderPlugin->GetLightCalculator (light, useAttenuation);
      calc->CalculateLighting (light, object2world, elementCount,
        csVertexListWalker<csVector3> (vbuf->Lock (CS_BUF_LOCK_READ),elementCount,vbuf->GetStride ()),
        csVertexListWalker<csVector3> (nbuf->Lock (CS_BUF_LOCK_READ),elementCount,nbuf->GetStride ()), 
        tmpColor);
    }
    else
    {
      for (size_t i = 0; i < (csMin(lights.Length (), numLights)); i++)
      {
        light = lights.Get (i);
        iVertexLightCalculator *calc = shaderPlugin->GetLightCalculator (light, useAttenuation);

        switch (lightMixMode)
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
      }
    }
    vbuf->Release ();
    nbuf->Release ();
    clbuf->Release ();
    if (cbuf) cbuf->Release ();
    modes.buffers->SetRenderBuffer (CS_BUFFER_COLOR, clbuf);
  }
}

void csVProcStandardProgram::ResetState ()
{
}

bool csVProcStandardProgram::Compile (csArray<iShaderVariableContext*> &staticContexts)
{
  return true;
}

bool csVProcStandardProgram::Load (iShaderTUResolver* tuResolve, const char* program, 
                                   csArray<csShaderVarMapping>& mappings)
{
  return false;
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
      case XMLTOKEN_LIGHTING:
        {
          numLights = child->GetAttributeValueAsInt ("lights");
          finalLightFactor = child->GetAttributeValueAsFloat ("finalfactor");
          if (child->GetAttributeValue ("attenuation"))
            useAttenuation = child->GetAttributeValueAsBool ("attenuation");

          if (finalLightFactor == 0 && numLights > 0) finalLightFactor = 1.0f;
          const char *str = child->GetAttributeValue ("mixmode");
          if (str)
          {
            if (strcasecmp (str, "none"))
              lightMixMode = LIGHTMIXMODE_NONE;
            else if (strcasecmp (str, "add"))
              lightMixMode = LIGHTMIXMODE_ADD;
            else if (strcasecmp (str, "multiply"))
              lightMixMode = LIGHTMIXMODE_MUL;
          }
          break;
        }
      default:
        {
          switch (commonTokens.Request (value))
          {
          case XMLTOKEN_PROGRAM:
          case XMLTOKEN_VARIABLEMAP:
          case XMLTOKEN_SHADERVAR:
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
