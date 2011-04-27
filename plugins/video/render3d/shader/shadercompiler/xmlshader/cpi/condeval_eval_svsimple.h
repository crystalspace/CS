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

#ifndef __CS_CONDEVAL_EVAL_SVSIMPLE_H__
#define __CS_CONDEVAL_EVAL_SVSIMPLE_H__

#include "condeval_slicealloc.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
#include "csutil/custom_new_disable.h"

  class ValueSetBoolAlloc
  {
    // Abuse the fact ValueSetBool doesn't need to be destructed
    csArray<uint8*, csArrayElementHandler<uint8*>, TempHeapAlloc> blocks;
    uint8* block;
    size_t blockRemaining;
    CS::Threading::Mutex mutex;
  public:
    ValueSetBoolAlloc() : blockRemaining (0) {}
    ~ValueSetBoolAlloc()
    {
      for (size_t i = 0; i < blocks.GetSize(); i++)
	SliceAllocatorBool::Free (blocks[i]);
    }
    
    ValueSetBool* Alloc()
    {
      CS::Threading::MutexScopedLock lock(mutex);
      if (blockRemaining == 0)
      {
	blockRemaining = SliceAllocatorBool::sliceSize;
	block = SliceAllocatorBool::Alloc (blockRemaining);
	blocks.Push (block);
      }
      void* r = block;
      block += sizeof (ValueSetBool);
      blockRemaining -= sizeof (ValueSetBool);
      return new (r) ValueSetBool;
    }
  };

#include "csutil/custom_new_enable.h"

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

  struct ValueSetBoolWrapper
  {
    const ValueSetBool* startVals;

    ValueSetBoolWrapper (const ValueSetBool& startVals) : startVals (&startVals) {}
    ValueSetBoolWrapper (bool b);

    // @@@ FIXME: probably cleaner to not (ab)use operators...
    friend Logic3 operator== (ValueSetBoolWrapper& a, ValueSetBoolWrapper& b)
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
    friend Logic3 operator!= (ValueSetBoolWrapper& a, ValueSetBoolWrapper& b)
    {
      Logic3 r = !operator== (a, b);
      return r;
    }

    operator Logic3 () const;
  };

  struct EvaluatorShadervarValuesSimple
  {
    typedef Logic3 EvalResult;
    typedef ValueSetBoolWrapper BoolType;
    typedef ValueSetWrapper FloatType;
    typedef ValueSetWrapper IntType;

    EvalResult GetDefaultResult() const
    { 
      return Logic3::Uncertain;
    }

    csConditionEvaluator& evaluator;
    const Variables& vars; 
    ValueSetBool boolUncertain;

    EvaluatorShadervarValuesSimple (csConditionEvaluator& evaluator, 
      const Variables& vars) : evaluator (evaluator), vars (vars), 
      createdValues (SliceAllocator::valueSetsPerSlice) {}

    BoolType Boolean (const CondOperand& operand);
    IntType Int (const CondOperand& operand)
    { return Float (operand); }
    FloatType Float (const CondOperand& operand);

    EvalResult LogicAnd (const CondOperand& a, const CondOperand& b);
    EvalResult LogicOr (const CondOperand& a, const CondOperand& b);

    csBlockAllocator<ValueSet, SliceAllocator> createdValues;
    ValueSetBoolAlloc createdBoolValues;
  protected:
    ValueSet& CreateValue ()
    {
      ValueSet* vs = createdValues.Alloc();
      return *vs;
    }
    ValueSetBool& CreateValueBool ()
    {
      ValueSetBool* vs = createdBoolValues.Alloc();
      return *vs;
    }
    ValueSet& CreateValue (float f)
    {
      ValueSet* vs = createdValues.Alloc();
      *vs = f;
      return *vs;
    }
    
    const Variables::Values* ValuesForOperand (const CondOperand& operand)
    {
      return vars.GetValues (operand.svLocation.svName, 
	operand.svLocation.indices ? *operand.svLocation.indices : 0,
	operand.svLocation.indices ? operand.svLocation.indices+1 : 0);
    }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_EVAL_SVSIMPLE_H__
