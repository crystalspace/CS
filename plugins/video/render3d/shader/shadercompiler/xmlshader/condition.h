/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_CONDITION_H__
#define __CS_CONDITION_H__

#include "csutil/csstring.h"
#include "csutil/strhash.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  /**
   * Possible operations for a node in the internal expression
   * representation.
   */
  enum ConditionOp
  {
    opInvalid = 0,

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
    operandNone = 0,
    operandOperation,

    operandFloat,
    operandInt,
    operandBoolean,
    operandSV,
    operandSVValueInt,
    operandSVValueFloat,
    operandSVValueX,
    operandSVValueY,
    operandSVValueZ,
    operandSVValueW,
    operandSVValueTexture,
    operandSVValueBuffer
  };

  typedef size_t csConditionID;
  const csConditionID csCondAlwaysFalse = (csConditionID)~0;
  const csConditionID csCondAlwaysTrue = (csConditionID)~1;

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
    CondOperand (int /* justToHaveADifferentSignature*/) 
    { /* Speed hack for when being a member of CondOperation:
       * it's "initialization" will null this as well */ }
  };

  /// An operation.
  struct CondOperation
  {
    ConditionOp operation;
    CondOperand left;
    CondOperand right;

    CondOperation () : left (23), right (42)
    { memset (this, 0, sizeof (*this)); }
  };

  static bool IsOpCommutative (ConditionOp op)
  {
    return (op == opAnd) || (op == opOr) || (op == opEqual) || (op == opNEqual);
  }

  class csConditionConstants
  {
    csHash<CondOperand, csString> constants;
  public:
    //@{
    /// Add a constant to the list of constants.
    bool AddConstant (const char* name, float value);
    bool AddConstant (const char* name, int value);
    bool AddConstant (const char* name, bool value);
    //@}

    const CondOperand* GetConstant (const char* name) const
    { return constants.GetElementPointer (name); }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#define NS_XMLSHADER  CS_PLUGIN_NAMESPACE_NAME(XMLShader)

CS_SPECIALIZE_TEMPLATE
class csHashComputer<NS_XMLSHADER::CondOperation>
{
  static uint ActualHash (NS_XMLSHADER::ConditionOp operation, 
    const NS_XMLSHADER::CondOperand& left, 
    const NS_XMLSHADER::CondOperand& right)
  {
    NS_XMLSHADER::CondOperation tempOp;
    tempOp.operation = operation;
    tempOp.left = left;
    tempOp.right = right;
    return csHashCompute ((char*)&tempOp, sizeof (tempOp));
  }
public:
  static uint ComputeHash (NS_XMLSHADER::CondOperation const& operation)
  {
    uint result = ActualHash (operation.operation, operation.left, 
      operation.right);
    if (NS_XMLSHADER::IsOpCommutative (operation.operation))
      result ^= ActualHash (operation.operation, operation.right, 
      operation.left);
    return result;
  }
};

CS_SPECIALIZE_TEMPLATE
class csComparator<NS_XMLSHADER::CondOperation, 
  NS_XMLSHADER::CondOperation>
{
public:
  static int Compare (NS_XMLSHADER::CondOperation const& op1, 
    NS_XMLSHADER::CondOperation const& op2)
  {
    if (op1.operation == op2.operation)
    {
      bool result = (memcmp (&op1.left, &op2.left, 
          sizeof (NS_XMLSHADER::CondOperand)) == 0) 
        && (memcmp (&op1.right, &op2.right, 
          sizeof (NS_XMLSHADER::CondOperand)) == 0);
      if (NS_XMLSHADER::IsOpCommutative (op1.operation))
      {
        result = result 
	  || ((memcmp (&op1.left, &op2.right, 
            sizeof (NS_XMLSHADER::CondOperand)) == 0)
	  && (memcmp (&op1.right, &op2.left, 
            sizeof (NS_XMLSHADER::CondOperand)) == 0));
      }
      if (result) return 0;
      // @@@ Hm, just some order...
      return (int)csHashComputer<NS_XMLSHADER::CondOperation>::ComputeHash (op1)
        - (int)csHashComputer<NS_XMLSHADER::CondOperation>::ComputeHash (op2);
    }
    else
      return (int)op1.operation - (int)op2.operation;
  }
};

#undef NS_XMLSHADER

#endif // __CS_CONDITION_H__
