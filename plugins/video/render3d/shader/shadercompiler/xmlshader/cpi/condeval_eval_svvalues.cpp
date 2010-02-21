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

#include "condeval.h"
#include "condeval_eval_svvalues.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  EvaluatorShadervarValues::BoolType EvaluatorShadervarValues::Boolean (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandOperation:
	{
	  /* Don't use the local trueVars/falseVars since the condition
	     checking may change them to something which is not correct for
	     the whole condition. */
	  Logic3 result (evaluator.CheckConditionResultsInternal (operand.operation,
	    vars));
          
	  ValueSetBool& vs = CreateValueBool();
	  ValueSetBool& vsTrue = CreateValueBool();
	  ValueSetBool& vsFalse = CreateValueBool();
	  switch (result.state)
	  {
	    case Logic3::Truth: vs = true; break;
	    case Logic3::Lie:   vs = false; break;
	    default:
	      /* vs defaults to 'uncertainity' */
	      vsTrue = true;
	      vsFalse = false;
	  }
	  return JanusValueSetBool (vs, vsTrue, vsFalse);
	}
      case operandBoolean:
	{
	  ValueSetBool& vs = CreateValueBool();
	  vs = operand.boolVal;
	  ValueSetBool& vsTrue = CreateValueBool();
	  ValueSetBool& vsFalse = CreateValueBool();
	  return JanusValueSetBool (vs, vsTrue, vsFalse);
	}
      case operandSV:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetVar();
	  Variables::Values* valuesTrue = ValuesForOperand (trueVars, operand);
	  Variables::Values* valuesFalse = ValuesForOperand (falseVars, operand);
	  return JanusValueSetBool (vs, valuesTrue->GetVar(), valuesFalse->GetVar());
	}
      case operandSVValueTexture:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetTex();
	  Variables::Values* valuesTrue = ValuesForOperand (trueVars, operand);
	  Variables::Values* valuesFalse = ValuesForOperand (falseVars, operand);
	  return JanusValueSetBool (vs, valuesTrue->GetTex(), valuesFalse->GetTex());
	}
      case operandSVValueBuffer:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  ValueSetBool& vs = CreateValueBool();
	  vs = startValues->GetBuf();
	  Variables::Values* valuesTrue = ValuesForOperand (trueVars, operand);
	  Variables::Values* valuesFalse = ValuesForOperand (falseVars, operand);
	  return JanusValueSetBool (vs, valuesTrue->GetBuf(), valuesFalse->GetBuf());
	}
      default:
	{
	  CS_ASSERT_MSG("Bug: Unexpected operand type", false);
	}
    }

    ValueSetBool& vsTrue = CreateValueBool();
    ValueSetBool& vsFalse = CreateValueBool();
    return JanusValueSetBool (boolUncertain, vsTrue, vsFalse);
  }

  EvaluatorShadervarValues::FloatType EvaluatorShadervarValues::Float (
    const CondOperand& operand)
  {
    switch (operand.type)
    {
      case operandFloat:
	{
	  ValueSet& vs = CreateValue();
	  vs = operand.floatVal;
	  ValueSet& vsTrue = CreateValue();
	  vsTrue = vs;
	  ValueSet& vsFalse = CreateValue();
	  vsFalse = vsTrue;
	  return JanusValueSet (vs, vsTrue, vsFalse);
	}
      case operandInt:
	{
	  ValueSet& vs = CreateValue();
	  vs = float (operand.intVal);
	  ValueSet& vsTrue = CreateValue();
	  vsTrue = vs;
	  ValueSet& vsFalse = CreateValue();
	  vsFalse = vsTrue;
	  return JanusValueSet (vs, vsTrue, vsFalse);
	}
      case operandSVValueFloat:
      case operandSVValueInt:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  Variables::Values* valuesTrue = ValuesForOperand (trueVars, operand);
	  Variables::Values* valuesFalse = ValuesForOperand (falseVars, operand);
	  return JanusValueSet (startValues->GetVec (0), valuesTrue->GetVec (0), valuesFalse->GetVec (0));
	}
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
	{
	  const Variables::Values* startValues = ValuesForOperand (operand);
	  Variables::Values* valuesTrue = ValuesForOperand (trueVars, operand);
	  Variables::Values* valuesFalse = ValuesForOperand (falseVars, operand);
	  int c = operand.type - operandSVValueX;
	  return JanusValueSet (startValues->GetVec (c), valuesTrue->GetVec (c), valuesFalse->GetVec (c));
	}
	break;
      default:
	;
    }

    const ValueSet& startValues = CreateValue();
    ValueSet& vsTrue = CreateValue();
    ValueSet& vsFalse = CreateValue();
    return JanusValueSet (startValues, vsTrue, vsFalse);
  }

  EvaluatorShadervarValues::EvalResult EvaluatorShadervarValues::LogicAnd (
    const CondOperand& a, const CondOperand& b)
  {
    Logic3 rA;
    Variables trueVarsA; 
    Variables falseVarsA;
    CS_ASSERT (a.type == operandOperation);
    rA = evaluator.CheckConditionResultsInternal (a.operation,
      vars, trueVarsA, falseVarsA);

    Logic3 rB;
    CS_ASSERT (b.type == operandOperation);
    if (rA.state == Logic3::Truth)
    {
      /* A is definitely true: so the possible values for a true/false outcome
       * are those of evaluating B. */
      rB = evaluator.CheckConditionResultsInternal (b.operation,
	trueVarsA, trueVars, falseVars);
    }
    else if (rA.state == Logic3::Lie)
    {
      /* A is definitely false: so the possible values for a true/false outcome
       * are those of evaluating A. */
      trueVars = trueVarsA;
      falseVars = falseVarsA;
      rB = Logic3::Uncertain;
    }
    else
    {
      Logic3 rAT;
      Variables trueVarsAT;
      Variables falseVarsAT;
      rAT = evaluator.CheckConditionResultsInternal (b.operation,
	trueVarsA, trueVarsAT, falseVarsAT);

      Logic3 rAF;
      Variables trueVarsAF;
      Variables falseVarsAF;
      rAF = evaluator.CheckConditionResultsInternal (b.operation,
	falseVarsA, trueVarsAF, falseVarsAF);

      trueVars = trueVarsAT;
      falseVars = falseVarsAT | trueVarsAF | falseVarsAF;

      if ((rAT.state == Logic3::Truth) && (rAF.state == Logic3::Truth))
	rB = Logic3::Truth;
      else if ((rAT.state == Logic3::Lie) && (rAF.state == Logic3::Lie))
	rB = Logic3::Lie;
      else
	rB = Logic3::Uncertain;
    }
    return rA && rB;
  }

  EvaluatorShadervarValues::EvalResult EvaluatorShadervarValues::LogicOr (
    const CondOperand& a, const CondOperand& b)
  {
    Logic3 rA;
    Variables trueVarsA; 
    Variables falseVarsA;
    CS_ASSERT (a.type == operandOperation);
    rA = evaluator.CheckConditionResultsInternal (a.operation,
      vars, trueVarsA, falseVarsA);

    Logic3 rB;
    CS_ASSERT (b.type == operandOperation);
    if (rA.state == Logic3::Truth)
    {
      /* A is definitely true: so the possible values for a true/false outcome
       * are those of evaluating A. */
      trueVars = trueVarsA;
      falseVars = falseVarsA;
      rB = Logic3::Uncertain;
    }
    else if (rA.state == Logic3::Lie)
    {
      /* A is definitely false: so the possible values for a true/false outcome
       * are those of evaluating B. */
      rB = evaluator.CheckConditionResultsInternal (b.operation,
	falseVarsA, trueVars, falseVars);
    }
    else
    {
      Logic3 rAT;
      Variables trueVarsAT;
      Variables falseVarsAT;
      rAT = evaluator.CheckConditionResultsInternal (b.operation,
	trueVarsA, trueVarsAT, falseVarsAT);

      Logic3 rAF;
      Variables trueVarsAF;
      Variables falseVarsAF;
      rAF = evaluator.CheckConditionResultsInternal (b.operation,
	falseVarsA, trueVarsAF, falseVarsAF);

      trueVars = trueVarsAT | falseVarsAT | trueVarsAF;
      falseVars = falseVarsAF;
      
      if ((rAT.state == Logic3::Truth) && (rAF.state == Logic3::Truth))
	rB = Logic3::Truth;
      else if ((rAT.state == Logic3::Lie) && (rAF.state == Logic3::Lie))
	rB = Logic3::Lie;
      else
	rB = Logic3::Uncertain;
    }
    return rA || rB;
  }
}
CS_PLUGIN_NAMESPACE_END(XMLShader)
