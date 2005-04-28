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

#ifndef __CONDEVAL_H__
#define __CONDEVAL_H__

#include "csutil/array.h"
#include "csutil/bitarray.h"
#include "csutil/hashr.h"
#include "iutil/strset.h"
#include "ivideo/shader/shader.h"

#include "docwrap.h"
#include "expparser.h"

/**
  * Possible operations for a node in the internal expression
  * representation.
  */
enum ConditionOp
{
  opInvalid = 0,

  opNot,
  opAnd,
  opOr,

  opEqual,
  opNEqual,
  opLesser,
  opLesserEq
};
/// Possible types of operands.
enum OperandType
{
  operandNone,
  operandOperation,

  operandFloat,
  operandInt,
  operandBoolean,
  operandSV,
  operandSVValueInt,
  operandSVValueFloat,
  operandSVValueTexture,
  operandSVValueBuffer,
};
/// An actual operand.
struct CondOperand
{
  OperandType type;
  union
  {
    int intVal;
    float floatVal;
    bool boolVal;
    csStringID svName;
    csConditionID operation;
  };
  CondOperand ()
  { memset (this, 0, sizeof (*this)); }
};
/// An operation.
struct CondOperation
{
  ConditionOp operation;
  CondOperand left;
  CondOperand right;

  CondOperation ()
  { operation = opInvalid; }
};

static bool IsOpCommutative (ConditionOp op)
{
  return (op == opAnd) || (op == opOr) || (op == opEqual) || (op == opNEqual);
}

CS_SPECIALIZE_TEMPLATE
class csHashComputer<CondOperation>
{
  static uint ActualHash (ConditionOp operation, const CondOperand& left, 
    const CondOperand& right)
  {
    CondOperation tempOp;
    tempOp.operation = operation;
    tempOp.left = left;
    tempOp.right = right;
    return csHashCompute ((char*)&tempOp, sizeof (tempOp));
  }
public:
  static uint ComputeHash (CondOperation const& operation)
  {
    uint result = ActualHash (operation.operation, operation.left, 
      operation.right);
    if (IsOpCommutative (operation.operation))
      result ^= ActualHash (operation.operation, operation.right, 
      operation.left);
    return result;
  }
};

CS_SPECIALIZE_TEMPLATE
class csComparator<CondOperation, CondOperation>
{
public:
  static int Compare (CondOperation const& op1, CondOperation const& op2)
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
      if (result) return 0;
      // @@@ Hm, just some order...
      return (int)csHashComputer<CondOperation>::ComputeHash (op1)
        - (int)csHashComputer<CondOperation>::ComputeHash (op2);
    }
    else
      return (int)op1.operation - (int)op2.operation;
  }
};


/**
 * Processes an expression tree and converts it into an internal 
 * representation and allows later evaluation of those expression.
 */
class csConditionEvaluator
{
  /// Used to resolve SV names.
  csRef<iStringSet> strings;

  class OperationHashKeyHandler
  {
    /**
     * Test whether an operation is communative.
     * Those ops are treated specially, so e.g. the expression
     * "a == b" gets the same hash as "b == a".
     */
    static bool IsOpCommutative (ConditionOp op);
    static uint ActualHash (ConditionOp operation,
      const CondOperand& left, const CondOperand& right);
  public:
    static uint ComputeHash (const CondOperation& operation);
    static bool CompareKeys (const CondOperation& op1, 
      const CondOperation& op2);
  };
  friend class OperationHashKeyHandler;

  csConditionID nextConditionID;
  csHashReversible<csConditionID, CondOperation> conditions;

  // Evaluation cache
  csBitArray condChecked;
  csBitArray condResult;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Check whether to operand types are compatible, ie can be compared.
   * E.g. 'bool' and 'int' are not compatible.
   */
  static bool OpTypesCompatible (OperandType t1, OperandType t2);
  /// Get a name for an operand type, for error reporting purposes.
  static const char* OperandTypeDescription (OperandType t);
  /**
   * Get the ID for an operation, but also do some optimization of the 
   * expression. */
  csConditionID FindOptimizedCondition (const CondOperation& operation);
  const char* ResolveExpValue (const csExpressionToken& value,
    CondOperand& operand);
  const char* ResolveOperand (csExpression* expression, 
    CondOperand& operand);
  const char* ResolveSVIdentifier (csExpression* expression, 
    CondOperand& operand);

  bool EvaluateOperandB (const CondOperand& operand, 
    const csRenderMeshModes& modes, const csShaderVarStack& stacks);
  int EvaluateOperandI (const CondOperand& operand, 
    const csRenderMeshModes& modes, const csShaderVarStack& stacks);
  float EvaluateOperandF (const CondOperand& operand, 
    const csRenderMeshModes& modes, const csShaderVarStack& stacks);

  bool EvaluateConst (const CondOperation& operation, bool& result);
  bool EvaluateOperandBConst (const CondOperand& operand, bool& result);
  bool EvaluateOperandIConst (const CondOperand& operand, int& result);
  bool EvaluateOperandFConst (const CondOperand& operand, float& result);
public:
  csConditionEvaluator (iStringSet* strings);

  /// Convert expression into internal representation.
  const char* ProcessExpression (csExpression* expression, 
    csConditionID& cond);

  /// Evaluate a condition and return the result.
  bool Evaluate (csConditionID condition, const csRenderMeshModes& modes,
    const csShaderVarStack& stacks);
  /**
   * Reset the evaluation cache. Prevents same conditions from being evaled 
   * twice.
   */
  void ResetEvaluationCache();

  /// Get number of conditions allocated so far
  size_t GetNumConditions() { return nextConditionID; }

  /**
   * Determines whether - under the condition that 'a' evaluates to aVal -
   * 'b' can still evaluate to something other than aVal.
   */
  bool ConditionIndependent (csConditionID a, bool aVal,
    csConditionID b);
};

#endif // __CONDEVAL_H__
