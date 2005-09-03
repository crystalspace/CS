/*
  Copyright (C) 2004 by Frank Richter
	    (C) 2004 by Jorrit Tyberghein

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
#include "tokenhelper.h"

bool csConditionConstants::AddConstant (const char* name, float value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandFloat;
  op.floatVal = value;

  constants.Put (name, op);
  return true;
}

bool csConditionConstants::AddConstant (const char* name, int value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandInt;
  op.intVal = value;

  constants.Put (name, op);
  return true;
}

bool csConditionConstants::AddConstant (const char* name, bool value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandBoolean;
  op.boolVal = value;

  constants.Put (name, op);
  return true;
}

//---------------------------------------------------------------------------

csConditionEvaluator::csConditionEvaluator (iStringSet* strings, 
    const csConditionConstants& constants) :
    strings(strings), nextConditionID(0), constants(constants)
{
}

const char* csConditionEvaluator::SetLastError (const char* msg, ...)
{
  csString temp;
  va_list args;
  va_start (args, msg);
  temp.FormatV (msg, args);
  va_end (args);
  return lastError.Replace (temp);
}

bool csConditionEvaluator::OpTypesCompatible (OperandType t1, OperandType t2)
{
  switch (t1)
  {
    case operandNone:
      return false;
    case operandOperation:
      return OpTypesCompatible (operandBoolean, t2);
    case operandFloat:
      return (t1 == t2) || (t2 == operandInt) 
	|| ((t2 >= operandSV) && OpTypesCompatible (t2, operandFloat));
	//(t2 == operandSVValueInt) || (t2 == operandSVValueFloat);
    case operandInt:
      return (t1 == t2) || (t2 == operandFloat) 
	|| ((t2 >= operandSV) && OpTypesCompatible (t2, operandInt));
      /*|| (t2 == operandSVValueInt) 
	|| (t2 == operandSVValueFloat);*/
    case operandBoolean:
      return (t1 == t2) || (t2 == operandOperation)
	|| ((t2 >= operandSV) && OpTypesCompatible (t2, operandBoolean));  
      /*(t2 == operandSV) || (t2 == operandSVValueTexture)
	|| (t2 == operandSVValueBuffer);*/

    case operandSV:
      return OpTypesCompatible (operandBoolean, t2);
    case operandSVValueInt:
      return OpTypesCompatible (operandInt, t2);
    case operandSVValueFloat:
      return OpTypesCompatible (operandFloat, t2);
    case operandSVValueTexture:
      return OpTypesCompatible (operandBoolean, t2);
    case operandSVValueBuffer:
      return OpTypesCompatible (operandBoolean, t2);

    default:
      CS_ASSERT (false);
      return false;
  }
}

const char* csConditionEvaluator::OperandTypeDescription (OperandType t)
{
  switch (t)
  {
    case operandNone:
      return "none";
    case operandOperation:
      return "operation";
    case operandFloat:
      return "float";
    case operandInt:
      return "int";
    case operandBoolean:
      return "bool";
    case operandSV:
      return "shadervar";
    case operandSVValueInt:
      return "shadervar int value";
    case operandSVValueFloat:
      return "shadervar float value";
    case operandSVValueTexture:
      return "shadervar texture value";
    case operandSVValueBuffer:
      return "shadervar buffer value";

    default:
      CS_ASSERT (false);
      return 0;
  }
}

