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
  numLights (0), useAttenuation (true), doSpecular (false),
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
  bones_indices_name = strings->Request("bones indices");
  bones_weights_name = strings->Request("bones weights");
  bones_name = strings->Request("bones");
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
                           const iShaderVarStack* Stacks)
{
   csRef<csShaderVariable>_sv;
   _sv = csGetShaderVariableFromStack (Stacks, bones_name);
   if (!_sv.IsValid())
   {
     return false;
   }

   const iArrayReadOnly<csShaderVariable*>* stacks = Stacks;

  BufferName bones_indices_buff_name;
  BufferName bones_weights_buff_name;
  bones_indices_buff_name.userName = bones_indices_name;
  bones_weights_buff_name.userName = bones_weights_name;
  iRenderBuffer *bones_indices_buf = GetBuffer (bones_indices_buff_name, modes, stacks);
  if (!bones_indices_buf)
  {
    return false;
  }

  iRenderBuffer *bones_weights_buf = GetBuffer (bones_weights_buff_name, modes, stacks);
  if (!bones_weights_buf)
  {
    return false;
  }

  iRenderBuffer *vbuf = GetBuffer (positionBuffer, modes, stacks);
  iRenderBuffer *nbuf = GetBuffer (normalBuffer, modes, stacks);
  iRenderBuffer *tbuf = GetBuffer (tangentBuffer, modes, stacks);
  iRenderBuffer *btbuf = GetBuffer (bitangentBuffer, modes, stacks);

  size_t elements_count = vbuf->GetElementCount ();

    csRef<iRenderBuffer> spbuf = modes.buffers->GetRenderBuffer (skinnedPositionOutputBuffer);
  if (!spbuf.IsValid())
  {
      spbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);
  }

    csRef<iRenderBuffer> snbuf;
  if (doNormalSkinning)
  {
    snbuf = modes.buffers->GetRenderBuffer (skinnedNormalOutputBuffer);
    if (!snbuf.IsValid())
    {
      snbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);
    }
  }

  csRef<iRenderBuffer> tnbuf;
  tnbuf = modes.buffers->GetRenderBuffer (skinnedTangentOutputBuffer);
  if (doTangentSkinning)
  {
    if (!tnbuf.IsValid())
    {
      tnbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);
    }
  }

  csRef<iRenderBuffer> btnbuf;
  if (doBiTangentSkinning)
  {
    btnbuf = modes.buffers->GetRenderBuffer (skinnedBiTangentOutputBuffer);
    if (!btnbuf.IsValid())
    {
      btnbuf = csRenderBuffer::CreateRenderBuffer (elements_count, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, 3, true);
    }
  }

  csArray<csReversibleTransform> bone_transforms;
  bone_transforms.SetSize(_sv->GetArraySize());
  for (size_t i = 0; i < _sv->GetArraySize()/2; i++)
  {
    csShaderVariable *bone_sv = _sv->GetArrayElement(i*2);
    csVector4 vec;
    bone_sv->GetValue(vec);
    csMatrix3 bone1_rot = csMatrix3(csQuaternion(vec.x, vec.y, vec.z, vec.w));

    bone_sv = _sv->GetArrayElement(i*2 + 1);
    bone_sv->GetValue(vec);
    csVector3 bone1_offs = csVector3(vec.x, vec.y, vec.z);

    csReversibleTransform bone_transform;
    bone_transform.SetT2O(bone1_rot);
    bone_transform.SetOrigin(bone1_offs);
    bone_transforms[i] = bone_transform;
  }

  csVertexListWalker<int> bones_indices_bufWalker (bones_indices_buf, 4);
  csVertexListWalker<float> bones_weights_bufWalker (bones_weights_buf, 4);
  csVertexListWalker<float> vbufWalker (vbuf, 3);
  csRenderBufferLock<csVector3, iRenderBuffer*> tmpPos (spbuf);
  for (size_t i = 0; i < elements_count; i++)
  {
    const int* c = bones_indices_bufWalker;
    const float* d = bones_weights_bufWalker;
    const float* e = vbufWalker;

    const csReversibleTransform & bone_transform1 = bone_transforms[c[0]];
    const csReversibleTransform & bone_transform2 = bone_transforms[c[1]];
    const csReversibleTransform & bone_transform3 = bone_transforms[c[2]];
    const csReversibleTransform & bone_transform4 = bone_transforms[c[3]];

    tmpPos[i] = d[0] ? (bone_transform1.GetT2O()*csVector3(e[0], e[1], e[2]) + bone_transform1.GetOrigin())*d[0] : csVector3(e[0], e[1], e[2]);
    if (d[1])
    {
      tmpPos[i] += (bone_transform2.GetT2O()*csVector3(e[0], e[1], e[2]) + bone_transform2.GetOrigin())*d[1];
    }
    if (d[2])
    {
      tmpPos[i] += (bone_transform3.GetT2O()*csVector3(e[0], e[1], e[2]) + bone_transform2.GetOrigin())*d[2];
    }
    if (d[3])
    {
      tmpPos[i] += (bone_transform4.GetT2O()*csVector3(e[0], e[1], e[2]) + bone_transform2.GetOrigin())*d[3];
    }

    ++bones_indices_bufWalker;
    ++bones_weights_bufWalker;
    ++vbufWalker;
  }
  modes.buffers->SetAccessor (modes.buffers->GetAccessor(),
  modes.buffers->GetAccessorMask() & ~(1 << skinnedPositionOutputBuffer));
  modes.buffers->SetRenderBuffer (skinnedPositionOutputBuffer, spbuf);

  if (doNormalSkinning)
  {
    bones_indices_bufWalker.ResetState ();
    bones_weights_bufWalker.ResetState ();
    csVertexListWalker<float> nbufWalker (nbuf, 3);
    csRenderBufferLock<csVector3, iRenderBuffer*> tmpNorm (snbuf);
    for (size_t i = 0; i < elements_count; i++)
    {
      const int* c = bones_indices_bufWalker;
      const float* d = bones_weights_bufWalker;
      const float* e = nbufWalker;

      const csReversibleTransform & bone_transform1 = bone_transforms[c[0]];
      const csReversibleTransform & bone_transform2 = bone_transforms[c[1]];
      const csReversibleTransform & bone_transform3 = bone_transforms[c[2]];
      const csReversibleTransform & bone_transform4 = bone_transforms[c[3]];

      tmpNorm[i] = d[0] ? (bone_transform1.GetT2O()*csVector3(e[0], e[1], e[2]))*d[0] : csVector3(e[0], e[1], e[2]);
      if (d[1])
      {
        tmpNorm[i] += (bone_transform2.GetT2O()*csVector3(e[0], e[1], e[2]))*d[1];
      }
      if (d[2])
      {
        tmpNorm[i] += (bone_transform3.GetT2O()*csVector3(e[0], e[1], e[2]))*d[2];
      }
      if (d[3])
      {
        tmpNorm[i] += (bone_transform4.GetT2O()*csVector3(e[0], e[1], e[2]))*d[3];
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

      const csReversibleTransform & bone_transform1 = bone_transforms[c[0]];
      const csReversibleTransform & bone_transform2 = bone_transforms[c[1]];
      const csReversibleTransform & bone_transform3 = bone_transforms[c[2]];
      const csReversibleTransform & bone_transform4 = bone_transforms[c[3]];

      tmpTan[i] = d[0] ? (bone_transform1.GetT2O()*csVector3(e[0], e[1], e[2]))*d[0] : csVector3(e[0], e[1], e[2]);
      if (d[1])
      {
        tmpTan[i] += (bone_transform2.GetT2O()*csVector3(e[0], e[1], e[2]))*d[1];
      }
      if (d[2])
      {
        tmpTan[i] += (bone_transform3.GetT2O()*csVector3(e[0], e[1], e[2]))*d[2];
      }
      if (d[3])
      {
        tmpTan[i] += (bone_transform4.GetT2O()*csVector3(e[0], e[1], e[2]))*d[3];
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

      const csReversibleTransform & bone_transform1 = bone_transforms[c[0]];
      const csReversibleTransform & bone_transform2 = bone_transforms[c[1]];
      const csReversibleTransform & bone_transform3 = bone_transforms[c[2]];
      const csReversibleTransform & bone_transform4 = bone_transforms[c[3]];

      tmpBiTan[i] = d[0] ? (bone_transform1.GetT2O()*csVector3(e[0], e[1], e[2]))*d[0] : csVector3(e[0], e[1], e[2]);
      if (d[1])
      {
        tmpBiTan[i] += (bone_transform2.GetT2O()*csVector3(e[0], e[1], e[2]))*d[1];
      }
      if (d[2])
      {
        tmpBiTan[i] += (bone_transform3.GetT2O()*csVector3(e[0], e[1], e[2]))*d[2];
      }
      if (d[3])
      {
        tmpBiTan[i] += (bone_transform4.GetT2O()*csVector3(e[0], e[1], e[2]))*d[3];
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
                                         const iShaderVarStack* Stacks)
{
  bool skin_verts_updated = false;// @@@ FIXME - time related detection if vertices are not already updated
  if (doVertexSkinning)
  {
    skin_verts_updated = UpdateSkinnedVertices (modes, Stacks);
  }
  if (numLights > 0)
  {
    const iArrayReadOnly<csShaderVariable*>* stacks = Stacks;
    int lightsActive = 0;
    csStringID id;
    id = shaderPlugin->lsvCache.GetDefaultSVId (
      csLightShaderVarCache::varLightCount);
    csShaderVariable* sv;
    if ((stacks->GetSize() > id) && ((sv = stacks->Get (id)) != 0))
      sv->GetValue (lightsActive);

    iRenderBuffer *vbuf = doVertexSkinning && skin_verts_updated ? 
        modes.buffers->GetRenderBuffer (skinnedPositionOutputBuffer):
      GetBuffer (positionBuffer, modes, stacks);

    iRenderBuffer *nbuf = doNormalSkinning && skin_verts_updated ? 
        modes.buffers->GetRenderBuffer (skinnedNormalOutputBuffer):
      GetBuffer (normalBuffer, modes, stacks);

    iRenderBuffer *cbuf = GetBuffer (colorBuffer, modes, stacks);

    if (vbuf == 0 || nbuf == 0) return;
    
    csReversibleTransform camtrans;
    if ((stacks->GetSize() > shaderPlugin->string_world2camera) 
      && ((sv = stacks->Get (shaderPlugin->string_world2camera)) != 0))
      sv->GetValue (camtrans);
    csVector3 eyePos (camtrans.GetT2OTranslation ());
    csVector3 eyePosObject (mesh->object2world.Other2This (eyePos));
    
    bool hasAlpha = cbuf && cbuf->GetComponentCount() >= 4;

    //copy output
    size_t elementCount = vbuf->GetElementCount ();
    csRef<iRenderBuffer> clbuf = 
      csRenderBuffer::CreateRenderBuffer (elementCount, CS_BUF_STREAM,
      CS_BUFCOMP_FLOAT, hasAlpha ? 4 : 3, true);
    csRef<iRenderBuffer> specBuf;
    if (doSpecular)
    {
      specBuf = csRenderBuffer::CreateRenderBuffer (elementCount, 
        CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3, true);
      csRenderBufferLock<float> tmpColor (specBuf);
      memset (tmpColor, 0, sizeof(csColor) * elementCount);
    }
    float shininess = GetParamFloatVal (Stacks, shininessParam, 0.0f);

    // tempdata
    {
      // @@@ FIXME: should probably get rid of the multiple locking/unlocking...
      csRenderBufferLock<float> tmpColor (clbuf);
      memset (tmpColor, 0, sizeof(float) * (hasAlpha?4:3) * elementCount);
    }

    if (lightsActive > 0)
    {
      if (lightMixMode == LIGHTMIXMODE_NONE)
      {
	//only calculate last, other have no effect
	const size_t lightNum = csMin((size_t)lightsActive, numLights)-1;

	if ((disableMask.Length() <= lightNum) 
	  || !disableMask.IsBitSet (lightNum))
	{
	  csLightProperties light (lightNum, shaderPlugin->lsvCache, Stacks);
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
	  if ((disableMask.Length() > i) && disableMask.IsBitSet (i))
	  {
	    useMixMode = lightMixMode;
	    continue;
	  }
	  
	  csLightProperties light (i, shaderPlugin->lsvCache, Stacks);
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

    if (cbuf) 
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
    }

    float finalLightFactorReal = GetParamFloatVal (Stacks, finalLightFactor,
      1.0f);
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
    if (doSpecular)
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

bool csVProcStandardProgram::Compile ()
{
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
    name.userName = strings->Request (str);

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
  const iArrayReadOnly<csShaderVariable*>* stacks)
{
  if (name.defaultName != CS_BUFFER_NONE)
    return modes.buffers->GetRenderBuffer (name.defaultName);

  csShaderVariable* sv;
  if ((stacks->GetSize() > name.userName) && ((sv = stacks->Get (name.userName))))
  {
    iRenderBuffer* buf;
    sv->GetValue (buf);
    return buf;
  }
  return 0;
}

}
CS_PLUGIN_NAMESPACE_END(VProc_std)
