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
#include "csutil/cowwrapper.h"
#include "csutil/hashr.h"
#include "csutil/memheap.h"
#include "csutil/ptrwrap.h"
#include "csgeom/math.h"
#include "iutil/strset.h"
#include "ivideo/shader/shader.h"

#include "condition.h"
#include "expparser.h"
#include "logic3.h"
#include "valueset.h"
#include "mybitarray.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

/// Container for shader expression constants
class csConditionEvaluator;

/**
 * Class for an optimization: Some compilers use a 'vector ctor' even if the
 * array has just a size of 1, so specialize 1-elemented arrays that they 
 * aren't arrays at all.
 */
template<int N>
struct ValueSetArray
{
  ValueSet x[N];

  ValueSet& operator[] (size_t index) 
  {
    CS_ASSERT(index < N);
    return x[index];
  }
  const ValueSet& operator[] (size_t index) const
  {
    CS_ASSERT(index < N);
    return x[index];
  }
};

template<>
struct ValueSetArray<1>
{
  ValueSet x;

  ValueSet& operator[] (size_t index) 
  {
    CS_ASSERT(index == 0);
    (void)index;
    return x;
  }
  const ValueSet& operator[] (size_t index) const
  {
    CS_ASSERT(index == 0);
    (void)index;
    return x;
  }
};

class Variables
{
public:
  struct Values
  {
    void IncRef () { refcount++; }
    static size_t deallocCount;
    void DecRef () 
    { 
      refcount--; 
      if (refcount == 0) 
      {
        Variables::ValAlloc().Free (this);
        /*deallocCount++;
        if (deallocCount > 65536)
        {
          Variables::ValAlloc().Compact();
          deallocCount = 0;
        }*/
      }
    }
    int GetRefCount () const { return refcount; }

  private:
    int refcount;
    CS_DECLARE_STATIC_CLASSVAR_REF (def, Def, ValueSet);

    enum
    {
      valueVar = 0,
      valueVec0,
      valueVec1,
      valueVec2,
      valueVec3,
      valueTex,
      valueBuf,

      valueFirst = valueVar,
      valueLast = valueBuf,

      // Bits needed to store one of the value* values above
      valueBits = 3,
      valueIndexOffs = valueLast + 1
    };
    uint valueFlags;
    void SetValueIndex (uint value, uint index)
    {
      static const uint valueMask = (1 << valueBits) - 1;
      uint shift = valueIndexOffs + value * valueBits;
      valueFlags &= ~(valueMask << shift);
      valueFlags |= index << shift;
    }
    uint GetValueIndex (uint value) const
    {
      static const uint valueMask = (1 << valueBits) - 1;
      uint shift = valueIndexOffs + value * valueBits;
      return (valueFlags >> shift) & valueMask;
    }

    /* It works like this:
     * - the first inlinedSets value sets are inlined
     * - all sets above that stored in multiValues
     * - since the references may not move, use a linked list for the
     *   non-inlined sets.
     */
    const static uint inlinedSets = 1;
    ValueSetArray<inlinedSets> inlineValues;
    struct ValueSetChain
    {
      ValueSet vs;
      ValueSetChain* nextPlease;

      ValueSetChain () : nextPlease (0) {}
      ValueSetChain (const ValueSetChain& other) : vs (other.vs), 
        nextPlease (0) {}
      ~ValueSetChain() { delete nextPlease; }
      // @@@ TODO: probably should use a blockalloc here as well
    };
    ValueSetChain* multiValues;

    ValueSet& GetMultiValue (uint num);
    const ValueSet& GetMultiValue (uint num) const;

    ValueSet& GetValue (int type);
    const ValueSet& GetValue (int type) const;

    uint ValueCount (uint flags) const
    {
      uint n = 0;
      for (uint v = valueFirst; v <= valueLast; v++)
        if (flags & (1 << v)) n++;
      return n;
    }
    
  public:
    ValueSet& GetVar() { return GetValue (valueVar); }
    const ValueSet& GetVar() const { return GetValue (valueVar); }
    ValueSet& GetVec (int n) 
    { 
      CS_ASSERT((n >= 0) && (n < 4));
      return GetValue (valueVec0 + n);
    }
    const ValueSet& GetVec (int n) const
    { 
      CS_ASSERT((n >= 0) && (n < 4));
      return GetValue (valueVec0 + n);
    }
    ValueSet& GetTex() { return GetValue (valueTex); }
    const ValueSet& GetTex() const { return GetValue (valueTex); }
    ValueSet& GetBuf() { return GetValue (valueBuf); }
    const ValueSet& GetBuf() const { return GetValue (valueBuf); }