csConditionID csConditionEvaluator::FindOptimizedCondition (
  const CondOperation& operation)
{
  CondOperation newOp = operation;
  if ((newOp.left.type == operandOperation)
    && ((newOp.left.operation == csCondAlwaysFalse)
    || (newOp.left.operation == csCondAlwaysTrue)))
  {
    newOp.left.type = operandBoolean;
    newOp.left.boolVal = newOp.left.operation == csCondAlwaysTrue;
  }
  if ((newOp.right.type == operandOperation)
    && ((newOp.right.operation == csCondAlwaysFalse)
    || (newOp.right.operation == csCondAlwaysTrue)))
  {
    newOp.right.type = operandBoolean;
    newOp.right.boolVal = newOp.left.operation == csCondAlwaysTrue;
  }
  if ((newOp.left.type >= operandFloat) 
    && (newOp.left.type < operandSV)
    && (newOp.right.type >= operandFloat)
    && (newOp.right.type < operandSV))
  {
    bool res;
    if (EvaluateConst (newOp, res))
      return (res ? csCondAlwaysTrue : csCondAlwaysFalse);
  }
  if (newOp.operation == opAnd)
  {
    if ((newOp.left.type == operandBoolean) && (newOp.left.boolVal == false))
      return csCondAlwaysFalse;
    if ((newOp.right.type == operandBoolean) && (newOp.right.boolVal == false))
      return csCondAlwaysFalse;
    if ((newOp.left.type == operandBoolean) && (newOp.left.boolVal == true))
    {
      if (newOp.right.type == operandOperation)
	return (newOp.right.operation);
      else
      {
	CondOperation newNewOp;
	newNewOp.left = newOp.right;
	newNewOp.operation = opEqual;
	newNewOp.right.type = operandBoolean;
	newNewOp.right.boolVal = true;
	return FindOptimizedCondition (newNewOp);
      }
    }
    if ((newOp.right.type == operandBoolean) && (newOp.right.boolVal == true))
    {
      if (newOp.left.type == operandOperation)
	return (newOp.left.operation);
      else
      {
	CondOperation newNewOp;
	newNewOp.left = newOp.left;
	newNewOp.operation = opEqual;
	newNewOp.right.type = operandBoolean;
	newNewOp.right.boolVal = true;
	return FindOptimizedCondition (newNewOp);
      }
    }
  }
  else if (newOp.operation == opOr)
  {
    if ((newOp.left.type == operandBoolean) && (newOp.left.boolVal == true))
      return csCondAlwaysTrue;
    if ((newOp.right.type == operandBoolean) && (newOp.right.boolVal == true))
      return csCondAlwaysTrue;
    if ((newOp.left.type == operandBoolean) && (newOp.left.boolVal == false))
    {
      if (newOp.right.type == operandOperation)
	return (newOp.right.operation);
      else
      {
	CondOperation newNewOp;
	newNewOp.left = newOp.right;
	newNewOp.operation = opEqual;
	newNewOp.right.type = operandBoolean;
	newNewOp.right.boolVal = true;
	return FindOptimizedCondition (newNewOp);
      }
    }
    if ((newOp.right.type == operandBoolean) && (newOp.right.boolVal == false))
    {
      if (newOp.left.type == operandOperation)
	return (newOp.left.operation);
      else
      {
	CondOperation newNewOp;
	newNewOp.left = newOp.left;
	newNewOp.operation = opEqual;
	newNewOp.right.type = operandBoolean;
	newNewOp.right.boolVal = true;
	return FindOptimizedCondition (newNewOp);
      }
    }
  }
  else if (newOp.operation == opNot)
  {
    if (newOp.right.type == operandBoolean)
      return newOp.right.boolVal ? csCondAlwaysFalse : csCondAlwaysTrue;
  }

  csConditionID id = conditions.Get (operation, (csConditionID)~0);
  if (id == (csConditionID)~0)
  {
    id = nextConditionID++;
    conditions.Put (operation, id);
  }
  return id;
}

const char* csConditionEvaluator::ResolveExpValue (const csExpressionToken& value,
  CondOperand& operand)
{
  if (value.type == tokenNumber)
  {
    csString number;
    number.Append (value.tokenStart, value.tokenLen);
    bool isFloat = (strpbrk (number, ".eE") != 0);
    if (isFloat)
    {
      char dummy;
      if (sscanf (number, "%f%c", &operand.floatVal, &dummy) != 1)
      {
	return SetLastError ("Malformed float value: '%s'",
	  number.GetData());
      }
      operand.type = operandFloat;
    }
    else
    {
      char dummy;
      if (sscanf (number, "%d%c", &operand.intVal, &dummy) != 1)
      {
	return SetLastError ("Malformed int value: '%s'",
	  number.GetData());
      }
      operand.type = operandInt;
    }
    return 0;
  }
  else if (value.type == tokenIdentifier)
  {
    if (TokenEquals (value.tokenStart, value.tokenLen, "true"))
    {
      operand.type = operandBoolean;
      operand.boolVal = true;
      return 0;
    }
    else if (TokenEquals (value.tokenStart, value.tokenLen, "false"))
    {
      operand.type = operandBoolean;
      operand.boolVal = false;
      return 0;
    }
    else
    {
      return SetLastError ("Unknown identifier '%s'",
	csExpressionToken::Extractor (value).Get ());
    }
  }
  else
  {
    return SetLastError ("Value of '%s' of type '%s'",
      csExpressionToken::Extractor (value).Get (), csExpressionToken::TypeDescription (value.type));
  }

  CS_ASSERT (false);
  return 0;
}

