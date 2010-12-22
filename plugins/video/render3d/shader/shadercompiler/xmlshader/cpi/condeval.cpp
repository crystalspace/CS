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

#include "csgfx/renderbuffer.h"
#include "csutil/scanstr.h"
#include "csutil/stringquote.h"

#include "condeval.h"
#include "condeval_eval_sv.h"
#include "condeval_eval_svsimple.h"
#include "condeval_eval_svvalues.h"
#include "tokenhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

using namespace CS;

//---------------------------------------------------------------------------

CS_IMPLEMENT_STATIC_CLASSVAR_REF(SliceAllocator, sliceAlloc,
  SliceAlloc, SliceAllocator::BlockAlloc, (32));

CS_IMPLEMENT_STATIC_CLASSVAR_REF(SliceAllocatorBool, sliceAlloc,
  SliceAllocBool, SliceAllocatorBool::BlockAlloc, (32));

//---------------------------------------------------------------------------

csConditionID ConditionIDMapper::GetConditionID (const CondOperation& operation,
                                                 bool get_new)
{
  csConditionID id = conditions.Get (operation, (csConditionID)~0);
  if ((id == (csConditionID)~0) && get_new)
  {
    id = nextConditionID++;
    conditions.Put (operation, id);
  }
  return id;
}

CondOperation ConditionIDMapper::GetCondition (csConditionID condition)
{
  const CondOperation* op = conditions.GetKeyPointer (condition);
  CS_ASSERT(op != 0);
  return *op;
}

//---------------------------------------------------------------------------

template<typename T>
static void SetSizeFill1 (T& array, size_t newSize)
{
  size_t oldSize = array.GetSize ();
  array.SetSize (newSize);
  if (newSize > oldSize)
  {
    for (size_t b = oldSize; b < newSize; b++)
      array.SetBit (b);
  }
}
  
//---------------------------------------------------------------------------

csConditionEvaluator::csConditionEvaluator (iShaderVarStringSet* strings,
  const csConditionConstants& constants) :
  strings(strings), ticketEvalPool (0),
  evalStatePool (0), constants(constants)
{
}

csConditionEvaluator::~csConditionEvaluator ()
{
  while (ticketEvalPool != 0)
  {
    TicketEvaluator* p = ticketEvalPool;
    ticketEvalPool = p->poolNext;
    cs_free (p);
  }
  while (evalStatePool != 0)
  {
    EvalState* p = evalStatePool;
    evalStatePool = p->poolNext;
    cs_free (p);
  }
}

size_t* csConditionEvaluator::AllocSVIndices (size_t num)
{
  if (num == 0) return 0;
  
  LockType lock (mutex);
  size_t* mem = (size_t*)scratch.Alloc ((num+1) * sizeof (size_t));
  return mem;
}

size_t* csConditionEvaluator::AllocSVIndicesInternal (
  const CS::Graphics::ShaderVarNameParser& parser)
{
  const size_t num = parser.GetIndexNum();
  if (num == 0) return 0;

  size_t* mem;
  mem = (size_t*)scratch.Alloc ((num+1) * sizeof (size_t));
  size_t* p = mem;
  
  *p++ = num;
  for (size_t n = 0; n < num; n++)
    *p++ = parser.GetIndexValue (n);
  return mem;
}

size_t* csConditionEvaluator::AllocSVIndicesInternal (size_t num)
{
  if (num == 0) return 0;
  
  size_t* mem = (size_t*)scratch.Alloc ((num+1) * sizeof (size_t));
  return mem;
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

csConditionID csConditionEvaluator::FindOptimizedConditionInternal (
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
    newOp.right.boolVal = newOp.right.operation == csCondAlwaysTrue;
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
	return FindOptimizedConditionInternal (newNewOp);
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
	return FindOptimizedConditionInternal (newNewOp);
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
	return FindOptimizedConditionInternal (newNewOp);
      }
    }
  }

  csConditionID condID = conditions.GetConditionID (operation, false);
  if (condID == (csConditionID)~0)
  {
    condID = conditions.GetConditionID (operation, true);
    MarkAffectionBySVs (condID, operation.left);
    MarkAffectionBySVs (condID, operation.right);
  }
  return condID;
}

