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
#include "condeval_eval_svsimple.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  EvaluatorShadervarValuesSimple::BoolType EvaluatorShadervarValuesSimple::Boolean (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandOperation:
	{
	  ValueSetBool& vs = CreateValueBool();
	  Logic3 result (evaluator.EvaluateInternal (*this, operand.operation));
	  switch (result.state)
	  {
	    case Logic3::Truth: vs = true; break;
	    case Logic3::Lie:   vs = false; break;
	    default: /* vs defaults to 'uncertainity' */ break;
	  }
	  return ValueSetBoolWrapper (vs);
	}
      case operandBoolean:
	{
	  ValueSetBool& vs = CreateValueBool();
	  vs = operand.boolVal;
	  return ValueSetBoolWrapper (vs);
	}
      case operandSV:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetVar();
	  return ValueSetBoolWrapper (vs);
	}
      case operandSVValueTexture:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetTex();
	  return ValueSetBoolWrapper (vs);
	}
      case operandSVValueBuffer:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetBuf();
	  return ValueSetBoolWrapper (vs);
	}
      default:
	{
	  CS_ASSERT_MSG("Bug: Unexpected operand type", false);
	}
    }

    return ValueSetBoolWrapper (boolUncertain);
  }

  EvaluatorShadervarValuesSimple::FloatType EvaluatorShadervarValuesSimple::Float (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandFloat:
	{
	  ValueSet& vs = CreateValue();
	  vs = operand.floatVal;
	  return ValueSetWrapper (vs);
	}
      case operandInt:
	{
	  ValueSet& vs = CreateValue();
	  vs = float (operand.intVal);
	  return ValueSetWrapper (vs);
	}
      case operandSVValueFloat:
      case operandSVValueInt:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  return ValueSetWrapper (startValues->GetVec (0));
	}
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  int c = operand.type - operandSVValueX;
	  return ValueSetWrapper (startValues->GetVec (c));
	}
	break;
      default:
	;
    }

    const ValueSet& startValues = CreateValue();
    return ValueSetWrapper (startValues);
  }

  EvaluatorShadervarValuesSimple::EvalResult EvaluatorShadervarValuesSimple::LogicAnd (
    const CondOperand& a, const CondOperand& b)
  {
    Logic3 rA;
    CS_ASSERT (a.type == operandOperation);
    rA = evaluator.CheckConditionResultsInternal (a.operation,
      vars);

    Logic3 rB;
    CS_ASSERT (b.type == operandOperation);
    rB = evaluator.CheckConditionResultsInternal (b.operation,
      vars);
    return rA && rB;
  }

  EvaluatorShadervarValuesSimple::EvalResult EvaluatorShadervarValuesSimple::LogicOr (
    const CondOperand& a, const CondOperand& b)
  {
    Logic3 rA;
    CS_ASSERT (a.type == operandOperation);
    rA = evaluator.CheckConditionResultsInternal (a.operation,
      vars);

    Logic3 rB;
    CS_ASSERT (b.type == operandOperation);
    rB = evaluator.CheckConditionResultsInternal (b.operation,
      vars);
    return rA || rB;
  }

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
