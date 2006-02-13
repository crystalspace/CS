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

#ifndef __CS_CONDEVAL_H__
#define __CS_CONDEVAL_H__

#include "csutil/array.h"
#include "csutil/bitarray.h"
#include "csutil/blockallocator.h"
#include "csutil/hashr.h"
#include "csgeom/math.h"
#include "iutil/strset.h"
#include "ivideo/shader/shader.h"

#include "condition.h"
#include "expparser.h"
#include "logic3.h"
#include "valueset.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

/// Container for shader expression constants
class csConditionEvaluator;

class Variables
{
public:
  struct Values
  {
    // The variable "itself" (boolean for variables resp. constants)
    ValueSet var;
    ValueSet vec[4];
    ValueSet tex;
    ValueSet buf;

    Values () {}
    Values (const float f) : var (f), tex (f), buf (f)
    {
      for (int i = 0; i < 4; i++)
        vec[i] = ValueSet (f);
    }

    Values& operator=(const Values& other)
    {
      var = other.var;
      for (int i = 0; i < 4; i++)
        vec[i] = other.vec[i];
      tex = other.tex;
      buf = other.buf;
      return *this;
    }
    friend Values operator& (const Values& a, const Values& b)
    {
      Values newValues;
      newValues.var = a.var & b.var;
      for (int i = 0; i < 4; i++)
        newValues.vec[i] = a.vec[i] & b.vec[i];
      newValues.tex = a.tex & b.tex;
      newValues.buf = a.buf & b.buf;
      return newValues;
    }
    friend Values operator| (const Values& a, const Values& b)
    {
      Values newValues;
      newValues.var = a.var | b.var;
      for (int i = 0; i < 4; i++)
        newValues.vec[i] = a.vec[i] | b.vec[i];
      newValues.tex = a.tex | b.tex;
      newValues.buf = a.buf | b.buf;
      return newValues;
    }
    friend Logic3 operator== (const Values& a, const Values& b);
    friend Logic3 operator!= (const Values& a, const Values& b);

    friend Logic3 operator< (const Values& a, const Values& b);
    friend Logic3 operator<= (const Values& a, const Values& b);
  };
protected:
  Values def;
  csArray<Values*> possibleValues;
  csBlockAllocator<Values> valAlloc;

  void CopyValues (const Variables& other)
  {
    for (size_t i = 0; i < other.possibleValues.GetSize(); i++)
    {
      const Values* v = other.possibleValues[i];
      if (v != 0)
      {
        Values* newVals = valAlloc.Alloc ();
        *newVals = *v;
        possibleValues.Push (newVals);
      }
      else
        possibleValues.Push (0);
    }
  }
public:
  Variables () {}
  Variables (const Variables& other)
  { CopyValues (other); }
  Variables& operator= (const Variables& other)
  {
    valAlloc.Empty();
    possibleValues.Empty();
    CopyValues (other);
    return *this;
  }