const char* csConditionEvaluator::ResolveOperand (csExpression* expression, 
  CondOperand& operand)
{
  const char* err;

  if (expression->type == csExpression::Value)
  {
    err = ResolveExpValue (expression->valueValue, operand);
    if (err)
    {
      return SetLastError ("Can't resolve value '%s': %s",
	csExpressionToken::Extractor (expression->valueValue).Get (), err);
    }
    return 0;
  }

  const csExpressionToken& t = expression->expressionValue.op;
  if (TokenEquals (t.tokenStart, t.tokenLen, "."))
  {
    CS_ASSERT (expression->expressionValue.left->type 
      == csExpression::Value);
    const csExpressionToken& left = 
      expression->expressionValue.left->valueValue;
    if (TokenEquals (left.tokenStart, left.tokenLen, "vars"))
    {
      err = ResolveSVIdentifier (expression->expressionValue.right,
	operand);
      if (err)
	return err;
      return 0;
    }
    else if (TokenEquals (left.tokenStart, left.tokenLen, "consts"))
    {
      err = ResolveConst (expression->expressionValue.right,
	operand);
      if (err)
	return err;
      return 0;
    }
    else
    {
      return SetLastError ("Unknown identifier '%s'",
	csExpressionToken::Extractor (left).Get ());
    }
  }
  else
  {
    operand.type = operandOperation;
    err = ProcessExpression (expression, operand.operation);
    if (err)
      return err;
    return 0;
  }
  CS_ASSERT (false);
  return 0;
}

const char* csConditionEvaluator::ResolveSVIdentifier (
  csExpression* expression, CondOperand& operand)
{
  if (expression->type == csExpression::Value)
  {
    operand.type = operandSV;
    operand.svName = strings->Request (
      csExpressionToken::Extractor (expression->valueValue).Get ());
    return 0;
  }
  else
  {
    const csExpressionToken& t = expression->expressionValue.op;
    if (!TokenEquals (t.tokenStart, t.tokenLen, "."))
    {
      return SetLastError ("Unexpected operator '%s'",
	csExpressionToken::Extractor (t).Get ());
    }
    if (expression->expressionValue.left->type != csExpression::Value)
    {
      CS_ASSERT_MSG ("It should not happen that the 'left' subexpression "
	"is not a value", false);
      return "Left subexpression is not of type 'value'";
    }
    if (expression->expressionValue.right->type != csExpression::Value)
    {
      /* @@@ Happens for stuff like vars.foo.bar.baz, where bar.baz
       * will incarnate here as a right expression of type Expression.
       * Clearer error msg?
       */
      return "Right subexpression is not of type 'value'";
    }
    operand.svName = strings->Request (
      csExpressionToken::Extractor (expression->expressionValue.left->valueValue).Get ());
    const csExpressionToken& right = 
      expression->expressionValue.right->valueValue;
    if (TokenEquals (right.tokenStart, right.tokenLen, "int"))
    {
      operand.type = operandSVValueInt;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "float"))
    {
      operand.type = operandSVValueFloat;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "buffer"))
    {
      operand.type = operandSVValueBuffer;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "texture"))
    {
      operand.type = operandSVValueTexture;
    }
    else
    {
      return SetLastError ("Unknown shader variable specializer '%s'",
	csExpressionToken::Extractor (right).Get ());
    }
    return 0;
  }
  CS_ASSERT (false);
  return 0;
}

const char* csConditionEvaluator::ResolveConst (csExpression* expression, 
						CondOperand& operand)
{
  if (expression->type == csExpression::Value)
  {
    csExpressionToken::Extractor symbol (expression->valueValue);
    const CondOperand* constOp = 
      constants.constants.GetElementPointer (symbol.Get());
    if (!constOp)
    {
      SetLastError ("Unknown symbol '%s'", symbol.Get());
    }
    operand = *constOp;
    return 0;
  }
  else
    return "Expression is not a value";
}

