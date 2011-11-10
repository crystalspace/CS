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

#include "ivideo/rndbuf.h"
#include "csutil/csstring.h"
#include "csutil/strhash.h"
#include "csutil/sysfunc.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  /* @@@ NOTE! Enums are written to disk (shader cache). If you change value 
     make sure you also change cacheFileMagic in shader.cpp! */

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
  const csConditionID csCondUnknown = (csConditionID)~2;

  /// An actual operand.
  struct CondOperand
  {
    OperandType type;
    struct ShaderVarLocator
    {
      CS::StringIDValue svName;
      size_t* indices;
      csRenderBufferName bufferName;
    };
    union
    {
      int intVal;
      float floatVal;
      bool boolVal;
      ShaderVarLocator svLocation;
      csConditionID operation;
    };
    CondOperand ()
    { memset (this, 0, sizeof (*this)); }
    CondOperand (int /* justToHaveADifferentSignature*/) 
    { /* Speed hack for when being a member of CondOperation:
       * it's "initialization" will null this as well */ }
       
    inline void Clear () { memset (this, 0, sizeof (*this)); }
       
    bool operator== (const CondOperand& other) const
    {
      if (type != other.type) return false;
      if (type >= operandSV)
      {
        if (svLocation.svName != other.svLocation.svName) return false;
        if ((svLocation.indices != 0) != (other.svLocation.indices != 0)) return false;
        if (svLocation.indices == 0) return true;
        size_t n1 = *svLocation.indices;
        size_t n2 = *other.svLocation.indices;
        if (n1 != n2) return false;
        return memcmp (svLocation.indices+1, other.svLocation.indices+1, n1 * sizeof (size_t)) == 0;
      }
      else
      {
        return memcmp (this, &other, sizeof (CondOperand)) == 0;
      }
    }
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

template<>
class csHashComputer<NS_XMLSHADER::CondOperation>
{
  static uint ActualHash (NS_XMLSHADER::ConditionOp operation, 
    const NS_XMLSHADER::CondOperand& left, 
    const NS_XMLSHADER::CondOperand& right)
  {
    NS_XMLSHADER::CondOperation tempOp;
    uint leftIndexHash = 0, rightIndexHash = 0;
    tempOp.operation = operation;
    tempOp.left = left;
    if (tempOp.left.type >= NS_XMLSHADER::operandSV)
    {
      if (tempOp.left.svLocation.indices != 0)
      {
        leftIndexHash = csHashCompute (
          (char*)(tempOp.left.svLocation.indices + 1),
          *tempOp.left.svLocation.indices * sizeof (size_t));
      }
      tempOp.left.svLocation.indices = (size_t*)(uintptr_t)leftIndexHash;
    }
    tempOp.right = right;
    if (tempOp.right.type >= NS_XMLSHADER::operandSV)
    {
      if (tempOp.right.svLocation.indices != 0)
      {
        rightIndexHash = csHashCompute (
          (char*)(tempOp.right.svLocation.indices + 1),
          *tempOp.right.svLocation.indices * sizeof (size_t));
      }
      tempOp.right.svLocation.indices = (size_t*)(uintptr_t)rightIndexHash;
    }
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

template<>
class csComparator<NS_XMLSHADER::CondOperation, 
  NS_XMLSHADER::CondOperation>
{
public:
  static int Compare (NS_XMLSHADER::CondOperation const& op1, 
    NS_XMLSHADER::CondOperation const& op2)
  {
    if (op1.operation == op2.operation)
    {
      bool result;
      result = (op1.left == op2.left)
	&& (op1.right == op2.right);
      if (NS_XMLSHADER::IsOpCommutative (op1.operation))
      {
        result = result 
	  || ((op1.left == op2.right)
	    && (op1.right == op2.left));
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