csConditionID csConditionEvaluator::FindOptimizedCondition (
  const CondOperation& operation)
{
  LockType lock (mutex);
  return FindOptimizedConditionInternal (operation);
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
      if (csScanStr (number, "%f%c", &operand.floatVal, &dummy) != 1)
      {
	return SetLastError ("Malformed float value: %s",
	  CS::Quote::Single (number.GetData()));
      }
      operand.type = operandFloat;
    }
    else
    {
      char dummy;
      if (sscanf (number, "%d%c", &operand.intVal, &dummy) != 1)
      {
	return SetLastError ("Malformed int value: %s",
	  CS::Quote::Single (number.GetData()));
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
      return SetLastError ("Unknown identifier %s",
	CS::Quote::Single (csExpressionToken::Extractor (value).Get ()));
    }
  }
  else
  {
    return SetLastError ("Value of %s of type %s",
      CS::Quote::Single (csExpressionToken::Extractor (value).Get ()),
      CS::Quote::Single (csExpressionToken::TypeDescription (value.type)));
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
      return SetLastError ("Can't resolve value %s: %s",
	CS::Quote::Single (csExpressionToken::Extractor (expression->valueValue).Get ()),
	err);
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
      return SetLastError ("Unknown identifier %s",
	CS::Quote::Single (csExpressionToken::Extractor (left).Get ()));
    }
  }
  else
  {
    operand.type = operandOperation;
    err = ProcessExpressionInternal (expression, operand.operation);
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
    csExpressionToken::Extractor svIdentifier (expression->valueValue);
    CS::Graphics::ShaderVarNameParser svParser (svIdentifier.Get());
    
    operand.type = operandSV;
    operand.svLocation.svName = strings->Request (svParser.GetShaderVarName());
    operand.svLocation.indices = AllocSVIndicesInternal (svParser);
    return 0;
  }
  else
  {
    const csExpressionToken& t = expression->expressionValue.op;
    if (!TokenEquals (t.tokenStart, t.tokenLen, "."))
    {
      return SetLastError ("Unexpected operator %s",
	CS::Quote::Single (csExpressionToken::Extractor (t).Get ()));
    }
    if (expression->expressionValue.left->type != csExpression::Value)
    {
      CS_ASSERT_MSG ("It should not happen that the 'left' subexpression "
	"is not a value", false);
      return SetLastError ("Left subexpression is not of type %s",
			   CS::Quote::Single ("value"));
    }
    if (expression->expressionValue.right->type != csExpression::Value)
    {
      /* @@@ Happens for stuff like vars.foo.bar.baz, where bar.baz
       * will incarnate here as a right expression of type Expression.
       * Clearer error msg?
       */
      return SetLastError ("Right subexpression is not of type %s",
			   CS::Quote::Single ("value"));
    }
    {
      csExpressionToken::Extractor svIdentifier (expression->expressionValue.left->valueValue);
      CS::Graphics::ShaderVarNameParser svParser (svIdentifier.Get());
      
      operand.type = operandSV;
      operand.svLocation.svName = strings->Request (svParser.GetShaderVarName());
      operand.svLocation.indices = AllocSVIndicesInternal (svParser);
      operand.svLocation.bufferName = csRenderBuffer::GetBufferNameFromDescr (
        svParser.GetShaderVarName());
    }
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
      return SetLastError ("Unknown shader variable specializer %s",
	CS::Quote::Single (csExpressionToken::Extractor (right).Get ()));
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
      SetLastError ("Unknown symbol %s", CS::Quote::Single (symbol.Get()));
    }
    operand = *constOp;
    return 0;
  }
  else
    return "Expression is not a value";
}