const char* csConditionEvaluator::ProcessExpression (
  csExpression* expression, csConditionID& cond)
{
  const char* err;
  CondOperation newOp;

  if (expression->type == csExpression::Value)
  {
    newOp.operation = opEqual;
    err = ResolveExpValue (expression->valueValue, newOp.left);
    if (err)
    {
      return SetLastError ("Can't resolve value '%s': %s",
	csExpressionToken::Extractor (expression->valueValue).Get (), err);
    }
    newOp.right.type = operandBoolean;
    newOp.right.boolVal = true;
    if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
    {
      return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	csExpressionToken::Extractor (expression->valueValue).Get (), OperandTypeDescription (newOp.left.type),
	OperandTypeDescription (newOp.right.type));
    }
  }
  else
  {
    const csExpressionToken& t = expression->expressionValue.op;
    if (TokenEquals (t.tokenStart, t.tokenLen, "."))
    {
      newOp.operation = opEqual;
      err = ResolveOperand (expression, newOp.left);
      if (err != 0) return err;
      newOp.right.type = operandBoolean;
      newOp.right.boolVal = true;
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "!"))
    {
      memset (&newOp.left, 0, sizeof (newOp.left));
      newOp.left.type = operandNone;
      newOp.operation = opNot;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.right.type),
	  OperandTypeDescription (operandBoolean));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "=="))
    {
      newOp.operation = opEqual;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "!="))
    {
      newOp.operation = opNEqual;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "<"))
    {
      newOp.operation = opLesser;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "<="))
    {
      newOp.operation = opLesserEq;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, ">="))
    {
      newOp.operation = opLesserEq;
      err = ResolveOperand (expression->expressionValue.left, newOp.right);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.left);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, ">"))
    {
      newOp.operation = opLesser;
      err = ResolveOperand (expression->expressionValue.left, newOp.right);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.left);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (newOp.right.type));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "&&"))
    {
      newOp.operation = opAnd;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.right.type),
	  OperandTypeDescription (operandBoolean));
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "||"))
    {
      newOp.operation = opOr;
      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.right.type),
	  OperandTypeDescription (operandBoolean));
      }
    }
    else
    {
      return SetLastError ("Unknown operator '%s'", 
	csExpressionToken::Extractor (t).Get ());
    }
  }

  cond = FindOptimizedCondition (newOp);
  return 0;
}

bool csConditionEvaluator::Evaluate (csConditionID condition, 
				     const csRenderMeshModes& modes,
				     const csShaderVarStack& stacks)
{
  if (condition == csCondAlwaysTrue)
    return true;
  else if (condition == csCondAlwaysFalse)
    return false;

  /* Hack: it can happen that while evaluating a shader, new conditions 
   * are added (notably when a shader source is retrieved from an
   * external source). Make sure the cache is large enough.
   */
  if (condChecked.Length() < GetNumConditions ())
  {
    condChecked.SetLength (GetNumConditions ());
    condResult.SetLength (GetNumConditions ());
  }

  if (condChecked.IsBitSet (condition))
  {
    return condResult.IsBitSet (condition);
  }

  bool result = false;

  const CondOperation* op = conditions.GetKeyPointer (condition);
  CS_ASSERT (op != 0);

  switch (op->operation)
  {
    case opNot:
      result = !EvaluateOperandB (op->right, modes, stacks);
      break;
    case opAnd:
      result = EvaluateOperandB (op->left, modes, stacks)
	&& EvaluateOperandB (op->right, modes, stacks);
      break;
    case opOr:
      result = EvaluateOperandB (op->left, modes, stacks)
	|| EvaluateOperandB (op->right, modes, stacks);
      break;
    case opEqual:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueFloat))
	{
	  const float f1 = EvaluateOperandF (op->left, modes, stacks);
	  const float f2 = EvaluateOperandF (op->right, modes, stacks);
	  result = (f1 - f2) < EPSILON;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  result = EvaluateOperandB (op->left, modes, stacks)
	    == EvaluateOperandB (op->right, modes, stacks);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  result = i1 == i2;
	}
	break;
      }
    case opNEqual:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueFloat))
	{
	  const float f1 = EvaluateOperandF (op->left, modes, stacks);
	  const float f2 = EvaluateOperandF (op->right, modes, stacks);
	  result = (f1 - f2) >= EPSILON;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  result = EvaluateOperandB (op->left, modes, stacks)
	    != EvaluateOperandB (op->right, modes, stacks);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  result = i1 != i2;
	}
	break;
      }
    case opLesser:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueFloat))
	{
	  const float f1 = EvaluateOperandF (op->left, modes, stacks);
	  const float f2 = EvaluateOperandF (op->right, modes, stacks);
	  result = (f1 < f2);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  result = i1 < i2;
	}
	break;
      }
    case opLesserEq:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueFloat))
	{
	  const float f1 = EvaluateOperandF (op->left, modes, stacks);
	  const float f2 = EvaluateOperandF (op->right, modes, stacks);
	  result = (f1 <= f2);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  result = i1 <= i2;
	}
	break;
      }
    default:
      CS_ASSERT (false);
  }

  condChecked.Set (condition, true);
  condResult.Set (condition, result);

  return result;
}