    Values () : refcount (1), valueFlags (0), multiValues (0) { }
    Values (const Values& other) : refcount (1), valueFlags (other.valueFlags), 
      multiValues (0)
    {
      for (uint n = 0; n < inlinedSets; n++)
      {
        inlineValues[n] = other.inlineValues[n];
      }

      ValueSetChain* s = other.multiValues;
      ValueSetChain** d = &multiValues;
      while (s != 0)
      {
        ValueSetChain* p = new ValueSetChain (*s);
        *d = p;
        d = &p->nextPlease;
        s = s->nextPlease;
      }
    }
    ~Values()
    {
      delete multiValues;
    }

    Values& operator=(const Values& other)
    {
      valueFlags = other.valueFlags;

      for (uint n = 0; n < inlinedSets; n++)
      {
        inlineValues[n] = other.inlineValues[n];
      }

      delete multiValues; multiValues = 0;
      ValueSetChain* s = other.multiValues;
      ValueSetChain** d = &multiValues;
      while (s != 0)
      {
        ValueSetChain* p = new ValueSetChain (*s);
        *d = p;
        d = &p->nextPlease;
        s = s->nextPlease;
      }
      return *this;
    }
    friend Values operator& (const Values& a, const Values& b)
    {
      Values newValues;
      for (int v = valueFirst; v <= valueLast; v++)
      {
        const bool aHas = (a.valueFlags & (1 << v)) != 0;
        const bool bHas = (b.valueFlags & (1 << v)) != 0;
        if (aHas && bHas)
          newValues.GetValue (v) = a.GetValue (v) & b.GetValue (v);
        else if (aHas && !bHas)
          newValues.GetValue (v) = a.GetValue (v);
        else if (!aHas && bHas)
          newValues.GetValue (v) = b.GetValue (v);
      }
      return newValues;
    }
    Values& operator&= (const Values& other)
    {
      for (int v = valueFirst; v <= valueLast; v++)
      {
        if (other.valueFlags & (1 << v))
          GetValue (v) &= other.GetValue (v);
      }
      return *this;
    }
    friend Values operator| (const Values& a, const Values& b)
    {
      Values newValues;
      for (int v = valueFirst; v <= valueLast; v++)
      {
        const bool aHas = (a.valueFlags & (1 << v)) != 0;
        const bool bHas = (b.valueFlags & (1 << v)) != 0;
        if (aHas && bHas)
          newValues.GetValue (v) = a.GetValue (v) | b.GetValue (v);
      }
      return newValues;
    }
    Values& operator|= (const Values& other)
    {
      for (int v = valueFirst; v <= valueLast; v++)
      {
        if (valueFlags & (1 << v))
          GetValue (v) |= other.GetValue (v);
      }
      return *this;
    }
    friend Logic3 operator== (const Values& a, const Values& b);
    friend Logic3 operator!= (const Values& a, const Values& b);

    friend Logic3 operator< (const Values& a, const Values& b);
    friend Logic3 operator<= (const Values& a, const Values& b);

    static void CompactAllocator()
    {
      ValAlloc().Compact();
      CowBlockAllocator::CompactAllocator();
    }
  };
protected:
  typedef csBlockAllocator<Values, TempHeapAlloc> ValBlockAlloc;
  CS_DECLARE_STATIC_CLASSVAR_REF (valAlloc,
    ValAlloc, ValBlockAlloc);
  static size_t valDeAllocCount;
  CS_DECLARE_STATIC_CLASSVAR (def,
    Def, Values);

  class ValuesWrapper
  {
    csRef<Values> values;
  public:
    operator const Values* () const 
    { 
      return values; 
    }
    operator csRef<Values>& () 
    { 
      if (values.IsValid() && (values->GetRefCount() > 1))
      {
        Values* newVals = ValAlloc().Alloc ();
        *newVals = *values;
        values.AttachNew (newVals);
      }
      return values; 
    }
  };

  struct ValuesArray
  {
    struct Entry
    {
      csStringID n;
      ValuesWrapper v;

      inline operator csStringID() const { return n; }
    };
    typedef csArray<Entry, csArrayElementHandler<Entry>,
      TempHeapAlloc,
      csArrayCapacityLinear<csArrayThresholdFixed<4> > > ArrayType;

