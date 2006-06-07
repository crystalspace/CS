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
#include "tokenhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

using namespace CS;

CS_IMPLEMENT_STATIC_CLASSVAR_REF(Variables::Values, def,
  Def, ValueSet, ());

ValueSet& Variables::Values::GetMultiValue (uint num)
{
  ValueSetChain* p = multiValues;
  while (num > 0) p = p->nextPlease;
  return p->vs;
}

const ValueSet& Variables::Values::GetMultiValue (uint num) const
{
  ValueSetChain* p = multiValues;
  while (num > 0) p = p->nextPlease;
  return p->vs;
}

ValueSet& Variables::Values::GetValue (int type)
{
  CS_ASSERT ((type >= valueFirst) && (type <= valueLast));

  const uint flag = 1 << type;

  if (!(valueFlags & flag))
  {
    uint count = ValueCount (valueFlags);
    SetValueIndex (type, count);
    valueFlags |= flag;
    if (count >= inlinedSets)
    {
      ValueSetChain** d = &multiValues;
      while (*d != 0)
      {
        d = &(*d)->nextPlease;
      }
      *d = new ValueSetChain ();
      return (*d)->vs;
    }
  }

  uint index = GetValueIndex (type);
  if (index < inlinedSets)
    return inlineValues[index];
  else
    return GetMultiValue (index - inlinedSets);
}

const ValueSet& Variables::Values::GetValue (int type) const
{
  CS_ASSERT ((type >= valueFirst) && (type <= valueLast));
  if (valueFlags & (1 << type))
  {
    uint index = GetValueIndex (type);
    if (index < inlinedSets)
      return inlineValues[index];
    else
      return GetMultiValue (index - inlinedSets);
  }
  else
    return Def();
}

//---------------------------------------------------------------------------

struct SliceAllocator
{
  static const size_t valueSetsPerSlice = 32;
  static const size_t sliceSize = valueSetsPerSlice * sizeof (ValueSet);
  typedef uint8 SliceType[sliceSize];

  typedef csBlockAllocator<SliceType, TempHeapAlloc> BlockAlloc;
  CS_DECLARE_STATIC_CLASSVAR_REF (sliceAlloc, SliceAlloc, 
    BlockAlloc);

  static inline uint8* Alloc (size_t blocksize) 
  {
    CS_ASSERT(blocksize == sliceSize);
    return (uint8*)SliceAlloc().AllocUninit();
  }
  static inline void Free (uint8* p)
  {
    SliceAlloc().Free ((SliceType*)p);
  }
  static void CompactAllocator()
  {
    SliceAlloc().Compact();
  }
  static void SetMemTrackerInfo (const char*) {}
};
CS_IMPLEMENT_STATIC_CLASSVAR_REF(SliceAllocator, sliceAlloc,
  SliceAlloc, SliceAllocator::BlockAlloc, (32));

CS_IMPLEMENT_STATIC_CLASSVAR_REF(Variables, valAlloc,
  ValAlloc, Variables::ValBlockAlloc, (1024));
size_t Variables::Values::deallocCount = 0;
CS_IMPLEMENT_STATIC_CLASSVAR(Variables, def,
  Def, Variables::Values, ());
CS_IMPLEMENT_STATIC_CLASSVAR_REF(Variables::CowBlockAllocator, 
  allocator, Allocator, Variables::CowBlockAllocator::BlockAlloc, (256));

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
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
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
    case operandSVValueX:
      return "shadervar vector x value";
    case operandSVValueY:
      return "shadervar vector y value";
    case operandSVValueZ:
      return "shadervar vector z value";
    case operandSVValueW:
      return "shadervar vector w value";
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
    else if (TokenEquals (right.tokenStart, right.tokenLen, "x"))
    {
      operand.type = operandSVValueX;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "y"))
    {
      operand.type = operandSVValueY;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "z"))
    {
      operand.type = operandSVValueZ;
    }
    else if (TokenEquals (right.tokenStart, right.tokenLen, "w"))
    {
      operand.type = operandSVValueW;
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
    const CondOperand* constOp = constants.GetConstant (symbol.Get());
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
  CondOperation newOp;
  const char* err = ProcessExpression (expression, newOp);
  if (err) return err;
  cond = FindOptimizedCondition (newOp);
  return 0;
}