void csConditionEvaluator::ResetEvaluationCache()
{
  condChecked.SetLength (GetNumConditions ());
  condChecked.Clear();
  condResult.SetLength (GetNumConditions ());
}

bool csConditionEvaluator::EvaluateOperandB (const CondOperand& operand, 
  const csRenderMeshModes& modes, const csShaderVarStack& stacks)
{
  switch (operand.type)
  {
    case operandOperation:
      return Evaluate (operand.operation, modes, stacks);
    case operandBoolean:
      return operand.boolVal;
    case operandSV:
      {
	if (stacks.Length() > operand.svName)
	{
	  csShaderVariable* sv = stacks[operand.svName];
	  return sv != 0;
	}
      }
    case operandSVValueTexture:
      {
	if (stacks.Length() > operand.svName)
	{
	  csShaderVariable* sv = stacks[operand.svName];
	  if (sv != 0)
	  {
	    iTextureHandle* th;
	    if (sv->GetValue (th))
	      return th != 0;
	  }
	}
      }
    case operandSVValueBuffer:
      //@@TODO: CHECK FOR DEFAULTBUFFERS
      {
	if (stacks.Length() > operand.svName)
	{
	  csShaderVariable* sv = stacks[operand.svName];
	  if (sv != 0)
	  {
	    iRenderBuffer* th;
	    if (sv->GetValue (th))
	      return th != 0;
	  }
	}
      }
    default:
      ;
  }
  return false;
}

int csConditionEvaluator::EvaluateOperandI (const CondOperand& operand, 
  const csRenderMeshModes& modes, const csShaderVarStack& stacks)
{
  switch (operand.type)
  {
    case operandInt:
      return operand.intVal;
    case operandFloat:
      return (int)operand.floatVal;
    case operandSVValueFloat:
      {
	if (stacks.Length() > operand.svName)
	{
	    csShaderVariable* sv = stacks[operand.svName];
	    if (sv != 0)
	    {
	      float v;
	      if (sv->GetValue (v))
		return (int)v;
	    }
	}
      }
    case operandSVValueInt:
      {
	if (stacks.Length() > operand.svName)
	{
	  csShaderVariable* sv = stacks[operand.svName];
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return v;
	  }
	}
      }
    default:
      ;
  }

  return 0;
}

float csConditionEvaluator::EvaluateOperandF (const CondOperand& operand, 
  const csRenderMeshModes& modes, const csShaderVarStack& stacks)
{
  switch (operand.type)
  {
    case operandFloat:
      return operand.floatVal;
    case operandInt:
      return (float)operand.intVal;
    case operandSVValueFloat:
      {
	if (stacks.Length() > operand.svName)
	{
	  csShaderVariable* sv = stacks[operand.svName];
	  if (sv != 0)
	  {
	    float v;
	    if (sv->GetValue (v))
	      return v;
	  }
	}
      }
    case operandSVValueInt:
      {
	if (stacks.Length() > operand.svName)
	{
          csShaderVariable* sv = stacks[operand.svName];
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return (float)v;
	  }
	}
      }
    default:
      ;
  }

  return 0.0f;
}

