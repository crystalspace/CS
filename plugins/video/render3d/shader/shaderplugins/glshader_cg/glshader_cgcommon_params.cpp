/*
  Copyright (C) 2002-2005 by Marten Svanfeldt
			     Anders Stenberg
			     Frank Richter

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

#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glhelper.h"

#include "glshader_cg.h"
#include "glshader_cgcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

template<typename Setter>
void csShaderGLCGCommon::SetParameterValue (const Setter& setter,
                                            ShaderParameter* sparam,
                                            csShaderVariable* var)
{
  if (sparam == 0) return;
    
  CGparameter param = sparam->param;
  CGtype paramType = sparam->paramType;
  uint slot = sparam->baseSlot;
  
  switch (paramType)
  {
    case CG_INT:
      {
	int i;
	if (var->GetValue (i))
	  setter.Parameter1i (slot, param, i);
      }
      break;
    case CG_FLOAT:
      {
	float f;
	if (var->GetValue (f))
	  setter.Parameter1f (slot, param, f);
      }
      break;
    case CG_FLOAT2:
      {
	csVector2 v;
	if (var->GetValue (v))
	  setter.Parameter2fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT3:
      {
	csVector3 v;
	if (var->GetValue (v))
	  setter.Parameter3fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT4:
      {
	csVector4 v;
	if (var->GetValue (v))
	  setter.Parameter4fv (slot, param, &v.x);
      }
      break;
    case CG_FLOAT3x3:
      {
	float matrix[9];
	SVtoCgMatrix3x3 (var, matrix);
        setter.MatrixParameter3x3 (slot, param, matrix);
      }
      break;
    case CG_FLOAT4x4:
      {
	float matrix[16];
	SVtoCgMatrix4x4 (var, matrix);
	setter.MatrixParameter4x4 (slot, param, matrix);
      }
      break;
    case CG_ARRAY:
      {
	if (var->GetArraySize () == 0) 
	  break;
	  
	size_t numElements = csMin (sparam->arrayItems.GetSize(),
				    var->GetArraySize ());
	if (numElements == 0) 
	  break;
	  
	for (size_t idx = 0; idx < numElements; idx++)
	{
	  csShaderVariable *element =
	    var->GetArrayElement (idx);
	  if (element != 0)
	    SetParameterValue (setter, sparam->arrayItems[idx], element);
	}
      }
      break;
    default:
      CS_ASSERT_MSG("Don't support CG param type (yet)", false);
  }
}

void csShaderGLCGCommon::SVtoCgMatrix3x3  (csShaderVariable* var, float* matrix)
{
  if (var->GetType () == csShaderVariable::MATRIX)
  {
    csMatrix3 m;
    if (var->GetValue (m))
    {
      CS::PluginCommon::MakeGLMatrix3x3 (m, matrix, true);
    }
  }
  else if (var->GetType () == csShaderVariable::TRANSFORM)
  {
    csReversibleTransform t;
    if (var->GetValue (t))
    {
      CS::PluginCommon::MakeGLMatrix3x3 (t.GetO2T(), matrix, true);
    }
  }
  else if (var->GetType () == csShaderVariable::ARRAY)
  {
    if (var->GetArraySize () != 3)
      return;

    csVector3 v;
    for (uint idx = 0; idx < var->GetArraySize (); idx++)
    {
      csShaderVariable *element =
	var->GetArrayElement (idx);
      if (element != 0 && element->GetValue (v))
      {
	matrix[idx*3+0] = v[0]; 
	matrix[idx*3+1] = v[1];
	matrix[idx*3+2] = v[2];
      }
    }
  }
  else
  {
    CS_ASSERT_MSG("Can't convert all SV contents to FLOAT3x3 (yet)", false);
    memset (matrix, 0, 9 * sizeof (float));
  }
}

void csShaderGLCGCommon::SVtoCgMatrix4x4  (csShaderVariable* var, float* matrix)
{
  if (var->GetType () == csShaderVariable::MATRIX3X3)
  {
    csMatrix3 m;
    if (var->GetValue (m))
    {
      makeGLMatrix (m, matrix, true);
    }
  }
  else if (var->GetType () == csShaderVariable::TRANSFORM)
  {
    csReversibleTransform t;
    if (var->GetValue (t))
    {
      makeGLMatrix (t, matrix, true);
    }
  }
  else if (var->GetType () == csShaderVariable::ARRAY)
  {
    if (var->GetArraySize () != 4)
      return;

    csVector4 v;
    for (uint idx = 0; idx < var->GetArraySize (); idx++)
    {
      csShaderVariable *element =
	var->GetArrayElement (idx);
      if (element != 0 && element->GetValue (v))
      {
	matrix[idx*4+0] = v[0]; 
	matrix[idx*4+1] = v[1];
	matrix[idx*4+2] = v[2];
	matrix[idx*4+3] = v[3];
      }
    }
  }
  else if (var->GetType () == csShaderVariable::MATRIX4X4)
  {
    CS::Math::Matrix4 m;
    if (var->GetValue (m))
    {
      CS::PluginCommon::MakeGLMatrix4x4 (m, matrix, true);
    }
  }
  else
  {
    CS_ASSERT_MSG("Can't convert all SV contents to FLOAT4x4 (yet)", false);
    memset (matrix, 0, 16 * sizeof (float));
  }
}

template<typename Array, typename ParamSetter>
void csShaderGLCGCommon::ApplyVariableMapArray (const Array& array,
                                                const ParamSetter& setter,
                                                const csShaderVariableStack& stack)
{
  csRef<csShaderVariable> var;
  
  for(size_t i = 0; i < array.GetSize (); ++i)
  {
    const VariableMapEntry& mapping = array[i];
    
    switch (mapping.mappingParam.name)
    {
      case svClipPackedDist0:
        var = clipPackedDists[0];
        break;
      case svClipPackedDist1:
        var = clipPackedDists[1];
        break;
      case svClipPlane-0:
      case svClipPlane-1:
      case svClipPlane-2:
      case svClipPlane-3:
      case svClipPlane-4:
      case svClipPlane-5:
        var = clipPlane[svClipPlane-mapping.mappingParam.name];
        break;
      default:
        var = GetParamSV (stack, mapping.mappingParam);
    }
    // If var is null now we have no const nor any passed value, ignore it
    if (!var.IsValid ())
      continue;

    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    SetParameterValue (setter, param, var);
  }
}

struct SetterCg
{
  void Parameter1i (uint slot, CGparameter param, int v) const
  { cgSetParameter1i (param, v); }
  void Parameter1f (uint slot, CGparameter param, float v) const
  { cgSetParameter1f (param, v); }
  void Parameter2fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter2fv (param, v); }
  void Parameter3fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter3fv (param, v); }
  void Parameter4fv (uint slot, CGparameter param, float* v) const
  { cgSetParameter4fv (param, v); }
  void MatrixParameter3x3 (uint slot, CGparameter param, float* v) const
  { cgGLSetMatrixParameterfr (param, v); }
  void MatrixParameter4x4 (uint slot, CGparameter param, float* v) const
  { cgGLSetMatrixParameterfr (param, v); }
};

template<GLenum Target, bool GP4Prog>
struct SetterARB : public SetterCg
{
  csGLExtensionManager* ext;

  SetterARB (csGLExtensionManager* ext) : ext (ext) {}

  void Parameter1i (uint slot, CGparameter param, int v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter1i (slot, param, v);
    else if (GP4Prog)
      ext->glProgramLocalParameterI4iNV (Target, slot, v, 0, 0, 0);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v, 0, 0, 0);
  }
  void Parameter1f (uint slot, CGparameter param, float v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter1f (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v, 0, 0, 0);
  }
  void Parameter2fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter2fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v[0], v[1], 0, 0);
  }
  void Parameter3fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter3fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fARB (Target, slot, v[0], v[1], v[2], 0);
  }
  void Parameter4fv (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::Parameter4fv (slot, param, v);
    else
      ext->glProgramLocalParameter4fvARB (Target, slot, v);
  }
  void MatrixParameter3x3 (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::MatrixParameter4x4 (slot, param, v);
    else
    {
      float m4x4[16];
      m4x4[0] = v[0]; m4x4[1] = v[1]; m4x4[2] = v[2]; m4x4[3] = 0;
      m4x4[4] = v[3]; m4x4[5] = v[4]; m4x4[6] = v[5]; m4x4[7] = 0;
      m4x4[8] = v[6]; m4x4[9] = v[7]; m4x4[10] = v[8]; m4x4[11] = 0;
      m4x4[12] = 0; m4x4[13] = 0; m4x4[14] = 0; m4x4[15] = 1;
    
      if (ext->CS_GL_EXT_gpu_program_parameters)
        ext->glProgramLocalParameters4fvEXT (Target, slot, 4, m4x4);
      else
      {
	for (int i = 0; i < 4; i++)
	  ext->glProgramLocalParameter4fvARB (Target, slot+i, m4x4+i*4);
      }
    }
  }
  void MatrixParameter4x4 (uint slot, CGparameter param, float* v) const
  {
    if (slot == (uint)~0)
      SetterCg::MatrixParameter4x4 (slot, param, v);
    else
    {
      if (ext->CS_GL_EXT_gpu_program_parameters)
        ext->glProgramLocalParameters4fvEXT (Target, slot, 4, v);
      else
      {
	for (int i = 0; i < 4; i++)
	  ext->glProgramLocalParameter4fvARB (Target, slot+i, v+i*4);
      }
    }
  }
};

void csShaderGLCGCommon::ApplyVariableMapArrays (const csShaderVariableStack& stack)
{
  // set variables
  if ((programType == progVP) && (programProfile >= CG_PROFILE_ARBVP1))
  {
    if (programProfile >= CG_PROFILE_GPU_VP)
    {
      SetterARB<GL_VERTEX_PROGRAM_ARB, true> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
    else
    {
      SetterARB<GL_VERTEX_PROGRAM_ARB, false> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
  }
  else if ((programType == progFP) &&
    ((programProfile >= CG_PROFILE_ARBFP1)
    || (programProfile == CG_PROFILE_FP40)))
  {
    if (programProfile >= CG_PROFILE_GPU_FP)
    {
      SetterARB<GL_FRAGMENT_PROGRAM_ARB, true> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
    else
    {
      SetterARB<GL_FRAGMENT_PROGRAM_ARB, false> setter (shaderPlug->ext);
      ApplyVariableMapArray (variablemap, setter, stack);
    }
  }
  else
  {
    SetterCg setter;
    ApplyVariableMapArray (variablemap, setter, stack);
  }
  
}

void csShaderGLCGCommon::SetParameterValueCg (ShaderParameter* sparam,
                                              csShaderVariable* var)
{
  SetterCg setter;
  SetParameterValue (setter, sparam, var);
}

void csShaderGLCGCommon::FreeShaderParam (ShaderParameter* sparam)
{
  if (sparam == 0) return;
  for (size_t i = 0; i < sparam->arrayItems.GetSize(); i++)
  {
    FreeShaderParam (sparam->arrayItems[i]);
  }
  shaderPlug->paramAlloc.Free (sparam);
}

void csShaderGLCGCommon::FillShaderParam (ShaderParameter* sparam, 
                                          CGparameter param)
{
  sparam->param = param;
  sparam->paramType = cgGetParameterType (param);
  if (sparam->paramType == CG_ARRAY)
  {
    size_t arraySize = cgGetArraySize (param, 0);
    for (size_t i = arraySize; i-- > 0; )
    {
      CGparameter element = cgGetArrayParameter (param, int (i));
      ShaderParameter* newsparam = shaderPlug->paramAlloc.Alloc();
      FillShaderParam (newsparam, element);
      sparam->arrayItems.Put (i, newsparam);
    }
  }
}

void csShaderGLCGCommon::GetShaderParamSlot (ShaderParameter* sparam)
{
  if (cgGetParameterType (sparam->param) != CG_ARRAY)
  {
    CGresource rsc = cgGetParameterResource (sparam->param);
    if ((rsc == CG_C) || (rsc == CG_GLSL_UNIFORM))
    {
      sparam->baseSlot = cgGetParameterResourceIndex (sparam->param);
    }
  }
  for (size_t i = 0; i < sparam->arrayItems.GetSize(); i++)
  {
    if (sparam->arrayItems[i] != 0)
      GetShaderParamSlot (sparam->arrayItems[i]);
  }
}

void csShaderGLCGCommon::GetParamsFromVmap()
{
  size_t i = 0;
  while (i < variablemap.GetSize ())
  {
    // Get the Cg parameter
    CGparameter param = cgGetNamedParameter (program, 
      variablemap[i].destination);
    ShaderParameter* sparam =
      reinterpret_cast<ShaderParameter*> (variablemap[i].userVal);

    if (!param /*||
	(cgGetParameterType (param) != CG_ARRAY && !cgIsParameterReferenced (param))*/)
    {
      variablemap.DeleteIndex (i);
      FreeShaderParam (sparam);
      continue;
    }
    FillShaderParam (sparam, param);
    bool assumeConst = sparam->assumeConstant;
    // Mark constants as to be folded in
    if (assumeConst || variablemap[i].mappingParam.IsConstant())
    {
      csShaderVariable* var = variablemap[i].mappingParam.var;
      if (var != 0)
	SetParameterValueCg (sparam, var);
      cgSetParameterVariability (param, CG_LITERAL);
      variablemap.DeleteIndex (i);
      FreeShaderParam (sparam);
      continue;
    }
    i++;
  }

  variablemap.ShrinkBestFit();
}