const char* csConditionEvaluator::ProcessExpression (csExpression* expression, 
  CondOperation& operation)
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
      newOp.operation = opEqual;
      err = ResolveOperand (expression->expressionValue.right, newOp.left);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (operandBoolean));
      }
      newOp.right.type = operandBoolean;
      newOp.right.boolVal = false;
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
      if (!OpTypesCompatible (newOp.left.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (newOp.left.type != operandOperation)
      {
        /* Convert to "(left eq true) and (right eq true)" to make the possible 
         * value determination simpler. */
        CondOperation newOpL;
        newOpL.operation = opEqual;
        newOpL.left = newOp.left;
        newOpL.right.type = operandBoolean;
        newOpL.right.boolVal = true;

        newOp.left.type = operandOperation;
        newOp.left.operation = FindOptimizedCondition (newOpL);
      }

      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.right.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (newOp.right.type != operandOperation)
      {
        /* Convert to "(left eq true) and (right eq true)" to make the possible 
         * value determination simpler. */
        CondOperation newOpR;
        newOpR.operation = opEqual;
        newOpR.left = newOp.right;
        newOpR.right.type = operandBoolean;
        newOpR.right.boolVal = true;

        newOp.right.type = operandOperation;
        newOp.right.operation = FindOptimizedCondition (newOpR);
      }
    }
    else if (TokenEquals (t.tokenStart, t.tokenLen, "||"))
    {
      newOp.operation = opOr;

      err = ResolveOperand (expression->expressionValue.left, newOp.left);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.left.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.left.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (newOp.left.type != operandOperation)
      {
        /* Convert to "(left eq true) or (right eq true)" to make the possible 
         * value determination simpler. */
        CondOperation newOpL;
        newOpL.operation = opEqual;
        newOpL.left = newOp.left;
        newOpL.right.type = operandBoolean;
        newOpL.right.boolVal = true;

        newOp.left.type = operandOperation;
        newOp.left.operation = FindOptimizedCondition (newOpL);
      }

      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of '%s' is '%s', not compatible to '%s'",
	  csExpressionToken::Extractor (t).Get (), OperandTypeDescription (newOp.right.type),
	  OperandTypeDescription (operandBoolean));
      }
      if (newOp.right.type != operandOperation)
      {
        /* Convert to "(left eq true) or (right eq true)" to make the possible 
         * value determination simpler. */
        CondOperation newOpR;
        newOpR.operation = opEqual;
        newOpR.left = newOp.right;
        newOpR.right.type = operandBoolean;
        newOpR.right.boolVal = true;

        newOp.right.type = operandOperation;
        newOp.right.operation = FindOptimizedCondition (newOpR);
      }
    }
    else
    {
      return SetLastError ("Unknown operator '%s'", 
	csExpressionToken::Extractor (t).Get ());
    }
  }

  operation = newOp;
  return 0;
}

bool csConditionEvaluator::IsConditionPartOf (csConditionID condition, 
                                              csConditionID containerCondition)
{
  if (condition == containerCondition) return true;

  const CondOperation* op = conditions.GetKeyPointer (containerCondition);
  CS_ASSERT (op != 0);

  if (op->left.type == operandOperation)
  {
    if (IsConditionPartOf (condition, op->left.operation)) return true;
  }
  if (op->right.type == operandOperation)
  {
    if (IsConditionPartOf (condition, op->right.operation)) return true;
  }
  return false;
}

bool csConditionEvaluator::Evaluate (csConditionID condition, 
				     const csRenderMeshModes& modes,
				     const iShaderVarStack* stacks)
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

  EvaluatorShadervar eval (*this, modes, stacks);
  bool result = Evaluate<EvaluatorShadervar> (eval, condition);

  condChecked.Set (condition, true);
  condResult.Set (condition, result);

  return result;
}

