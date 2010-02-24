/*
  Copyright (C) 2004-2006 by Frank Richter
	    (C) 2004-2006 by Jorrit Tyberghein

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

#include "condeval.h"
#include "condeval_eval_sv.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  csConditionEvaluator::EvaluatorShadervar::BoolType 
  csConditionEvaluator::EvaluatorShadervar::Boolean (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandOperation:
	return evaluator.EvaluateCachedInternal (evalState, *this, operand.operation);
      case operandBoolean:
	return operand.boolVal;
      case operandSV:
	{
	  return GetShaderVar (operand) != 0;
	}
	break;
      case operandSVValueTexture:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    iTextureHandle* th;
	    if (sv->GetValue (th))
	      return th != 0;
	  }
	}
	break;
      case operandSVValueBuffer:
	{
	  iRenderBuffer* rb = 0;
	  if (operand.svLocation.bufferName != CS_BUFFER_NONE)
	  {
	    if (modes && modes->buffers.IsValid())
	      rb = modes->buffers->GetRenderBuffer (
		operand.svLocation.bufferName);
	  }
	  else
	  {
	    csShaderVariable* sv = GetShaderVar (operand);
	    if (sv != 0)
	    {
	      sv->GetValue (rb);
	    }
	  }
	  return rb != 0;
	}
	break;
      default:
	;
    }
    return false;
  }

  csConditionEvaluator::EvaluatorShadervar::IntType 
  csConditionEvaluator::EvaluatorShadervar::Int (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandInt:
	return operand.intVal;
      case operandFloat:
	return (int)operand.floatVal;
      case operandSVValueFloat:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    float v;
	    if (sv->GetValue (v))
	      return (int)v;
	  }
	}
	break;
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    csVector4 v;
	    if (sv->GetValue (v))
	    {
	      int c = operand.type - operandSVValueX;
	      return (int)(v[c]);
	    }
	  }
	}
	break;
      case operandSVValueInt:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return v;
	  }
	}
	break;
      default:
	;
    }

    return 0;
  }

  csConditionEvaluator::EvaluatorShadervar::FloatType 
  csConditionEvaluator::EvaluatorShadervar::Float (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandFloat:
	return operand.floatVal;
      case operandInt:
	return (float)operand.intVal;
      case operandSVValueFloat:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    float v;
	    if (sv->GetValue (v))
	      return v;
	  }
	}
	break;
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    csVector4 v;
	    if (sv->GetValue (v))
	    {
	      int c = operand.type - operandSVValueX;
	      return v[c];
	    }
	  }
	}
	break;
      case operandSVValueInt:
	{
	  csShaderVariable* sv = GetShaderVar (operand);
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return (float)v;
	  }
	}
	break;
      default:
	;
    }

    return 0.0f;
  }

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