const char* csConditionEvaluator::ProcessExpressionInternal (
  csExpression* expression, csConditionID& cond)
{
  CondOperation newOp;
  const char* err = ProcessExpressionInternal (expression, newOp);
  if (err) return err;
  cond = FindOptimizedConditionInternal (newOp);
  return 0;
}

const char* csConditionEvaluator::ProcessExpression (
  csExpression* expression, csConditionID& cond)
{
  LockType lock (mutex);
  return ProcessExpressionInternal (expression, cond);
}

const char* csConditionEvaluator::ProcessExpressionInternal (csExpression* expression, 
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
      return SetLastError ("Can't resolve value %s: %s",
	CS::Quote::Single (csExpressionToken::Extractor (expression->valueValue).Get ()),
	err);
    }
    newOp.right.type = operandBoolean;
    newOp.right.boolVal = true;
    if (!OpTypesCompatible (newOp.left.type, newOp.right.type))
    {
      return SetLastError ("Type of %s is %s, not compatible to %s",
	CS::Quote::Single (csExpressionToken::Extractor (expression->valueValue).Get ()),
	CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
      newOp.right.Clear();
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (operandBoolean)));
      }
      newOp.right.Clear();
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)));
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (operandBoolean)));
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

        newOp.left.Clear();
        newOp.left.type = operandOperation;
        newOp.left.operation = FindOptimizedConditionInternal (newOpL);
      }

      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)),
	  CS::Quote::Single (OperandTypeDescription (operandBoolean)));
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

        newOp.right.Clear();
        newOp.right.type = operandOperation;
        newOp.right.operation = FindOptimizedConditionInternal (newOpR);
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
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.left.type)),
	  CS::Quote::Single (OperandTypeDescription (operandBoolean)));
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

        newOp.left.Clear();
        newOp.left.type = operandOperation;
        newOp.left.operation = FindOptimizedConditionInternal (newOpL);
      }

      err = ResolveOperand (expression->expressionValue.right, newOp.right);
      if (err)
	return err;
      if (!OpTypesCompatible (newOp.right.type, operandBoolean))
      {
	return SetLastError ("Type of %s is %s, not compatible to %s",
	  CS::Quote::Single (csExpressionToken::Extractor (t).Get ()),
	  CS::Quote::Single (OperandTypeDescription (newOp.right.type)),
	  CS::Quote::Single (OperandTypeDescription (operandBoolean)));
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

        newOp.right.Clear();
        newOp.right.type = operandOperation;
        newOp.right.operation = FindOptimizedConditionInternal (newOpR);
      }
    }
    else
    {
      return SetLastError ("Unknown operator %s", 
	CS::Quote::Single (csExpressionToken::Extractor (t).Get ()));
    }
  }

  operation = newOp;
  return 0;
}

const char* csConditionEvaluator::ProcessExpression (csExpression* expression, 
  CondOperation& operation)
{
  LockType lock (mutex);
  return ProcessExpressionInternal (expression, operation);
}

bool csConditionEvaluator::IsConditionPartOfInternal (csConditionID condition,
  csConditionID containerCondition)
{
  if (condition == containerCondition) return true;

  CondOperation op = conditions.GetCondition (containerCondition);

  if (op.left.type == operandOperation)
  {
    if (IsConditionPartOfInternal (condition, op.left.operation)) return true;
  }
  if (op.right.type == operandOperation)
  {
    if (IsConditionPartOfInternal (condition, op.right.operation)) return true;
  }
  return false;
}

bool csConditionEvaluator::IsConditionPartOf (csConditionID condition,
  csConditionID containerCondition)
{
  LockType lock (mutex);
  return IsConditionPartOfInternal (condition, containerCondition);
}

/* Default value for 'last SV' array.
   Use a unique pointer value instead of 0 as it can happen that an affecting
   SV gets added after the eval cache setup; it's "last" value at the next
   eval cache setup is 0 - which is a valid value for an SV in an SV stack and
   thus may result in incorrect use of a cached eval result. */
static const csShaderVariable definitelyUniqueSV;