csConditionEvaluator::EvaluatorShadervar::BoolType 
csConditionEvaluator::EvaluatorShadervar::Boolean (
  const CondOperand& operand)
{
  switch (operand.type)
  {
    case operandOperation:
      return evaluator.Evaluate<EvaluatorShadervar> (*this, operand.operation);
    case operandBoolean:
      return operand.boolVal;
    case operandSV:
      {
        if (stacks->GetSize() > operand.svName)
	{
	  csShaderVariable* sv = stacks->Get (operand.svName);
	  return sv != 0;
	}
      }
      break;
    case operandSVValueTexture:
      {
	if (stacks->GetSize() > operand.svName)
	{
	  csShaderVariable* sv = stacks->Get (operand.svName);
	  if (sv != 0)
	  {
	    iTextureHandle* th;
	    if (sv->GetValue (th))
	      return th != 0;
	  }
	}
      }
      break;
    case operandSVValueBuffer:
      //@@TODO: CHECK FOR DEFAULTBUFFERS
      {
	if (stacks->GetSize() > operand.svName)
	{
	  csShaderVariable* sv = stacks->Get (operand.svName);
	  if (sv != 0)
	  {
	    iRenderBuffer* th;
	    if (sv->GetValue (th))
	      return th != 0;
	  }
	}
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
	if (stacks->GetSize() > operand.svName)
	{
	    csShaderVariable* sv = stacks->Get (operand.svName);
	    if (sv != 0)
	    {
	      float v;
	      if (sv->GetValue (v))
		return (int)v;
	    }
	}
      }
      break;
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
      {
	if (stacks->GetSize() > operand.svName)
	{
	    csShaderVariable* sv = stacks->Get (operand.svName);
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
      }
      break;
    case operandSVValueInt:
      {
	if (stacks->GetSize() > operand.svName)
	{
	  csShaderVariable* sv = stacks->Get (operand.svName);
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return v;
	  }
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
	if (stacks->GetSize() > operand.svName)
	{
	  csShaderVariable* sv = stacks->Get (operand.svName);
	  if (sv != 0)
	  {
	    float v;
	    if (sv->GetValue (v))
	      return v;
	  }
	}
      }
      break;
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
      {
	if (stacks->GetSize() > operand.svName)
	{
	    csShaderVariable* sv = stacks->Get (operand.svName);
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
      }
      break;
    case operandSVValueInt:
      {
	if (stacks->GetSize() > operand.svName)
	{
          csShaderVariable* sv = stacks->Get (operand.svName);
	  if (sv != 0)
	  {
	    int v;
	    if (sv->GetValue (v))
	      return (float)v;
	  }
	}
      }
      break;
    default:
      ;
  }

  return 0.0f;
}

template<typename Evaluator>
typename_qualifier Evaluator::EvalResult csConditionEvaluator::Evaluate (
  Evaluator& eval, csConditionID condition)
{
  typedef typename_qualifier Evaluator::EvalResult EvResult;
  typedef typename_qualifier Evaluator::BoolType EvBool;
  typedef typename_qualifier Evaluator::FloatType EvFloat;
  typedef typename_qualifier Evaluator::IntType EvInt;
  EvResult result (eval.GetDefaultResult());

  const CondOperation* op = conditions.GetKeyPointer (condition);
  CS_ASSERT (op != 0);

  switch (op->operation)
  {
    case opAnd:
      result = eval.LogicAnd (op->left, op->right);
      break;
    case opOr:
      result = eval.LogicOr (op->left, op->right);
      break;
    case opEqual:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->left.type == operandSVValueX)
	  || (op->left.type == operandSVValueY)
	  || (op->left.type == operandSVValueZ)
	  || (op->left.type == operandSVValueW)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueX) 
	  || (op->right.type == operandSVValueY) 
	  || (op->right.type == operandSVValueZ) 
	  || (op->right.type == operandSVValueW) 
	  || (op->right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op->left));
	  EvFloat f2 (eval.Float (op->right));
          result = f1 == f2;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  EvBool b1 (eval.Boolean (op->left));
	  EvBool b2 (eval.Boolean (op->right));
	  result = b1 == b2;
	}
	else
	{
	  EvInt i1 (eval.Int (op->left));
	  EvInt i2 (eval.Int (op->right));
	  result = i1 == i2;
	}
	break;
      }
    case opNEqual:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->left.type == operandSVValueX)
	  || (op->left.type == operandSVValueY)
	  || (op->left.type == operandSVValueZ)
	  || (op->left.type == operandSVValueW)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueX) 
	  || (op->right.type == operandSVValueY) 
	  || (op->right.type == operandSVValueZ) 
	  || (op->right.type == operandSVValueW) 
	  || (op->right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op->left));
	  EvFloat f2 (eval.Float (op->right));
          result = f1 != f2;
	}
	else if (OpTypesCompatible (op->left.type, operandBoolean) 
	  && OpTypesCompatible (op->right.type, operandBoolean))
	{
	  EvBool b1 (eval.Boolean (op->left));
	  EvBool b2 (eval.Boolean (op->right));
	  result = b1 != b2;
	}
	else
	{
	  EvInt i1 (eval.Int (op->left));
	  EvInt i2 (eval.Int (op->right));
	  result = i1 != i2;
	}
	break;
      }
    case opLesser:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->left.type == operandSVValueX)
	  || (op->left.type == operandSVValueY)
	  || (op->left.type == operandSVValueZ)
	  || (op->left.type == operandSVValueW)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueX) 
	  || (op->right.type == operandSVValueY) 
	  || (op->right.type == operandSVValueZ) 
	  || (op->right.type == operandSVValueW) 
	  || (op->right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op->left));
	  EvFloat f2 (eval.Float (op->right));
	  result = (f1 < f2);
	}
	else
	{
	  EvInt i1 (eval.Int (op->left));
	  EvInt i2 (eval.Int (op->right));
	  result = i1 < i2;
	}
	break;
      }
    case opLesserEq:
      {
	if ((op->left.type == operandFloat) 
	  || (op->left.type == operandSVValueFloat)
	  || (op->left.type == operandSVValueX)
	  || (op->left.type == operandSVValueY)
	  || (op->left.type == operandSVValueZ)
	  || (op->left.type == operandSVValueW)
	  || (op->right.type == operandFloat) 
	  || (op->right.type == operandSVValueX) 
	  || (op->right.type == operandSVValueY) 
	  || (op->right.type == operandSVValueZ) 
	  || (op->right.type == operandSVValueW) 
	  || (op->right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op->left));
	  EvFloat f2 (eval.Float (op->right));
	  result = (f1 <= f2);
	}
	else
	{
	  EvInt i1 (eval.Int (op->left));
	  EvInt i2 (eval.Int (op->right));
	  result = i1 <= i2;
	}
	break;
      }
    default:
      CS_ASSERT (false);
  }

  return result;
}

