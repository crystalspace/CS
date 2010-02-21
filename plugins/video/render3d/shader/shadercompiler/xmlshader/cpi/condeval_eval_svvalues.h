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

#ifndef __CS_CONDEVAL_EVAL_SVVALUES_H__
#define __CS_CONDEVAL_EVAL_SVVALUES_H__

#include "condeval_eval_svsimple.h" // For ValueSetBoolAlloc

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

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

  struct JanusValueSetBool
  {
    const ValueSetBool* startVals;
    ValueSetBool* trueVals;
    ValueSetBool* falseVals;

    JanusValueSetBool (const ValueSetBool& startVals, ValueSetBool& trueVals, 
      ValueSetBool& falseVals) : startVals (&startVals), trueVals (&trueVals), 
      falseVals (&falseVals) {}

    // @@@ FIXME: probably cleaner to not (ab)use operators...
    friend Logic3 operator== (JanusValueSetBool& a, JanusValueSetBool& b)
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
	  ValueSetBool overlap = *a.startVals & *b.startVals;
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
    friend Logic3 operator!= (JanusValueSetBool& a, JanusValueSetBool& b)
    {
      Logic3 r = !operator== (a, b);
      ValueSetBool t1 = *a.trueVals;
      ValueSetBool t2 = *b.trueVals;
      ValueSetBool f1 = *a.falseVals;
      ValueSetBool f2 = *b.falseVals;
      /* To make != true, we need the ranges that makes == false
       * and vice versa */
      *a.trueVals = f1;
      *b.trueVals = f2;
      *a.falseVals = t1;
      *b.falseVals = t2;
      return r;
    }

    operator Logic3 () const;
  };

  struct EvaluatorShadervarValues
  {
    typedef Logic3 EvalResult;
    typedef JanusValueSetBool BoolType;
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
    ValueSetBool boolUncertain;

    EvaluatorShadervarValues (csConditionEvaluator& evaluator, const Variables& vars,
      Variables& trueVars, Variables& falseVars) : evaluator (evaluator), 
      vars (vars), trueVars (trueVars), falseVars (falseVars),
      createdValues (SliceAllocator::valueSetsPerSlice) {}

    BoolType Boolean (const CondOperand& operand);
    IntType Int (const CondOperand& operand)
    { return Float (operand); }
    FloatType Float (const CondOperand& operand);

    EvalResult LogicAnd (const CondOperand& a, const CondOperand& b);
    EvalResult LogicOr (const CondOperand& a, const CondOperand& b);

    csBlockAllocator<ValueSet/*, BlockAllocatorSlicePolicy*/> createdValues;
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
    
    Variables::Values* ValuesForOperand (Variables& vars, const CondOperand& operand)
    {
      return vars.GetValues (operand.svLocation.svName, 
	operand.svLocation.indices ? *operand.svLocation.indices : 0,
	operand.svLocation.indices ? operand.svLocation.indices+1 : 0);
    }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_EVAL_SVVALUES_H__