void csShaderGLCGCommon::GetPostCompileParamProps ()
{
  for(size_t i = 0; i < variablemap.GetSize (); )
  {
    VariableMapEntry& mapping = variablemap[i];
    ShaderParameter* sparam =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    
    if (!GetPostCompileParamProps (sparam))
    {
      variablemap.DeleteIndex (i);
      FreeShaderParam (sparam);
      continue;
    }
    
    GetShaderParamSlot (sparam);
    ++i;
  }
}

bool csShaderGLCGCommon::GetPostCompileParamProps (ShaderParameter* sparam)
{
  CGparameter param = sparam->param;
  if (sparam->paramType == CG_ARRAY)
  {
    bool ret = false;
    for (size_t i = sparam->arrayItems.GetSize(); i-- > 0; )
    {
      if (!GetPostCompileParamProps (sparam->arrayItems[i]))
      {
        if (i == sparam->arrayItems.GetSize()-1)
          sparam->arrayItems.Truncate (i);
        else
        {
          FreeShaderParam (sparam->arrayItems[i]);
          sparam->arrayItems[i] = 0;
        }
      }
      else
      {
        ret |= true;
      }
    }
    if (ret) sparam->arrayItems.ShrinkBestFit();
    return ret;
  }
  else
    return cgIsParameterReferenced (param) != 0;
}

void csShaderGLCGCommon::CollectUnusedParameters (csSet<csString>& unusedParams)
{
  CGparameter cgParam = cgGetFirstLeafParameter (program, CG_PROGRAM);
  while (cgParam)
  {
    if (!cgIsParameterUsed (cgParam, program))
    {
      csString param (cgGetParameterName (cgParam));
      for (size_t j = 0; j < param.Length(); j++)
      {
        if ((param[j] == '.') || (param[j] == '[') || (param[j] == ']'))
          param[j] = '_';
      }
      csString s;
      s.Format ("PARAM_%s_UNUSED", param.GetData());
      unusedParams.Add (s);
    }

    cgParam = cgGetNextLeafParameter (cgParam);
  }
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