void csConditionEvaluator::ResetEvaluationCache()
{
  condChecked.SetLength (GetNumConditions ());
  condChecked.Clear();
  condResult.SetLength (GetNumConditions ());
}

bool csConditionEvaluator::EvaluateConst (const CondOperation& op, bool& result)
{
  bool rB1, rB2;
  int rI1, rI2;
  float rF1, rF2;
  switch (op.operation)
  {
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

void csConditionEvaluator::GetUsedSVs2 (csConditionID condition, 
                                        MyBitArrayTemp& affectedSVs)
{
  const CondOperation* op = conditions.GetKeyPointer (condition);
  CS_ASSERT (op != 0);

  if (op->left.type == operandOperation)
    GetUsedSVs2 (op->left.operation, affectedSVs);
  else if ((op->left.type >= operandSV) 
    && (op->left.type <= operandSVValueBuffer))
  {
    if (affectedSVs.GetSize() <= op->left.svName) affectedSVs.SetSize (op->left.svName+1);
    affectedSVs.SetBit (op->left.svName);
  }

  if (op->right.type == operandOperation)
    GetUsedSVs2 (op->right.operation, affectedSVs);
  else if ((op->right.type >= operandSV) 
    && (op->right.type <= operandSVValueBuffer))
  {
    if (affectedSVs.GetSize() <= op->right.svName) affectedSVs.SetSize (op->right.svName+1);
    affectedSVs.SetBit (op->right.svName);
  }
}

void csConditionEvaluator::GetUsedSVs (csConditionID condition, 
                                       MyBitArrayTemp& affectedSVs)
{
  affectedSVs.Clear();
  if ((condition == csCondAlwaysFalse) || (condition == csCondAlwaysTrue))
    return;
  GetUsedSVs2 (condition, affectedSVs);
}

void csConditionEvaluator::CompactMemory ()
{
  Variables::Values::CompactAllocator();
  SliceAllocator::CompactAllocator();
  MyBitArrayAllocatorTemp::CompactAllocators();
}

struct ValueSetWrapper
{
  const ValueSet* startVals;

  ValueSetWrapper (const ValueSet& startVals) : startVals (&startVals) {}
  ValueSetWrapper (float f);

  // @@@ FIXME: probably cleaner to not (ab)use operators...
  friend Logic3 operator== (ValueSetWrapper& a, ValueSetWrapper& b)
  {
    if ((a.startVals->IsSingleValue() && b.startVals->IsSingleValue())
      && (a.startVals->GetSingleValue() == b.startVals->GetSingleValue()))
    {
      return Logic3::Truth;
    }
    else
    {
      if (a.startVals->Overlaps (*b.startVals))
      {
        return Logic3::Uncertain;
      }
      else
      {
        return Logic3::Lie;
      }
    }
  }
  friend Logic3 operator!= (ValueSetWrapper& a, ValueSetWrapper& b)
  {
    Logic3 r = !operator== (a, b);
    return r;
  }
  friend Logic3 operator< (ValueSetWrapper& a, ValueSetWrapper& b)
  {
    ValueSet::Interval::Side aMax = a.startVals->GetMax ();
    ValueSet::Interval::Side bMin = b.startVals->GetMin ();
    if (!a.startVals->Overlaps (*b.startVals))
    {
      if (aMax < bMin)
      {
        return Logic3::Truth;
      }
      else
      {
        return Logic3::Lie;
      }
    }
    else
    {
      ValueSet::Interval::Side aMin = a.startVals->GetMin ();
      ValueSet::Interval::Side bMax = b.startVals->GetMax ();
      if (aMin >= bMax)
      {
        return Logic3::Lie;
      }

      return Logic3::Uncertain;
    }
  }
  friend Logic3 operator<= (ValueSetWrapper& a, ValueSetWrapper& b)
  {
    ValueSet::Interval::Side aMax = a.startVals->GetMax ();
    ValueSet::Interval::Side bMin = b.startVals->GetMin ();
    if (!a.startVals->Overlaps (*b.startVals))
    {
      if (aMax <= bMin)
      {
        return Logic3::Truth;
      }
      else
      {
        return Logic3::Lie;
      }
    }
    else
    {
      return Logic3::Uncertain;
    }
  }

  operator Logic3 () const
  {
    ValueSet falseSet (0.0f);
    ValueSet trueSet (1.0f);
    bool canTrue = startVals->Overlaps (trueSet);
    bool canFalse = startVals->Overlaps (falseSet);
    if (canTrue && !canFalse)
      return Logic3::Truth;
    else if (!canTrue && canFalse)
      return Logic3::Lie;
    else
      return Logic3::Uncertain;
  }
};

struct EvaluatorShadervarValuesSimple
{
  typedef Logic3 EvalResult;
  typedef ValueSetWrapper BoolType;
  typedef ValueSetWrapper FloatType;
  typedef ValueSetWrapper IntType;

  EvalResult GetDefaultResult() const
  { 
    return Logic3::Uncertain;
  }

  csConditionEvaluator& evaluator;
  const Variables& vars; 
  ValueSet boolMask;

  EvaluatorShadervarValuesSimple (csConditionEvaluator& evaluator, 
    const Variables& vars) : evaluator (evaluator), vars (vars), 
    boolMask (ValueSet (0.0f) | ValueSet (1.0f)),
    createdValues (SliceAllocator::valueSetsPerSlice) {}

  BoolType Boolean (const CondOperand& operand);
  IntType Int (const CondOperand& operand)
  { return Float (operand); }
  FloatType Float (const CondOperand& operand);

  EvalResult LogicAnd (const CondOperand& a, const CondOperand& b);
  EvalResult LogicOr (const CondOperand& a, const CondOperand& b);

  csBlockAllocator<ValueSet, SliceAllocator> createdValues;
protected:
  ValueSet& CreateValue ()
  {
    ValueSet* vs = createdValues.Alloc();
    return *vs;
  }
  ValueSet& CreateValue (float f)
  {
    ValueSet* vs = createdValues.Alloc();
    *vs = f;
    return *vs;
  }
};

Logic3 csConditionEvaluator::CheckConditionResults (
  csConditionID condition, const Variables& vars)
{
  EvaluatorShadervarValuesSimple eval (*this, vars);
  return Evaluate (eval, condition);
}

EvaluatorShadervarValuesSimple::BoolType EvaluatorShadervarValuesSimple::Boolean (
  const CondOperand& operand)
{
  switch (operand.type)
  {
    case operandBoolean:
      {
        ValueSet& vs = CreateValue();
        vs = operand.boolVal ? 1.0f : 0.0f;
        return ValueSetWrapper (vs);
      }
    case operandSV:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetVar() & boolMask;
        return ValueSetWrapper (vs);
      }
    case operandSVValueTexture:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetTex() & boolMask;
        return ValueSetWrapper (vs);
      }
    case operandSVValueBuffer:
      //@@TODO: CHECK FOR DEFAULTBUFFERS
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetBuf() & boolMask;
        return ValueSetWrapper (vs);
      }
    default:
      {
        CS_ASSERT_MSG("Bug: Unexpected operand type", false);
      }
  }

  return ValueSetWrapper (boolMask);
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
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        return ValueSetWrapper (startValues->GetVec (0));
      }
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
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
  rA = evaluator.CheckConditionResults (a.operation,
    vars);

  Logic3 rB;
  CS_ASSERT (b.type == operandOperation);
  rB = evaluator.CheckConditionResults (b.operation,
    vars);
  return rA && rB;
}

