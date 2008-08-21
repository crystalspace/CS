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
#include "csgfx/shadervar.h"
#include "csgfx/vertexlight.h"
#include "csgfx/vertexlistwalker.h"
#include "csgeom/quaternion.h"

#include "imap/services.h"
#include "iutil/document.h"
#include "ivideo/rendermesh.h"

#include "vproc_program.h"

CS_PLUGIN_NAMESPACE_BEGIN(VProc_std)
{

  CS_LEAKGUARD_IMPLEMENT (csVProcStandardProgram);


  csVProcStandardProgram::csVProcStandardProgram (csVProc_Std *plug)
    : scfImplementationType (this, plug->objreg), shaderPlugin (plug), 
    lightMixMode (LIGHTMIXMODE_NONE), 
    colorMixMode (LIGHTMIXMODE_NONE), 
    specularOnDiffuse (false), 
    numLights (0), useAttenuation (true), doDiffuse (true), doSpecular (false),
    doVertexSkinning (false), doNormalSkinning (false), 
    doTangentSkinning (false), doBiTangentSkinning (false),
    specularOutputBuffer (CS_BUFFER_NONE),
    skinnedPositionOutputBuffer (CS_BUFFER_NONE),
    skinnedNormalOutputBuffer (CS_BUFFER_NONE),
    skinnedTangentOutputBuffer (CS_BUFFER_NONE),
    skinnedBiTangentOutputBuffer (CS_BUFFER_NONE),
    positionBuffer (CS_BUFFER_POSITION),
    normalBuffer (CS_BUFFER_NORMAL),
    colorBuffer (CS_BUFFER_COLOR_UNLIT),
    tangentBuffer (CS_BUFFER_TANGENT),
    bitangentBuffer (CS_BUFFER_BINORMAL)
  {
    InitTokenTable (tokens);
    bones_indices_name = stringsSvName->Request("bones indices");
    bones_weights_name = stringsSvName->Request("bones weights");
    bones_name = stringsSvName->Request("bones");
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

  bool csVProcStandardProgram::UpdateSkinnedVertices (csRenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    csRef<csShaderVariable>_sv;
    _sv = csGetShaderVariableFromStack (stack, bones_name);
    if (!_sv.IsValid())
    {
      return false;
    }

    BufferName bones_indices_buff_name;
    BufferName bones_weights_buff_name;
    bones_indices_buff_name.userName = bones_indices_name;
    bones_weights_buff_name.userName = bones_weights_name;
    iRenderBuffer *bones_indices_buf = GetBuffer (bones_indices_buff_name, modes, stack);
    if (!bones_indices_buf)
    {
      return false;
    }

    iRenderBuffer *bones_weights_buf = GetBuffer (bones_weights_buff_name, modes, stack);
    if (!bones_weights_buf)
    {
      return false;
    }

    iRenderBuffer *vbuf = GetBuffer (positionBuffer, modes, stack);
    iRenderBuffer *nbuf = GetBuffer (normalBuffer, modes, stack);
    iRenderBuffer *tbuf = GetBuffer (tangentBuffer, modes, stack);
    iRenderBuffer *btbuf = GetBuffer (bitangentBuffer, modes, stack);

    size_t elements_count = vbuf->GetElementCount ();

    csRef<iRenderBuffer> spbuf = modes.buffers->GetRenderBuffer (skinnedPositionOutputBuffer);
    if (!spbuf.IsValid())
    {
      spbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
        CS_BUFCOMP_FLOAT, 3);
    }

    csRef<iRenderBuffer> snbuf;
    if (doNormalSkinning)
    {
      snbuf = modes.buffers->GetRenderBuffer (skinnedNormalOutputBuffer);
      if (!snbuf.IsValid())
      {
        snbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
          CS_BUFCOMP_FLOAT, 3);
      }
    }

    csRef<iRenderBuffer> tnbuf;
    tnbuf = modes.buffers->GetRenderBuffer (skinnedTangentOutputBuffer);
    if (doTangentSkinning)
    {
      if (!tnbuf.IsValid())
      {
        tnbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
          CS_BUFCOMP_FLOAT, 3);
      }
    }

    csRef<iRenderBuffer> btnbuf;
    if (doBiTangentSkinning)
    {
      btnbuf = modes.buffers->GetRenderBuffer (skinnedBiTangentOutputBuffer);
      if (!btnbuf.IsValid())
      {
        btnbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
          CS_BUFCOMP_FLOAT, 3);
      }
    }

    const size_t bones_count = _sv->GetArraySize ()/2;
    struct csRotPos
    {
      csMatrix3 rot;
      csVector3 pos;
    };
    CS_ALLOC_STACK_ARRAY(csRotPos,rotpos,bones_count);
    for (size_t i = 0; i < bones_count; i++)
    {
      csQuaternion v;
      _sv->GetArrayElement(i*2)->GetValue(v);
      rotpos[i].rot = v.GetMatrix();
      _sv->GetArrayElement(i*2 + 1)->GetValue(rotpos[i].pos);
    }

    csVertexListWalker<int> bones_indices_bufWalker (bones_indices_buf, 4);
    csVertexListWalker<float> bones_weights_bufWalker (bones_weights_buf, 4);
    if (doVertexSkinning)
    {
      csVertexListWalker<float> vbufWalker (vbuf, 3);
      csRenderBufferLock<csVector3, iRenderBuffer*> tmpPos (spbuf);
      for (size_t i = 0; i < elements_count; i++)
      {
        const int* c = bones_indices_bufWalker;
        const float* d = bones_weights_bufWalker;
        const float* e = vbufWalker;

        csVector3 origin_pos = csVector3(e[0], e[1], e[2]);
        bool skinned = false;
        for (int k = 0; k < 4; k++)
        {
          if (d[k])
          {
            csRotPos& rp = rotpos[c[k]];
            if (!skinned)
            {
              tmpPos[i] = (rp.rot*origin_pos + rp.pos)*d[k];
              skinned =true;
            }
            else
            {
              tmpPos[i] += (rp.rot*origin_pos + rp.pos)*d[k];
            }
          }
        }
        if (!skinned)
        {
          tmpPos[i] = origin_pos;
        }

        ++bones_indices_bufWalker;
        ++bones_weights_bufWalker;
        ++vbufWalker;
      }
      modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
        modes.buffers->GetAccessorMask() & ~(1 << skinnedPositionOutputBuffer));
      modes.buffers->SetRenderBuffer (skinnedPositionOutputBuffer, spbuf);
    }

    if (doNormalSkinning)
    {
      bones_indices_bufWalker.ResetState();
      bones_weights_bufWalker.ResetState();
      csVertexListWalker<float> nbufWalker (nbuf, 3);
      csRenderBufferLock<csVector3, iRenderBuffer*> tmpNorm (snbuf);
      for (size_t i = 0; i < elements_count; i++)
      {
        const int* c = bones_indices_bufWalker;
        const float* d = bones_weights_bufWalker;
        const float* e = nbufWalker;

        csVector3 origin_norm = csVector3(e[0], e[1], e[2]);
        bool skinned = false;
        for (int k = 0; k < 4; k++)
        {
          if (d[k])
          {
            csRotPos& rp = rotpos[c[k]];
            if (!skinned)
            {
              tmpNorm[i] = (rp.rot*origin_norm)*d[k];
              skinned = true;
            }
            else
            {
              tmpNorm[i] += (rp.rot*origin_norm)*d[k];
            }
          }
        }
        if (!skinned)
        {
          tmpNorm[i] = origin_norm;
        }

        ++bones_indices_bufWalker;
        ++bones_weights_bufWalker;
        ++nbufWalker;
      }
      modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
        modes.buffers->GetAccessorMask() & ~(1 << skinnedNormalOutputBuffer));
      modes.buffers->SetRenderBuffer (skinnedNormalOutputBuffer, snbuf);
    }

    if (doTangentSkinning)
    {
      bones_indices_bufWalker.ResetState();
      bones_weights_bufWalker.ResetState();
      csVertexListWalker<float> tbufWalker (tbuf, 3);
      csRenderBufferLock<csVector3, iRenderBuffer*> tmpTan (tnbuf);
      for (size_t i = 0; i < elements_count; i++)
      {
        const int* c = bones_indices_bufWalker;
        const float* d = bones_weights_bufWalker;
        const float* e = tbufWalker;

        csVector3 origin_tang = csVector3(e[0], e[1], e[2]);
        bool skinned = false;
        for (int k = 0; k < 4; k++)
        {
          if (d[k])
          {
            csRotPos& rp = rotpos[c[k]];
            if (!skinned)
            {
              tmpTan[i] = (rp.rot*origin_tang)*d[k];
              skinned = true;
            }
            else
            {
              tmpTan[i] += (rp.rot*origin_tang)*d[k];
            }
          }
        }
        if (!skinned)
        {
          tmpTan[i] = origin_tang;
        }

        ++bones_indices_bufWalker;
        ++bones_weights_bufWalker;
        ++tbufWalker;
      }
      modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
        modes.buffers->GetAccessorMask() & ~(1 << skinnedTangentOutputBuffer));
      modes.buffers->SetRenderBuffer (skinnedTangentOutputBuffer, tnbuf);
    }

    if (doBiTangentSkinning)
    {
      bones_indices_bufWalker.ResetState();
      bones_weights_bufWalker.ResetState();
      csVertexListWalker<float> btbufWalker (btbuf, 3);
      csRenderBufferLock<csVector3, iRenderBuffer*> tmpBiTan (btnbuf);
      for (size_t i = 0; i < elements_count; i++)
      {
        const int* c = bones_indices_bufWalker;
        const float* d = bones_weights_bufWalker;
        const float* e = btbufWalker;

        csVector3 origin_bitang = csVector3(e[0], e[1], e[2]);
        bool skinned = false;
        for (int k = 0; k < 4; k++)
        {
          if (d[k])
          {
            csRotPos& rp = rotpos[c[k]];
            if (!skinned)
            {
              tmpBiTan[i] = (rp.rot*origin_bitang)*d[k];
              skinned = true;
            }
            else
            {
              tmpBiTan[i] += (rp.rot*origin_bitang)*d[k];
            }
          }
        }
        if (!skinned)
        {
          tmpBiTan[i] = origin_bitang;
        }

        ++bones_indices_bufWalker;
        ++bones_weights_bufWalker;
        ++btbufWalker;
      }
      modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
        modes.buffers->GetAccessorMask() & ~(1 << skinnedBiTangentOutputBuffer));
      modes.buffers->SetRenderBuffer (skinnedBiTangentOutputBuffer, btnbuf);
    }

    return true;
  }

  void csVProcStandardProgram::SetupState (const csRenderMesh* mesh,
    csRenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    bool skin_updated = false;// @@@ FIXME - time related detection if vertices are not already updated
    if (doVertexSkinning || doNormalSkinning || 
      doTangentSkinning || doBiTangentSkinning)
    {
      skin_updated = UpdateSkinnedVertices (modes, stack);
    }
    if (numLights > 0)
    {
      int lightsActive = 0;
      CS::ShaderVarStringID id;
      id = shaderPlugin->lsvCache.GetDefaultSVId (
        csLightShaderVarCache::varLightCount);
      csShaderVariable* sv;
      if ((stack.GetSize () > id) && ((sv = stack[id]) != 0))
        sv->GetValue (lightsActive);

      iRenderBuffer *vbuf = doVertexSkinning && skin_updated ? 
        modes.buffers->GetRenderBuffer (skinnedPositionOutputBuffer) :
        GetBuffer (positionBuffer, modes, stack);

      iRenderBuffer *nbuf = doNormalSkinning && skin_updated ? 
        modes.buffers->GetRenderBuffer (skinnedNormalOutputBuffer) :
        GetBuffer (normalBuffer, modes, stack);

      iRenderBuffer *cbuf = GetBuffer (colorBuffer, modes, stack);

      if (vbuf == 0 || nbuf == 0) return;

      csReversibleTransform camtrans;
      if ((stack.GetSize () > shaderPlugin->string_world2camera) 
        && ((sv = stack[shaderPlugin->string_world2camera]) != 0))
        sv->GetValue (camtrans);
      csVector3 eyePos (camtrans.GetT2OTranslation ());
      csVector3 eyePosObject (mesh->object2world.Other2This (eyePos));

      bool hasAlpha = cbuf && cbuf->GetComponentCount() >= 4;

      //copy output
      size_t elementCount = vbuf->GetElementCount ();
      csRef<iRenderBuffer> clbuf;
      if (doDiffuse)
      {
        clbuf = csRenderBuffer::CreateRenderBuffer (elementCount, CS_BUF_STREAM,
          CS_BUFCOMP_FLOAT, hasAlpha ? 4 : 3);
        // @@@ FIXME: should probably get rid of the multiple locking/unlocking...
        csRenderBufferLock<float> tmpColor (clbuf);
        memset (tmpColor, 0, sizeof(float) * (hasAlpha?4:3) * elementCount);
      }
      csRef<iRenderBuffer> specBuf;
      if (doSpecular)
      {
        specBuf = csRenderBuffer::CreateRenderBuffer (elementCount, 
          CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
        csRenderBufferLock<float> tmpColor (specBuf);
        memset (tmpColor, 0, sizeof(csColor) * elementCount);
      }
      float shininess = GetParamFloatVal (stack, shininessParam, 0.0f);

      if (lightsActive > 0)
      {
        if (lightMixMode == LIGHTMIXMODE_NONE)
        {
          //only calculate last, other have no effect
          const size_t lightNum = csMin((size_t)lightsActive, numLights)-1;

          if ((disableMask.GetSize() <= lightNum) 
            || !disableMask.IsBitSet (lightNum))
          {
            csLightProperties light (lightNum, shaderPlugin->lsvCache, stack,
              mesh->object2world);
            iVertexLightCalculator *calc = 
              shaderPlugin->GetLightCalculator (light, useAttenuation);
            calc->CalculateLighting (light, eyePosObject, shininess, elementCount, 
              vbuf, nbuf, clbuf, specBuf);
          }
        }
        else
        {
          LightMixmode useMixMode = LIGHTMIXMODE_ADD;
          for (size_t i = 0; i < (csMin((size_t)lightsActive, numLights)); i++)
          {
            if ((disableMask.GetSize() > i) && disableMask.IsBitSet (i))
            {
              useMixMode = lightMixMode;
              continue;
            }

            csLightProperties light (i, shaderPlugin->lsvCache, stack,
              mesh->object2world);
            iVertexLightCalculator *calc = 
              shaderPlugin->GetLightCalculator (light, useAttenuation);

            switch (useMixMode)
            {
            case LIGHTMIXMODE_ADD:
              {
                calc->CalculateLightingAdd (light, eyePosObject, shininess, elementCount, 
                  vbuf, nbuf, clbuf, specBuf);
                break;
              }
            case LIGHTMIXMODE_MUL:
              {
                calc->CalculateLightingMul (light, eyePosObject, shininess, elementCount,
                  vbuf, nbuf, clbuf, specBuf);
                break;
              }
            case LIGHTMIXMODE_NONE:
              break;
            }
            useMixMode = lightMixMode;
          }
        }

      }

      if (doDiffuse && cbuf)
      {
        switch (colorMixMode)
        {
        case LIGHTMIXMODE_NONE:
          if (!hasAlpha) break;
          {
            csVertexListWalker<float> cbufWalker (cbuf, 4);
            csRenderBufferLock<csVector4, iRenderBuffer*> tmpColor (clbuf);
            for (size_t i = 0; i < elementCount; i++)
            {
              const float* c = cbufWalker;
              tmpColor[i].w = c[3];
              ++cbufWalker;
            }
          }
          break;
        case LIGHTMIXMODE_ADD:
          {
            csVertexListWalker<float> cbufWalker (cbuf, 4);
            csRenderBufferLock<csVector4, iRenderBuffer*> tmpColor (clbuf);
            for (size_t i = 0; i < elementCount; i++)
            {
              csVector4& t = tmpColor[i];
              const float* c = cbufWalker;
              for (int j = 0; j < 3; j++)
                t[j] += c[j];
              if (hasAlpha) t[3] = c[3];
              ++cbufWalker;
            }
          }
          break;
        case LIGHTMIXMODE_MUL:
          {
            csVertexListWalker<float> cbufWalker (cbuf, 4);
            csRenderBufferLock<csVector4, iRenderBuffer*> tmpColor (clbuf);
            for (size_t i = 0; i < elementCount; i++)
            {
              csVector4& t = tmpColor[i];
              const float* c = cbufWalker;
              for (int j = 0; j < 3; j++)
                t[j] *= c[j];
              if (hasAlpha) t[3] = c[3];
              ++cbufWalker;
            }
          }
          break;
        default:
          CS_ASSERT (false);
        }
        if (doSpecular && specularOnDiffuse)
        {
	  csRenderBufferLock<csColor> tmpColor (clbuf);
	  csRenderBufferLock<csColor> tmpColor2 (specBuf);
	  for (size_t i = 0; i < elementCount; i++)
	  {
	    tmpColor[i] += tmpColor2[i];
	  }
        }
      }

      float finalLightFactorReal = GetParamFloatVal (stack, finalLightFactor,
	1.0f);
      if (doDiffuse)
      {
	{
	  csRenderBufferLock<csColor> tmpColor (clbuf);
	  for (size_t i = 0; i < elementCount; i++)
	  {
	    tmpColor[i] *= finalLightFactorReal;
	  }
	}
  
	modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
	  modes.buffers->GetAccessorMask() & ~CS_BUFFER_COLOR_MASK);
	modes.buffers->SetRenderBuffer (CS_BUFFER_COLOR, clbuf);
      }
      if (doSpecular && !specularOnDiffuse)
      {
        csRenderBufferLock<csColor> tmpColor (specBuf);
        for (size_t i = 0; i < elementCount; i++)
        {
          tmpColor[i] *= finalLightFactorReal;
        }

        modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
          modes.buffers->GetAccessorMask() & ~(1 << specularOutputBuffer));
        modes.buffers->SetRenderBuffer (specularOutputBuffer, specBuf);
      }
    }
  }

  void csVProcStandardProgram::ResetState ()
  {
  }

  bool csVProcStandardProgram::Compile (iHierarchicalCache*, csRef<iString>* tag)
  {
    tag->AttachNew (new scfString ("default"));
  
    return true;
  }

  bool csVProcStandardProgram::Load (iShaderDestinationResolver*, 
    const char*, 
    csArray<csShaderVarMapping>&)
  {
    return false;
  }

  bool csVProcStandardProgram::ParseLightMixMode (iDocumentNode* child, 
    LightMixmode& mixmode)
  {
    const char *str = child->GetContentsValue ();
    if (str)
    {
      if (strcasecmp (str, "none") == 0)
        mixmode = LIGHTMIXMODE_NONE;
      else if (strcasecmp (str, "add") == 0)
        mixmode = LIGHTMIXMODE_ADD;
      else if (strcasecmp (str, "multiply") == 0)
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
      name.userName = stringsSvName->Request (str);

    return true;
  }

  bool csVProcStandardProgram::Load (iShaderDestinationResolver* /*resolve*/, 
    iDocumentNode* program)
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
        case XMLTOKEN_ENABLELIGHT:
          {
            size_t n = (size_t)child->GetAttributeValueAsInt ("num");
            bool b;
            if (!synsrv->ParseBool (child, b, true))
              return false;
            if (!b)
            {
              if (disableMask.GetSize() <= n)
                disableMask.SetSize (n+1);
              disableMask.SetBit (n);
            }
            break;
          }
        case XMLTOKEN_DIFFUSE:
          {
            if (!synsrv->ParseBool (child, doDiffuse, true))
              return false;
          }
          break;
        case XMLTOKEN_SPECULAR:
          {
            if (!synsrv->ParseBool (child, doSpecular, true))
              return false;
            const char* buffer = child->GetAttributeValue ("buffer");
            if (!buffer)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "'buffer' attribute missing");
              return false;
            }
            csRenderBufferName bufferName = csRenderBuffer::GetBufferNameFromDescr (buffer);
            if (bufferName == CS_BUFFER_NONE)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "buffer name '%s' invalid here", buffer);
              return false;
            }
            specularOutputBuffer = bufferName;
          }
          break;
        case XMLTOKEN_SPECULAREXP:
          if (!ParseProgramParam (child, shininessParam,
            ParamFloat | ParamShaderExp))
            return false;
          break;
        case XMLTOKEN_SPECULARONDIFFUSE:
          {
            if (!synsrv->ParseBool (child, specularOnDiffuse, true))
              return false;
          }
          break;
        case XMLTOKEN_SKINNED_POSITION:
          {
            if (!synsrv->ParseBool (child, doVertexSkinning, true))
              return false;
            const char* buffer = child->GetAttributeValue ("buffer");
            if (!buffer)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "'buffer' attribute missing");
              return false;
            }
            csRenderBufferName bufferName = csRenderBuffer::GetBufferNameFromDescr (buffer);
            if (bufferName == CS_BUFFER_NONE)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "buffer name '%s' invalid here", buffer);
              return false;
            }
            skinnedPositionOutputBuffer = bufferName;
          }
          break;
        case XMLTOKEN_SKINNED_NORMAL:
          {
            if (!synsrv->ParseBool (child, doNormalSkinning, true))
              return false;
            const char* buffer = child->GetAttributeValue ("buffer");
            if (!buffer)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "'buffer' attribute missing");
              return false;
            }
            csRenderBufferName bufferName = csRenderBuffer::GetBufferNameFromDescr (buffer);
            if (bufferName == CS_BUFFER_NONE)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "buffer name '%s' invalid here", buffer);
              return false;
            }
            skinnedNormalOutputBuffer = bufferName;
          }
          break;
        case XMLTOKEN_SKINNED_TANGENT:
          {
            if (!synsrv->ParseBool (child, doTangentSkinning, true))
              return false;
            const char* buffer = child->GetAttributeValue ("buffer");
            if (!buffer)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "'buffer' attribute missing");
              return false;
            }
            csRenderBufferName bufferName = csRenderBuffer::GetBufferNameFromDescr (buffer);
            if (bufferName == CS_BUFFER_NONE)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "buffer name '%s' invalid here", buffer);
              return false;
            }
            skinnedTangentOutputBuffer = bufferName;
          }
          break;
        case XMLTOKEN_SKINNED_BITANGENT:
          {
            if (!synsrv->ParseBool (child, doBiTangentSkinning, true))
              return false;
            const char* buffer = child->GetAttributeValue ("buffer");
            if (!buffer)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "'buffer' attribute missing");
              return false;
            }
            csRenderBufferName bufferName = csRenderBuffer::GetBufferNameFromDescr (buffer);
            if (bufferName == CS_BUFFER_NONE)
            {
              synsrv->ReportError ("crystalspace.graphics3d.shader.vproc_std",
                pnode, "buffer name '%s' invalid here", buffer);
              return false;
            }
            skinnedBiTangentOutputBuffer = bufferName;
          }
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
    csRenderMeshModes& modes, 
    const csShaderVariableStack& stack)
  {
    if (name.defaultName != CS_BUFFER_NONE)
      return modes.buffers->GetRenderBuffer (name.defaultName);

    csShaderVariable* sv;
    if ((stack.GetSize () > name.userName) && ((sv = stack[name.userName])))
    {
      iRenderBuffer* buf;
      sv->GetValue (buf);
      return buf;
    }
    return 0;
  }

  void csVProcStandardProgram::TryAddUsedShaderVarBufferName (const BufferName& name,
    csBitArray& bits) const
  {
    if (name.defaultName == CS_BUFFER_NONE)
    {
      TryAddUsedShaderVarName (name.userName, bits);
    }
  }

  void csVProcStandardProgram::GetUsedShaderVars (csBitArray& bits) const
  {
    TryAddUsedShaderVarProgramParam (finalLightFactor, bits);
    TryAddUsedShaderVarProgramParam (shininessParam, bits);

    TryAddUsedShaderVarName (bones_indices_name, bits);
    TryAddUsedShaderVarName (bones_weights_name, bits);
    TryAddUsedShaderVarName (bones_name, bits);

    TryAddUsedShaderVarBufferName (positionBuffer, bits);
    TryAddUsedShaderVarBufferName (normalBuffer, bits);
    TryAddUsedShaderVarBufferName (colorBuffer, bits);
    TryAddUsedShaderVarBufferName (tangentBuffer, bits);
    TryAddUsedShaderVarBufferName (bitangentBuffer, bits);
  }

}
CS_PLUGIN_NAMESPACE_END(VProc_std)