    ArrayType _array;
    inline operator ArrayType& ()
    {
      return _array;
    }
    inline ValuesWrapper& GetExtend (csStringID n) 
    { 
      size_t candidate;
      size_t index = _array.FindSortedKey (csArrayCmp<Entry, csStringID> (n),
        &candidate);
      if (index != csArrayItemNotFound) return _array[index].v;
      Entry newEntry;
      newEntry.n = n;
      _array.Insert (candidate, newEntry);
      return _array[candidate].v;
    }
    inline size_t GetSize () const { return _array.GetSize (); }
    inline ValuesWrapper& GetIndex (size_t n) { return _array.Get (n).v; }
    inline const ValuesWrapper& Get (csStringID n) const 
    { 
      size_t index = _array.FindSortedKey (csArrayCmp<Entry, csStringID> (n));
      return _array[index].v;
    }
    inline bool Has (csStringID n) const
    { 
      size_t index = _array.FindSortedKey (csArrayCmp<Entry, csStringID> (n));
      return index != csArrayItemNotFound;
    }
    inline const Entry& operator[] (size_t n) const { return _array[n]; }
    inline void Delete (csStringID n)
    {
      size_t index = _array.FindSortedKey (csArrayCmp<Entry, csStringID> (n));
      _array.DeleteIndex (index);
    }
    inline void DeleteIndex (size_t index) { _array.DeleteIndex (index); }
  };
  class CowBlockAllocator;
  typedef CS::CowWrapper<ValuesArray, CowBlockAllocator> ValuesArrayWrapper;
  class CowBlockAllocator
  {
  public:
    typedef csFixedSizeAllocator<ValuesArrayWrapper::allocSize, 
      TempHeapAlloc> BlockAlloc;
  private:
    CS_DECLARE_STATIC_CLASSVAR_REF (allocator,
      Allocator, BlockAlloc);
  public:
    static void* Alloc (size_t n)
    {
      (void)n; // Pacify compiler warnings
      CS_ASSERT(n == ValuesArrayWrapper::allocSize);
      return Allocator().Alloc ();
    }
    static void Free (void* p)
    {
      Allocator().Free (p);
    }
    static void CompactAllocator()
    {
      Allocator().Compact();
    }
  };
  ValuesArrayWrapper possibleValues;
public:
  Variables () : possibleValues (ValuesArray ()) {}
  Variables (const Variables& other) : possibleValues (other.possibleValues)
  { }
  Variables& operator= (const Variables& other)
  {
    possibleValues = other.possibleValues;
    return *this;
  }
  ~Variables () 
  {
  }

  Values* GetValues (csStringID variable)
  {
    csRef<Values>& vals = possibleValues->GetExtend (variable);
    if (!vals) vals.AttachNew (ValAlloc().Alloc());
    return vals;
  }
  const Values* GetValues (csStringID variable) const
  {
    const Values* vals = 0;
    if (possibleValues->Has (variable))
      vals = possibleValues->Get (variable);
    if (vals != 0)
      return vals;
    else
      return Def();
  }
  friend Variables operator| (const Variables& a, const Variables& b)
  {
    Variables newVars (a);
    ValuesArray& nva = *newVars.possibleValues;
    const ValuesArray& pvB = *b.possibleValues;
    for (size_t n = 0; n < nva.GetSize(); )
    {
      csStringID name = nva[n].n;
      if (pvB.Has (name))
      {
        const Values* entry = pvB.Get (name);
        csRef<Values>& vw = nva.GetIndex (n);
        *vw |= *entry;
        n++;
      }
      else
        nva.DeleteIndex (n);
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
  MyBitArrayMalloc condChecked;
  MyBitArrayMalloc condResult;

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

  void GetUsedSVs2 (csConditionID condition, MyBitArrayTemp& affectedSVs);

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
    const iArrayReadOnly<csShaderVariable*>* stacks;

    EvalResult GetDefaultResult() const { return false; }

    EvaluatorShadervar (csConditionEvaluator& evaluator,
      const csRenderMeshModes& modes, const iShaderVarStack* stacks) : 
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
  typename Evaluator::EvalResult Evaluate (Evaluator& eval, csConditionID condition);

  csConditionEvaluator (iStringSet* strings, 
    const csConditionConstants& constants);

  /// Convert expression into internal representation.
  const char* ProcessExpression (csExpression* expression, 
    csConditionID& cond);

  /// Evaluate a condition and return the result.
  bool Evaluate (csConditionID condition, const csRenderMeshModes& modes,
    const iShaderVarStack* stacks);
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
  Logic3 CheckConditionResults (csConditionID condition,
    const Variables& vars);

  const char* ProcessExpression (csExpression* expression, 
    CondOperation& operation);
  /**
   * Get the ID for an operation, but also do some optimization of the 
   * expression. 
   */
  csConditionID FindOptimizedCondition (const CondOperation& operation);

  /**
   * Test if \c condition is a sub-condition of \c containerCondition 
   * (e.g. true if \c condition is AND-combined with some other condition).
   */
  bool IsConditionPartOf (csConditionID condition, 
    csConditionID containerCondition);

  /// Determine which SVs are used in some condition.
  void GetUsedSVs (csConditionID condition, MyBitArrayTemp& affectedSVs);

  /// Try to release unused temporary memory
  static void CompactMemory ();
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_H__