void csConditionEvaluator::SetupEvalCacheInternal (const csShaderVariableStack* stack)
{
  if (stack != 0)
  {
    if (evalCache.lastShaderVars.GetSize() != svAffectedConditions.GetSize())
    {
      evalCache.lastShaderVars.DeleteAll();
      evalCache.lastShaderVars.SetSize (svAffectedConditions.GetSize(),
	&definitelyUniqueSV);
    }
	
    for (size_t i = 0; i < svAffectedConditions.GetSize(); i++)
    {
      SVAffection& affection = svAffectedConditions[i];
      csShaderVariable* sv = (affection.svName < stack->GetSize()) ?
	(*stack)[affection.svName] : 0;
      if (evalCache.lastShaderVars[i] != sv)
      {
	if (affection.affectedConditions.GetSize() != evalCache.condChecked.GetSize())
	{
	  /* Sometimes the affected conditions mask is bigger,
	   * sometimes the condition checked mask - the former can happen
	   * when a fallback shader or new tech is loaded that uses hitherto
	   * unknown conditions */
	  size_t newCondSize = csMax (affection.affectedConditions.GetSize(),
	    evalCache.condChecked.GetSize());
	  SetSizeFill1 (affection.affectedConditions, newCondSize);
	  evalCache.condChecked.SetSize (newCondSize);
	  evalCache.condResult.SetSize (newCondSize);
	}
	evalCache.condChecked &= affection.affectedConditions;
	evalCache.lastShaderVars[i] = sv;
      }
    }
    /* Checking against last buffer value is expensive, so just mask all
       conditions depending on buffer values */
    MyBitArrayMalloc& bitarray = bufferAffectConditions;
    if (bitarray.GetSize() != evalCache.condChecked.GetSize())
    {
      size_t newCondSize = csMax (bitarray.GetSize(),
	evalCache.condChecked.GetSize());
      SetSizeFill1 (bitarray, newCondSize);
      evalCache.condChecked.SetSize (newCondSize);
      evalCache.condResult.SetSize (newCondSize);
    }
    evalCache.condChecked &= bitarray;
  }
}


csPtr<csConditionEvaluator::TicketEvaluator> csConditionEvaluator::BeginTicketEvaluationCaching (
  const CS::Graphics::RenderMeshModes& modes,
  const csShaderVariableStack* stack)
{
  mutex.Lock();

  EvalState* evalState = &evalCache;
  EvaluatorShadervar eval (*this, evalState, &modes, stack);
  
  void* newp;
  if (ticketEvalPool != 0)
  {
    newp = ticketEvalPool;
    ticketEvalPool = ticketEvalPool->poolNext;
  }
  else
  {
    newp = cs_malloc (sizeof (TicketEvaluator));
  }
#include "csutil/custom_new_disable.h"
  TicketEvaluator* newEval = new (newp) TicketEvaluator (this, true, evalState, eval);
#include "csutil/custom_new_enable.h"
  {
    uint currentFrame = ~0;
    if (engine.IsValid()) currentFrame = engine->GetCurrentFrameNumber();
    if ((evalCache.lastEvalFrame != currentFrame)
      || (currentFrame == (uint)~0)) // be conservative in case we don't have an engine
    {
      evalCache.condChecked.SetSize (conditions.GetNumConditions ());
      evalCache.condChecked.Clear();
      evalCache.condResult.SetSize (conditions.GetNumConditions ());
      evalCache.lastShaderVars.DeleteAll();
      evalCache.lastEvalFrame = currentFrame;
    }
  }
  SetupEvalCacheInternal (stack);
  
  return csPtr<TicketEvaluator> (newEval);
}