bool csConditionEvaluator::EvaluateConst (const CondOperation& op, bool& result)
{
  bool rB1, rB2;
  int rI1, rI2;
  float rF1, rF2;
  switch (op.operation)
  {
    case opNot:
      if (!EvaluateOperandBConst (op.right, rB1)) return false;
      result = !rB1;
      break;
    case opAnd:
      if (!EvaluateOperandBConst (op.left, rB1)) return false;
      if (!EvaluateOperandBConst (op.right, rB2)) return false;
      result = rB1 && rB2;
      break;
    case opOr:
      if (!EvaluateOperandBConst (op.left, rB1)) return false;
      if (!EvaluateOperandBConst (op.right, rB2)) return false;
      result = rB1 || rB2;
      break;
    case opEqual:
      {
	if ((op.left.type == operandFloat) || (op.right.type == operandFloat))
	{
	  if (!EvaluateOperandFConst (op.left, rF1)) return false;
	  if (!EvaluateOperandFConst (op.right, rF2)) return false;
	  result = (rF1 - rF2) < EPSILON;
	}
	else if (OpTypesCompatible (op.left.type, operandBoolean) 
	  && OpTypesCompatible (op.right.type, operandBoolean))
	{
	  if (!EvaluateOperandBConst (op.left, rB1)) return false;
	  if (!EvaluateOperandBConst (op.right, rB2)) return false;
	  result = rB1 == rB2;
	}
	else
	{
	  if (!EvaluateOperandIConst (op.left, rI1)) return false;
	  if (!EvaluateOperandIConst (op.right, rI2)) return false;
	  result = rI1 == rI2;
	}
	break;
      }
    case opNEqual:
      {
	CondOperation op2 = op;
	op2.operation = opEqual;
	if (!EvaluateConst (op2, result)) return false;
	result = !result;
	break;
      }
    case opLesser:
      {
	if ((op.left.type == operandFloat) || (op.right.type == operandFloat))
	{
	  if (!EvaluateOperandFConst (op.left, rF1)) return false;
	  if (!EvaluateOperandFConst (op.right, rF2)) return false;
	  result = rF1 < rF2;
	}
	else
	{
	  if (!EvaluateOperandIConst (op.left, rI1)) return false;
	  if (!EvaluateOperandIConst (op.right, rI2)) return false;
	  result = rI1 < rI2;
	}
	break;
      }
    case opLesserEq:
      {
	if ((op.left.type == operandFloat) || (op.right.type == operandFloat))
	{
	  if (!EvaluateOperandFConst (op.left, rF1)) return false;
	  if (!EvaluateOperandFConst (op.right, rF2)) return false;
	  result = rF1 <= rF2;
	}
	else
	{
	  if (!EvaluateOperandIConst (op.left, rI1)) return false;
	  if (!EvaluateOperandIConst (op.right, rI2)) return false;
	  result = rI1 <= rI2;
	}
	break;
      }
    default:
      return false;
  }
  return true;
}

bool csConditionEvaluator::EvaluateOperandBConst (const CondOperand& operand, 
  bool& result)
{
  switch (operand.type)
  {
    case operandOperation:
      {
	if (operand.operation == csCondAlwaysTrue)
	  result = true;
	else if (operand.operation == csCondAlwaysFalse)
	  result = false;
	else
	{
	  const CondOperation* op = 
	    conditions.GetKeyPointer (operand.operation);
	  CS_ASSERT (op != 0);
	  if (!EvaluateConst (*op, result)) return false;
	}
      }
      break;
    case operandBoolean:
      result = operand.boolVal;
      break;
    default:
      return false;
  }
  return true;
}

bool csConditionEvaluator::EvaluateOperandIConst (const CondOperand& operand, 
  int& result)
{
  switch (operand.type)
  {
    case operandInt:
      result = operand.intVal;
      break;
    case operandFloat:
      result = (int)operand.floatVal;
      break;
    default:
      return false;
  }

  return true;
}

bool csConditionEvaluator::EvaluateOperandFConst (const CondOperand& operand, 
  float& result)
{
  switch (operand.type)
  {
    case operandFloat:
      result = operand.floatVal;
      break;
    case operandInt:
      result = (float)operand.intVal;
      break;
    default:
      return false;
  }

  return true;
}

bool csConditionEvaluator::ConditionIndependent (csConditionID a, bool aVal,
						 csConditionID b)
{
  if (a == b) return false;

  if (aVal && (b == csCondAlwaysTrue)) return false;
  if (!aVal && (b == csCondAlwaysFalse)) return false;

  if ((a == csCondAlwaysFalse) || (a == csCondAlwaysTrue) 
    || (b == csCondAlwaysFalse) || (b == csCondAlwaysTrue))
    return true;

  const CondOperation* A = conditions.GetKeyPointer (a);
  CS_ASSERT (A);
  const CondOperation* B = conditions.GetKeyPointer (b);
  CS_ASSERT (B);

  if ((A->operation == opNot) && (A->right.type >= operandSV)
    && (B->operation == opEqual) && (B->left.type >= operandSV)
    && (B->right.type == operandBoolean) && (B->right.boolVal == true)
    && (B->left.svName == A->right.svName)) return false;
  if ((B->operation == opNot) && (B->right.type >= operandSV)
    && (A->operation == opEqual) && (A->left.type >= operandSV)
    && (A->right.type == operandBoolean) && (A->right.boolVal == true)
    && (A->left.svName == B->right.svName)) return false;
  
  if ((A->operation == opNot) && (A->right.type == operandOperation)
    && (A->right.operation == b)) return false;
  if ((B->operation == opNot) && (B->right.type == operandOperation)
    && (B->right.operation == a)) return false;

  return true;
}