EvaluatorShadervarValuesSimple::EvalResult EvaluatorShadervarValuesSimple::LogicOr (
  const CondOperand& a, const CondOperand& b)
{
  Logic3 rA;
  CS_ASSERT (a.type == operandOperation);
  rA = evaluator.CheckConditionResults (a.operation,
    vars);

  Logic3 rB;
  CS_ASSERT (b.type == operandOperation);
  rB = evaluator.CheckConditionResults (b.operation,
    vars);
  return rA || rB;
}

/**
 * This struct contains references to 2 value sets.
 * Used by csConditionEvaluator::CheckConditionResults() which returns the
 * possible values for variables for both the cases that the condition is
 * true or false.
 *
 * These variable values are actually constructed by operating with
 * JanusValueSet - operations will adjust the _operands_ to contain the
 * the values in case that operation returns true or false.
 */
struct JanusValueSet
{
  const ValueSet* startVals;
  ValueSet* trueVals;
  ValueSet* falseVals;

  JanusValueSet (const ValueSet& startVals, ValueSet& trueVals, 
    ValueSet& falseVals) : startVals (&startVals), trueVals (&trueVals), 
    falseVals (&falseVals) {}

  // @@@ FIXME: probably cleaner to not (ab)use operators...
  friend Logic3 operator== (JanusValueSet& a, JanusValueSet& b)
  {
    if ((a.startVals->IsSingleValue() && b.startVals->IsSingleValue())
      && (a.startVals->GetSingleValue() == b.startVals->GetSingleValue()))
    {
      *a.trueVals = *b.trueVals = *a.startVals;
      *a.falseVals = *b.falseVals = !*a.startVals;
      return Logic3::Truth;
    }
    else
    {
      if (a.startVals->Overlaps (*b.startVals))
      {
        // Make the "true" values those that both share.
        ValueSet overlap = *a.startVals & *b.startVals;
        *a.trueVals = *b.trueVals = overlap;
        // Take the overlap out of the "false" values.
        *a.falseVals = *a.startVals & !overlap;
        *b.falseVals = *b.startVals & !overlap;
        return Logic3::Uncertain;
      }
      else
      {
        *a.trueVals = *a.falseVals = *a.startVals;
        *b.trueVals = *b.falseVals = *b.startVals;
        return Logic3::Lie;
      }
    }
  }
  friend Logic3 operator!= (JanusValueSet& a, JanusValueSet& b)
  {
    Logic3 r = !operator== (a, b);
    ValueSet t1 = *a.trueVals;
    ValueSet t2 = *b.trueVals;
    ValueSet f1 = *a.falseVals;
    ValueSet f2 = *b.falseVals;
    /* To make != true, we need the ranges that makes == false
     * and vice versa */
    *a.trueVals = f1;
    *b.trueVals = f2;
    *a.falseVals = t1;
    *b.falseVals = t2;
    return r;
  }
  friend Logic3 operator< (JanusValueSet& a, JanusValueSet& b)
  {
    ValueSet::Interval::Side aMax = a.startVals->GetMax ();
    ValueSet::Interval::Side bMin = b.startVals->GetMin ();
    if (!a.startVals->Overlaps (*b.startVals))
    {
      /* The two value sets don't overlap.
       * We can just take the maximum/minimum values of the sets and
       * compare those to find whether a value from a can be smaller
       * than one from b.
       * In either case, the ranges can stay as they are.
       */
      *a.trueVals = *a.falseVals = *a.startVals;
      *b.trueVals = *b.falseVals = *b.startVals;
      if (aMax < bMin)
      {
        return Logic3::Truth;
      }
      else
      {
        return Logic3::Lie;
      }
    }
    else
    {
      ValueSet::Interval::Side aMin = a.startVals->GetMin ();
      ValueSet::Interval::Side bMax = b.startVals->GetMax ();

      if (aMin >= bMax)
      {
        return Logic3::Lie;
      }

      ValueSet::Interval intvA (
        ValueSet::Interval::Side (aMin.GetValue(), false),
        ValueSet::Interval::Side (false, false));
      ValueSet::Interval intvB (
        ValueSet::Interval::Side (true, false),
        ValueSet::Interval::Side (bMax.GetValue(), false));

      ValueSet setA (intvA);
      ValueSet setB (intvB);

      *a.trueVals = *a.startVals & setB;
      *a.falseVals = *a.startVals & !setB;
      *b.trueVals = *b.startVals & setA;
      *b.falseVals = *b.startVals & !setA;

      return Logic3::Uncertain;
    }
  }
  friend Logic3 operator<= (JanusValueSet& a, JanusValueSet& b)
  {
    ValueSet::Interval::Side aMax = a.startVals->GetMax ();
    ValueSet::Interval::Side bMin = b.startVals->GetMin ();
    if (!a.startVals->Overlaps (*b.startVals))
    {
      /* The two value sets don't overlap.
       * We can just take the maximum/minimum values of the sets and
       * compare those to find whether a value from a can be smaller
       * than one from b.
       * In either case, the ranges can stay as they are.
       */
      *a.trueVals = *a.falseVals = *a.startVals;
      *b.trueVals = *b.falseVals = *b.startVals;
      if (aMax <= bMin)
      {
        return Logic3::Truth;
      }
      else
      {
        return Logic3::Lie;
      }
    }
    else
    {
      ValueSet::Interval intvA (
        ValueSet::Interval::Side (a.startVals->GetMin ().GetValue(), true),
        ValueSet::Interval::Side (false, true));
      ValueSet::Interval intvB (
        ValueSet::Interval::Side (true, true),
        ValueSet::Interval::Side (b.startVals->GetMax ().GetValue(), true));

      ValueSet setA (intvA);
      ValueSet setB (intvB);

      *a.trueVals = *a.startVals & setB;
      *a.falseVals = *a.startVals & !setB;
      *b.trueVals = *b.startVals & setA;
      *b.falseVals = *b.startVals & !setA;

      return Logic3::Uncertain;
    }
  }

