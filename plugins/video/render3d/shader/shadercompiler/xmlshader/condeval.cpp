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

bool csConditionEvaluator::OperationHashKeyHandler::IsOpCommutative (
  ConditionOp op)
{
  return (op == opAnd) || (op == opOr) || (op == opEqual) || (op == opNEqual);
}

uint csConditionEvaluator::OperationHashKeyHandler::ActualHash (
  ConditionOp operation, const CondOperand& left, const CondOperand& right)
{
  CondOperation tempOp;
  tempOp.operation = operation;
  tempOp.left = left;
  tempOp.right = right;
  return csHashCompute ((char*)&tempOp, sizeof (tempOp));
}

uint csConditionEvaluator::OperationHashKeyHandler::ComputeHash (
  const CondOperation& operation)
{
  uint result = ActualHash (operation.operation, operation.left, 
    operation.right);
  if (IsOpCommutative (operation.operation))
    result ^= ActualHash (operation.operation, operation.right, 
    operation.left);
  return result;
}

bool csConditionEvaluator::OperationHashKeyHandler::CompareKeys (
  const CondOperation& op1, const CondOperation& op2)
{
  if (op1.operation == op2.operation)
  {
    bool result = (memcmp (&op1.left, &op2.left, sizeof (CondOperand)) == 0)
      && (memcmp (&op1.right, &op2.right, sizeof (CondOperand)) == 0);
    if (IsOpCommutative (op1.operation))
    {
      result = result 
	|| ((memcmp (&op1.left, &op2.right, sizeof (CondOperand)) == 0)
	&& (memcmp (&op1.right, &op2.left, sizeof (CondOperand)) == 0));
    }
    return result;
  }
  return false;
}

csConditionEvaluator::csConditionEvaluator (iStringSet* strings) : 
  nextConditionID(0)
{
  csConditionEvaluator::strings = strings;
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

csConditionID csConditionEvaluator::FindCondition (
  const CondOperation& operation)
{
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
    cond = FindCondition (newOp);
    return 0;
  }
  else
  {
    const csExpressionToken& t = expression->expressionValue.op;
    if (TokenEquals (t.tokenStart, t.tokenLen, "."))
    {
      newOp.operation = opEqual;
      err = ResolveOperand (expression, newOp.left);
      if (err)
      {
	return err;
      }
      newOp.right.type = operandBoolean;
      newOp.right.boolVal = true;
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
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
      cond = FindCondition (newOp);
      return 0;
    }
    else
    {
      return SetLastError ("Unknown operator '%s'", 
	csExpressionToken::Extractor (t).Get ());
    }
  }

  CS_ASSERT (false);
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

  const CondOperation* op = conditions.GetKeyPointer (condition);
  CS_ASSERT (op != 0);

  switch (op->operation)
  {
    case opNot:
      return !EvaluateOperandB (op->right, modes, stacks);
    case opAnd:
      return EvaluateOperandB (op->left, modes, stacks)
	&& EvaluateOperandB (op->right, modes, stacks);
    case opOr:
      return EvaluateOperandB (op->left, modes, stacks)
	|| EvaluateOperandB (op->right, modes, stacks);
    case opEqual:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueFloat))
	{
	  const float f1 = EvaluateOperandF (op->left, modes, stacks);
	  const float f2 = EvaluateOperandF (op->right, modes, stacks);
	  return (f1 - f2) < EPSILON;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  return EvaluateOperandB (op->left, modes, stacks)
	    == EvaluateOperandB (op->right, modes, stacks);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  return i1 == i2;
	}
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
	  return (f1 - f2) >= EPSILON;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  return EvaluateOperandB (op->left, modes, stacks)
	    != EvaluateOperandB (op->right, modes, stacks);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  return i1 != i2;
	}
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
	  return (f1 < f2);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  return i1 < i2;
	}
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
	  return (f1 <= f2);
	}
	else
	{
	  const int i1 = EvaluateOperandI (op->left, modes, stacks);
	  const int i2 = EvaluateOperandI (op->right, modes, stacks);
	  return i1 <= i2;
	}
      }
    default:
      CS_ASSERT (false);
  }

  return false;
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
	  if (stacks[operand.svName].Length() > 0)
	  {
	    csShaderVariable* sv = stacks[operand.svName].Top ();
	    return sv != 0;
	  }
	}
      }
    case operandSVValueTexture:
      {
	if (stacks.Length() > operand.svName)
	{
	  if (stacks[operand.svName].Length() > 0)
	  {
	    csShaderVariable* sv = stacks[operand.svName].Top ();
	    if (sv != 0)
	    {
	      iTextureHandle* th;
	      if (sv->GetValue (th))
		return th != 0;
	    }
	  }
	}
      }
    case operandSVValueBuffer:
      {
	if (stacks.Length() > operand.svName)
	{
	  if (stacks[operand.svName].Length() > 0)
	  {
	    csShaderVariable* sv = stacks[operand.svName].Top ();
	    if (sv != 0)
	    {
	      iRenderBuffer* th;
	      if (sv->GetValue (th))
		return th != 0;
	    }
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
    case operandSVValueInt:
      {
	if (stacks.Length() > operand.svName)
	{
	  if (stacks[operand.svName].Length() > 0)
	  {
	    csShaderVariable* sv = stacks[operand.svName].Top ();
	    if (sv != 0)
	    {
	      int v;
	      if (sv->GetValue (v))
		return v;
	    }
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
    case operandSVValueInt:
      {
	if (stacks.Length() > operand.svName)
	{
	  if (stacks[operand.svName].Length() > 0)
	  {
	    csShaderVariable* sv = stacks[operand.svName].Top ();
	    if (sv != 0)
	    {
	      float v;
	      if (sv->GetValue (v))
		return v;
	    }
	  }
	}
      }
    default:
      ;
  }

  return 0.0f;
}