  Values* GetValues (csStringID variable)
  {
    Values*& vals = possibleValues.GetExtend (variable);
    if (!vals) vals = valAlloc.Alloc();
    return vals;
  }
  const Values* GetValues (csStringID variable) const
  {
    const Values* vals = 0;
    if (possibleValues.GetSize () > variable)
      vals = possibleValues[variable];
    if (vals != 0)
      return vals;
    else
      return &def;
  }
  friend Variables operator& (const Variables& a, const Variables& b)
  {
    Variables newVars;
    const size_t num = csMin (a.possibleValues.GetSize(),
      b.possibleValues.GetSize());
    for (size_t i = 0; i < num; i++)
    {
      const Values* va = a.possibleValues[i];
      const Values* vb = b.possibleValues[i];
      if ((va != 0) && (vb != 0))
      {
        Values*& v = newVars.possibleValues.GetExtend (i);
        v = newVars.valAlloc.Alloc ();
        *v = *va & *vb;
      }
    }
    return newVars;
  }
  friend Variables operator| (const Variables& a, const Variables& b)
  {
    Variables newVars;
    const csArray<Values*>& pvA = a.possibleValues;
    const csArray<Values*>& pvB = b.possibleValues;
    const size_t num = csMax (pvA.GetSize(), pvB.GetSize());
    for (size_t i = 0; i < num; i++)
    {
      const Values* va = (i < pvA.GetSize()) ? pvA[i] : 0;
      const Values* vb = (i < pvB.GetSize()) ? pvB[i] : 0;
      if ((va != 0) || (vb != 0))
      {
        if (va == 0)
        {
          Values*& v = newVars.possibleValues.GetExtend (i);
          v = newVars.valAlloc.Alloc ();
          *v = *vb;
        }
        else if (vb == 0)
        {
          Values*& v = newVars.possibleValues.GetExtend (i);
          v = newVars.valAlloc.Alloc ();
          *v = *va;
        }
        else
        {
          Values*& v = newVars.possibleValues.GetExtend (i);
          v = newVars.valAlloc.Alloc ();
          *v = (*va | *vb);
        }
      }
    }
    return newVars;
  }
};

/**
 * Processes an expression tree and converts it into an internal 
 * representation and allows later evaluation of this expression.
 */
class csConditionEvaluator
{
  /// Used to resolve SV names.
  csRef<iStringSet> strings;

  csConditionID nextConditionID;
  csHashReversible<csConditionID, CondOperation> conditions;

  // Evaluation cache
  csBitArray condChecked;
  csBitArray condResult;

  // Constants
  const csConditionConstants& constants;

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
  const char* ResolveConst (csExpression* expression, 
    CondOperand& operand);

  bool EvaluateConst (const CondOperation& operation, bool& result);
  bool EvaluateOperandBConst (const CondOperand& operand, bool& result);
  bool EvaluateOperandIConst (const CondOperand& operand, int& result);
  bool EvaluateOperandFConst (const CondOperand& operand, float& result);

  struct EvaluatorShadervar
  {
    typedef bool EvalResult;
    typedef bool BoolType;
    struct FloatType
    {
      float v;

      FloatType () {}
      FloatType (float f) : v (f) {}
      operator float() const { return v; }
      bool operator== (const FloatType& other)
      { return fabsf (v - other.v) < SMALL_EPSILON; }
      bool operator!= (const FloatType& other)
      { return !operator==(other); }
    };
    typedef int IntType;
    csConditionEvaluator& evaluator;
    const csRenderMeshModes& modes;
    const csShaderVarStack& stacks;

    EvalResult GetDefaultResult() const { return false; }

    EvaluatorShadervar (csConditionEvaluator& evaluator,
      const csRenderMeshModes& modes, const csShaderVarStack& stacks) : 
        evaluator (evaluator), modes (modes), stacks (stacks)
    { }
    BoolType Boolean (const CondOperand& operand);
    IntType Int (const CondOperand& operand);
    FloatType Float (const CondOperand& operand);

    EvalResult LogicAnd (const CondOperand& a, const CondOperand& b)
    { return Boolean (a) && Boolean (b); }
    EvalResult LogicOr (const CondOperand& a, const CondOperand& b)
    { return Boolean (a) || Boolean (b); }
  };
public:
  template<typename Evaluator>
  typename_qualifier Evaluator::EvalResult Evaluate (Evaluator& eval, csConditionID condition);

  csConditionEvaluator (iStringSet* strings, 
    const csConditionConstants& constants);

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

  /*
   * Check whether a condition, given a set of possible values for
   * variables, is definitely true, definitely false, or uncertain.
   *
   * If uncertain returns the possible variable values in the case
   * the condition would be true resp. false.
   */
  Logic3 CheckConditionResults (csConditionID condition,
    const Variables& vars, Variables& trueVars, Variables& falseVars);
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_H__