  operator Logic3 () const
  {
    ValueSet falseSet (0.0f);
    ValueSet trueSet (1.0f);
    bool canTrue = startVals->Overlaps (trueSet);
    bool canFalse = startVals->Overlaps (falseSet);
    if (canTrue && !canFalse)
      return Logic3::Truth;
    else if (!canTrue && canFalse)
      return Logic3::Lie;
    else
      return Logic3::Uncertain;
  }
};

struct EvaluatorShadervarValues
{
  typedef Logic3 EvalResult;
  typedef JanusValueSet BoolType;
  typedef JanusValueSet FloatType;
  typedef JanusValueSet IntType;

  EvalResult GetDefaultResult() const
  { 
    return Logic3::Uncertain;
  }

  csConditionEvaluator& evaluator;
  const Variables& vars; 
  Variables& trueVars; 
  Variables& falseVars;
  ValueSet boolMask;

  EvaluatorShadervarValues (csConditionEvaluator& evaluator, const Variables& vars,
    Variables& trueVars, Variables& falseVars) : evaluator (evaluator), 
    vars (vars), trueVars (trueVars), falseVars (falseVars),
    boolMask (ValueSet (0.0f) | ValueSet (1.0f)),
    createdValues (SliceAllocator::valueSetsPerSlice) {}