csPtr<csConditionEvaluator::TicketEvaluator> csConditionEvaluator::BeginTicketEvaluation (
  const csBitArray& condSet,
  const csBitArray& condResults)
{
  EvalState* evalState;
  if (evalStatePool != 0)
  {
    evalState = evalStatePool;
    evalStatePool = evalStatePool->poolNext;
  }
  else
  {
#include "csutil/custom_new_disable.h"
    evalState = new (cs_malloc (sizeof (EvalState))) EvalState;
#include "csutil/custom_new_enable.h"
  }

  /* Hack: it can happen that while evaluating a shader, new conditions 
   * are added (notably when a shader source is retrieved from an
   * external source). Make sure the cache is large enough.
   */
  if (evalState->condChecked.GetSize() < conditions.GetNumConditions ())
  {
    evalState->condChecked.SetSize (conditions.GetNumConditions ());
    evalState->condResult.SetSize (conditions.GetNumConditions ());
  }
  evalState->condChecked.Clear();
  evalState->condResult.Clear();

  for (size_t i = 0; i < condResults.GetSize(); i++)
  {
    evalState->condChecked.Set (i, condSet[i]);
    evalState->condResult.Set (i, condResults[i]);
  }

  EvaluatorShadervar eval (*this, evalState, 0, 0);
  
  void* newp;
  if (ticketEvalPool != 0)
  {
    newp = ticketEvalPool;
    ticketEvalPool = ticketEvalPool->poolNext;
  }
  else
  {
    newp = cs_malloc (sizeof (TicketEvaluator));
  }
#include "csutil/custom_new_disable.h"
  TicketEvaluator* newEval = new (newp) TicketEvaluator (this, false, evalState, eval);
#include "csutil/custom_new_enable.h"

  return csPtr<TicketEvaluator> (newEval);
}

bool csConditionEvaluator::EvaluateCachedInternal (EvalState* evalState,
						   EvaluatorShadervar& eval,
						   csConditionID condition)
{
  if (condition == csCondAlwaysTrue)
    return true;
  else if (condition == csCondAlwaysFalse)
    return false;

  /* Hack: it can happen that while evaluating a shader, new conditions 
   * are added (notably when a shader source is retrieved from an
   * external source). Make sure the cache is large enough.
   */
  if (evalState->condChecked.GetSize() < conditions.GetNumConditions ())
  {
    evalState->condChecked.SetSize (conditions.GetNumConditions ());
    evalState->condResult.SetSize (conditions.GetNumConditions ());
  }

  if (evalState->condChecked.IsBitSet (condition))
  {
    return evalState->condResult.IsBitSet (condition);
  }

  bool result = EvaluateInternal (eval, condition);

  evalState->condChecked.Set (condition, true);
  evalState->condResult.Set (condition, result);

  return result;
}