  BoolType Boolean (const CondOperand& operand);
  IntType Int (const CondOperand& operand)
  { return Float (operand); }
  FloatType Float (const CondOperand& operand);

  EvalResult LogicAnd (const CondOperand& a, const CondOperand& b);
  EvalResult LogicOr (const CondOperand& a, const CondOperand& b);

  csBlockAllocator<ValueSet/*, BlockAllocatorSlicePolicy*/> createdValues;
protected:
  ValueSet& CreateValue ()
  {
    ValueSet* vs = createdValues.Alloc();
    return *vs;
  }
  ValueSet& CreateValue (float f)
  {
    ValueSet* vs = createdValues.Alloc();
    *vs = f;
    return *vs;
  }
};

Logic3 csConditionEvaluator::CheckConditionResults (
  csConditionID condition, const Variables& vars, 
  Variables& trueVars, Variables& falseVars)
{
  trueVars = falseVars = vars;
  EvaluatorShadervarValues eval (*this, vars, trueVars, falseVars);
  return Evaluate (eval, condition);
}

EvaluatorShadervarValues::BoolType EvaluatorShadervarValues::Boolean (
  const CondOperand& operand)
{
  switch (operand.type)
  {
    case operandBoolean:
      {
        ValueSet& vs = CreateValue();
        vs = operand.boolVal ? 1.0f : 0.0f;
        ValueSet& vsTrue = CreateValue();
        ValueSet& vsFalse = CreateValue();
        return JanusValueSet (vs, vsTrue, vsFalse);
      }
    case operandSV:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetVar() & boolMask;
        Variables::Values* valuesTrue = trueVars.GetValues (operand.svName);
        Variables::Values* valuesFalse = falseVars.GetValues (operand.svName);
        return JanusValueSet (vs, valuesTrue->GetVar(), valuesFalse->GetVar());
      }
    case operandSVValueTexture:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetTex() & boolMask;
        Variables::Values* valuesTrue = trueVars.GetValues (operand.svName);
        Variables::Values* valuesFalse = falseVars.GetValues (operand.svName);
        return JanusValueSet (vs, valuesTrue->GetTex(), valuesFalse->GetTex());
      }
    case operandSVValueBuffer:
      //@@TODO: CHECK FOR DEFAULTBUFFERS
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        ValueSet& vs = CreateValue();
        vs = startValues->GetBuf() & boolMask;
        Variables::Values* valuesTrue = trueVars.GetValues (operand.svName);
        Variables::Values* valuesFalse = falseVars.GetValues (operand.svName);
        return JanusValueSet (vs, valuesTrue->GetBuf(), valuesFalse->GetBuf());
      }
    default:
      {
        CS_ASSERT_MSG("Bug: Unexpected operand type", false);
      }
  }

  ValueSet& vsTrue = CreateValue();
  ValueSet& vsFalse = CreateValue();
  return JanusValueSet (boolMask, vsTrue, vsFalse);
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
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        Variables::Values* valuesTrue = trueVars.GetValues (operand.svName);
        Variables::Values* valuesFalse = falseVars.GetValues (operand.svName);
        return JanusValueSet (startValues->GetVec (0), valuesTrue->GetVec (0), valuesFalse->GetVec (0));
      }
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
      {
        const Variables::Values* startValues = vars.GetValues (operand.svName);
        Variables::Values* valuesTrue = trueVars.GetValues (operand.svName);
        Variables::Values* valuesFalse = falseVars.GetValues (operand.svName);
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
  rA = evaluator.CheckConditionResults (a.operation,
    vars, trueVarsA, falseVarsA);

  Logic3 rB;
  CS_ASSERT (b.type == operandOperation);
  if (rA.state == Logic3::Truth)
  {
    rB = evaluator.CheckConditionResults (b.operation,
      trueVarsA, trueVars, falseVars);
  }
  else if (rA.state == Logic3::Lie)
  {
    rB = evaluator.CheckConditionResults (b.operation,
      falseVarsA, trueVars, falseVars);
  }
  else
  {
    Logic3 rAT;
    Variables trueVarsAT;
    Variables falseVarsAT;
    rAT = evaluator.CheckConditionResults (b.operation,
      trueVarsA, trueVarsAT, falseVarsAT);

    Logic3 rAF;
    Variables trueVarsAF;
    Variables falseVarsAF;
    rAF = evaluator.CheckConditionResults (b.operation,
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
  rA = evaluator.CheckConditionResults (a.operation,
    vars, trueVarsA, falseVarsA);

  Logic3 rB;
  Variables trueVarsB; 
  Variables falseVarsB;
  CS_ASSERT (b.type == operandOperation);
  if (rA.state == Logic3::Truth)
  {
    rB = evaluator.CheckConditionResults (b.operation,
      trueVarsA, trueVars, falseVars);
  }
  else if (rA.state == Logic3::Lie)
  {
    rB = evaluator.CheckConditionResults (b.operation,
      falseVarsA, trueVars, falseVars);
  }
  else
  {
    Logic3 rAT;
    Variables trueVarsAT;
    Variables falseVarsAT;
    rAT = evaluator.CheckConditionResults (b.operation,
      trueVarsA, trueVarsAT, falseVarsAT);

    Logic3 rAF;
    Variables trueVarsAF;
    Variables falseVarsAF;
    rAF = evaluator.CheckConditionResults (b.operation,
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