template<typename Evaluator>
typename Evaluator::EvalResult csConditionEvaluator::EvaluateInternal (
  Evaluator& eval, csConditionID condition)
{
  typedef typename Evaluator::EvalResult EvResult;
  typedef typename Evaluator::BoolType EvBool;
  typedef typename Evaluator::FloatType EvFloat;
  typedef typename Evaluator::IntType EvInt;
  EvResult result (eval.GetDefaultResult());

  CondOperation op = conditions.GetCondition (condition);

  switch (op.operation)
  {
    case opAnd:
      result = eval.LogicAnd (op.left, op.right);
      break;
    case opOr:
      result = eval.LogicOr (op.left, op.right);
      break;
    case opEqual:
      {
	if ((op.left.type == operandFloat) 
	  || (op.left.type == operandSVValueFloat)
	  || (op.left.type == operandSVValueX)
	  || (op.left.type == operandSVValueY)
	  || (op.left.type == operandSVValueZ)
	  || (op.left.type == operandSVValueW)
	  || (op.right.type == operandFloat) 
	  || (op.right.type == operandSVValueX) 
	  || (op.right.type == operandSVValueY) 
	  || (op.right.type == operandSVValueZ) 
	  || (op.right.type == operandSVValueW) 
	  || (op.right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op.left));
	  EvFloat f2 (eval.Float (op.right));
          result = f1 == f2;
	}
	else if (OpTypesCompatible (op.left.type, operandBoolean) 
	  && OpTypesCompatible (op.right.type, operandBoolean))
	{
	  EvBool b1 (eval.Boolean (op.left));
	  EvBool b2 (eval.Boolean (op.right));
	  result = b1 == b2;
	}
	else
	{
	  EvInt i1 (eval.Int (op.left));
	  EvInt i2 (eval.Int (op.right));
	  result = i1 == i2;
	}
	break;
      }
    case opNEqual:
      {
	if ((op.left.type == operandFloat) 
	  || (op.left.type == operandSVValueFloat)
	  || (op.left.type == operandSVValueX)
	  || (op.left.type == operandSVValueY)
	  || (op.left.type == operandSVValueZ)
	  || (op.left.type == operandSVValueW)
	  || (op.right.type == operandFloat) 
	  || (op.right.type == operandSVValueX) 
	  || (op.right.type == operandSVValueY) 
	  || (op.right.type == operandSVValueZ) 
	  || (op.right.type == operandSVValueW) 
	  || (op.right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op.left));
	  EvFloat f2 (eval.Float (op.right));
          result = f1 != f2;
	}
	else if (OpTypesCompatible (op.left.type, operandBoolean) 
	  && OpTypesCompatible (op.right.type, operandBoolean))
	{
	  EvBool b1 (eval.Boolean (op.left));
	  EvBool b2 (eval.Boolean (op.right));
	  result = b1 != b2;
	}
	else
	{
	  EvInt i1 (eval.Int (op.left));
	  EvInt i2 (eval.Int (op.right));
	  result = i1 != i2;
	}
	break;
      }
    case opLesser:
      {
	if ((op.left.type == operandFloat) 
	  || (op.left.type == operandSVValueFloat)
	  || (op.left.type == operandSVValueX)
	  || (op.left.type == operandSVValueY)
	  || (op.left.type == operandSVValueZ)
	  || (op.left.type == operandSVValueW)
	  || (op.right.type == operandFloat) 
	  || (op.right.type == operandSVValueX) 
	  || (op.right.type == operandSVValueY) 
	  || (op.right.type == operandSVValueZ) 
	  || (op.right.type == operandSVValueW) 
	  || (op.right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op.left));
	  EvFloat f2 (eval.Float (op.right));
	  result = (f1 < f2);
	}
	else
	{
	  EvInt i1 (eval.Int (op.left));
	  EvInt i2 (eval.Int (op.right));
	  result = i1 < i2;
	}
	break;
      }
    case opLesserEq:
      {
	if ((op.left.type == operandFloat) 
	  || (op.left.type == operandSVValueFloat)
	  || (op.left.type == operandSVValueX)
	  || (op.left.type == operandSVValueY)
	  || (op.left.type == operandSVValueZ)
	  || (op.left.type == operandSVValueW)
	  || (op.right.type == operandFloat) 
	  || (op.right.type == operandSVValueX) 
	  || (op.right.type == operandSVValueY) 
	  || (op.right.type == operandSVValueZ) 
	  || (op.right.type == operandSVValueW) 
	  || (op.right.type == operandSVValueFloat))
	{
	  EvFloat f1 (eval.Float (op.left));
	  EvFloat f2 (eval.Float (op.right));
	  result = (f1 <= f2);
	}
	else
	{
	  EvInt i1 (eval.Int (op.left));
	  EvInt i2 (eval.Int (op.right));
	  result = i1 <= i2;
	}
	break;
      }
    default:
      CS_ASSERT (false);
  }

  return result;
}

template<typename Evaluator>
typename Evaluator::EvalResult csConditionEvaluator::Evaluate (
  Evaluator& eval, csConditionID condition)
{
  LockType lock (mutex);
  return EvaluateInternal (eval, condition);
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
	  CondOperation op = conditions.GetCondition (operand.operation);
	  if (!EvaluateConst (op, result)) return false;
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
  CondOperation op = conditions.GetCondition (condition);

  if (op.left.type == operandOperation)
    GetUsedSVs2 (op.left.operation, affectedSVs);
  else if ((op.left.type >= operandSV) 
    && (op.left.type <= operandSVValueBuffer))
  {
    if (affectedSVs.GetSize() <= op.left.svLocation.svName)
      affectedSVs.SetSize (op.left.svLocation.svName+1);
    affectedSVs.SetBit (op.left.svLocation.svName);
  }

  if (op.right.type == operandOperation)
    GetUsedSVs2 (op.right.operation, affectedSVs);
  else if ((op.right.type >= operandSV) 
    && (op.right.type <= operandSVValueBuffer))
  {
    if (affectedSVs.GetSize() <= op.right.svLocation.svName)
      affectedSVs.SetSize (op.right.svLocation.svName+1);
    affectedSVs.SetBit (op.right.svLocation.svName);
  }
}

void csConditionEvaluator::GetUsedSVs (csConditionID condition, 
                                       MyBitArrayTemp& affectedSVs)
{
  LockType lock (mutex);
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

csString csConditionEvaluator::GetConditionStringInternal (csConditionID id)
{
  if (id == csCondAlwaysFalse)
    return "AlwaysFalse";
  else if (id == csCondAlwaysTrue)
    return "AlwaysTrue";
  else
    return OperationToString (conditions.GetCondition (id));
}

csString csConditionEvaluator::GetConditionString (csConditionID id)
{
  LockType lock (mutex);
  return GetConditionStringInternal (id);
}

void csConditionEvaluator::MarkAffectionBySVs (csConditionID condition,
					       const CondOperand& operand)
{
  switch (operand.type)
  {
    case operandSV:
    case operandSVValueBuffer:
    case operandSVValueFloat:
    case operandSVValueTexture:
    case operandSVValueInt:
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
      {
	MyBitArrayMalloc* bits;
	if (operand.svLocation.bufferName != CS_BUFFER_NONE)
	  bits = &bufferAffectConditions;
	else
	{
	  size_t idx = svAffectedConditions.FindSortedKey (
	    csArrayCmp<SVAffection, CS::ShaderVarStringID> (operand.svLocation.svName));
	  if (idx == csArrayItemNotFound)
	    idx = svAffectedConditions.InsertSorted (SVAffection (operand.svLocation.svName));
	  SVAffection& affectedConds = svAffectedConditions[idx];
	  bits = &affectedConds.affectedConditions;
	}
	if (bits->GetSize() <= condition)
	  SetSizeFill1 (*bits, condition+1);
	bits->ClearBit (condition);
      }
      break;
    case operandOperation:
      {
	CondOperation operation = conditions.GetCondition (operand.operation);
	MarkAffectionBySVs (condition, operation.left);
	MarkAffectionBySVs (condition, operation.right);
      }
      break;
    default:
      // Do nothing
      break;
  }
}

csString csConditionEvaluator::OperationToString (const CondOperation& operation)
{
  const char* opStr;
  switch (operation.operation)
  {
    case opAnd:       opStr = "&&"; break;
    case opOr:        opStr = "||"; break;
    case opEqual:     opStr = "=="; break;
    case opNEqual:    opStr = "!="; break;
    case opLesser:    opStr = "<"; break;
    case opLesserEq:  opStr = "<="; break;
    default:
      return (const char*)0;
  }
  
  csString ret;
  ret.Format ("%s %s %s",
    OperandToString (operation.left).GetData(),
    opStr,
    OperandToString (operation.right).GetData());
  return ret;
}

csString csConditionEvaluator::OperandToString (const CondOperand& operand)
{
  csString ret;
  
  switch (operand.type)
  {
    case operandOperation:
      ret.Format ("(%s)", GetConditionStringInternal (operand.operation).GetData());
      break;
    case operandFloat:
      ret.Format ("%g", operand.floatVal);
      break;
    case operandInt:
      ret.Format ("%d", operand.intVal);
      break;
    case operandBoolean:
      ret = operand.boolVal ? "true" : "false";
      break;
    case operandSV:
    case operandSVValueInt:
    case operandSVValueFloat:
    case operandSVValueX:
    case operandSVValueY:
    case operandSVValueZ:
    case operandSVValueW:
    case operandSVValueTexture:
    case operandSVValueBuffer:
      {
        ret.Format ("vars.\"%s\"", strings->Request (
          operand.svLocation.svName));
        if (operand.svLocation.indices != 0)
        {
          size_t n = *operand.svLocation.indices;
          for (size_t i = 0; i < n; i++)
            ret.AppendFmt ("[%zu]", operand.svLocation.indices[i+1]);
        }
	switch (operand.type)
	{
	  case operandSVValueInt:      ret.Append (".int"); break;
	  case operandSVValueFloat:    ret.Append (".float"); break;
	  case operandSVValueX:        ret.Append (".x"); break;
	  case operandSVValueY:        ret.Append (".y"); break;
	  case operandSVValueZ:        ret.Append (".z"); break;
	  case operandSVValueW:        ret.Append (".w"); break;
	  case operandSVValueTexture:  ret.Append (".texture"); break;
	  case operandSVValueBuffer:   ret.Append (".buffer"); break;
	  default: break;
        }
        break;
      }
    default: break;
  }
  
  return ret;
}

Logic3 csConditionEvaluator::CheckConditionResultsInternal (
  csConditionID condition, const Variables& vars)
{
  EvaluatorShadervarValuesSimple eval (*this, vars);
  return EvaluateInternal (eval, condition);
}

Logic3 csConditionEvaluator::CheckConditionResults (
  csConditionID condition, const Variables& vars)
{
  LockType lock (mutex);
  return CheckConditionResultsInternal (condition, vars);
}

Logic3 csConditionEvaluator::CheckConditionResultsInternal (
  csConditionID condition, const Variables& vars, 
  Variables& trueVars, Variables& falseVars)
{
  trueVars = falseVars = vars;
  EvaluatorShadervarValues eval (*this, vars, trueVars, falseVars);
  return EvaluateInternal (eval, condition);
}

Logic3 csConditionEvaluator::CheckConditionResults (
  csConditionID condition, const Variables& vars, 
  Variables& trueVars, Variables& falseVars)
{
  LockType lock (mutex);
  return CheckConditionResultsInternal (condition, vars, trueVars, falseVars);
}

  void csConditionEvaluator::RecycleTicketEvaluator (TicketEvaluator* p)
  {
    p->poolNext = ticketEvalPool;
    ticketEvalPool = p;
  }

  void csConditionEvaluator::RecycleEvalState (EvalState* p)
  {
    if (p == &evalCache) return;
    p->poolNext = evalStatePool;
    evalStatePool = p;
  }

//---------------------------------------------------------------------------

  csConditionEvaluator::TicketEvaluator::TicketEvaluator (
    csConditionEvaluator* owner, bool hasLock,
      EvalState* evalState, EvaluatorShadervar& eval)
    : owner (owner), mutex (owner->mutex), inEval (true), hasLock (hasLock),
      evalState (evalState), eval (eval)
  {
  }
  
  csConditionEvaluator::TicketEvaluator::~TicketEvaluator()
  {
    EndEvaluation();
    owner->RecycleEvalState (evalState);
  }
  
  void csConditionEvaluator::TicketEvaluator::DecRef ()
  {
    csRefTrackerAccess::TrackDecRef (this, ref_count);
    ref_count--;
    if (ref_count <= 0)
    {
      csConditionEvaluator* _owner = owner;
      this->~TicketEvaluator();
      _owner->RecycleTicketEvaluator (this);
    }
  }
  
  bool csConditionEvaluator::TicketEvaluator::Evaluate (csConditionID condition)
  {
    CS_ASSERT(inEval);
    
    return owner->EvaluateCachedInternal (evalState, eval, condition);
  }
  
  void csConditionEvaluator::TicketEvaluator::EndEvaluation()
  {
    if (inEval)
    {
      if (hasLock) mutex.Unlock();
    }
    inEval = false;
  }
 
}
CS_PLUGIN_NAMESPACE_END(XMLShader)
