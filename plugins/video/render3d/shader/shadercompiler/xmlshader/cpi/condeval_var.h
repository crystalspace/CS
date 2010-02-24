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

#ifndef __CS_CONDEVAL_VAR_H__
#define __CS_CONDEVAL_VAR_H__

#include "ivideo/shader/shader.h"
#include "csutil/blockallocator.h"
#include "csutil/cowwrapper.h"

#include "valueset.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
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

  #include "csutil/custom_new_disable.h"

  class Variables
  {
  public:
    struct Values
    {
      void IncRef () { refcount++; }
      void DecRef () 
      { 
	refcount--; 
	if (refcount == 0) 
	{
	  Variables::ValAlloc().Free (this);
	}
      }
      int GetRefCount () const { return refcount; }

    private:
      friend class Variables;
    
      int refcount;
      CS_DECLARE_STATIC_CLASSVAR_REF (def, Def, ValueSet);

      enum
      {
	valueVec0 = 0,
	valueVec1,
	valueVec2,
	valueVec3,

	valueFirst = valueVec0,
	valueLast = valueVec3,

	// Bits needed to store one of the value* values above
	valueBits = 2,
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

      struct ValueSetChain;
      typedef csBlockAllocator<ValueSetChain, TempHeapAlloc> ValChainBlockAlloc;
      static void ValChainKill();
      
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
	~ValueSetChain() { ValChainAlloc().Free (nextPlease); }
      };
      ValueSetChain* multiValues;
      DECLARE_STATIC_CLASSVAR_DIRECT(ValChainAlloc, ValChainBlockAlloc, ValChainKill,);

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
      
      ValueSetBool valueVar;
      ValueSetBool valueTex;
      ValueSetBool valueBuf;
    public:
      ValueSetBool& GetVar() { return valueVar; }
      const ValueSetBool& GetVar() const { return valueVar; }
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
      ValueSetBool& GetTex() { return valueTex; }
      const ValueSetBool& GetTex() const { return valueTex; }
      ValueSetBool& GetBuf() { return valueBuf; }
      const ValueSetBool& GetBuf() const { return valueBuf; }

      Values () : refcount (1), valueFlags (0), multiValues (0) { }
      Values (const Values& other) : refcount (1), valueFlags (other.valueFlags), 
	multiValues (0), valueVar (other.valueVar), valueTex (other.valueTex),
	valueBuf (other.valueBuf)
      {
	for (uint n = 0; n < inlinedSets; n++)
	{
	  inlineValues[n] = other.inlineValues[n];
	}

	ValueSetChain* s = other.multiValues;
	ValueSetChain** d = &multiValues;
	while (s != 0)
	{
	  ValueSetChain* p = ValChainAlloc().Alloc (*s);
	  *d = p;
	  d = &p->nextPlease;
	  s = s->nextPlease;
	}
      }
      ~Values()
      {
	ValChainAlloc().Free (multiValues);
      }

      Values& operator=(const Values& other)
      {
	valueFlags = other.valueFlags;

	for (uint n = 0; n < inlinedSets; n++)
	{
	  inlineValues[n] = other.inlineValues[n];
	}
	valueVar = other.valueVar;
	valueTex = other.valueTex;
	valueBuf = other.valueBuf;

	ValChainAlloc().Free (multiValues); multiValues = 0;
	ValueSetChain* s = other.multiValues;
	ValueSetChain** d = &multiValues;
	while (s != 0)
	{
	  ValueSetChain* p = ValChainAlloc().Alloc (*s);
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
	newValues.valueVar = a.valueVar & b.valueVar;
	newValues.valueTex = a.valueTex & b.valueTex;
	newValues.valueBuf = a.valueBuf & b.valueBuf;
	return newValues;
      }
      Values& operator&= (const Values& other)
      {
	for (int v = valueFirst; v <= valueLast; v++)
	{
	  if (other.valueFlags & (1 << v))
	    GetValue (v) &= other.GetValue (v);
	}
	valueVar &= other.valueVar;
	valueTex &= other.valueTex;
	valueBuf &= other.valueBuf;
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
	newValues.valueVar = a.valueVar | b.valueVar;
	newValues.valueTex = a.valueTex | b.valueTex;
	newValues.valueBuf = a.valueBuf | b.valueBuf;
	return newValues;
      }
      Values& operator|= (const Values& other)
      {
	for (int v = valueFirst; v <= valueLast; v++)
	{
	  if (valueFlags & (1 << v))
	    GetValue (v) |= other.GetValue (v);
	}
	valueVar |= other.valueVar;
	valueTex |= other.valueTex;
	valueBuf |= other.valueBuf;
	return *this;
      }
      friend Logic3 operator== (const Values& a, const Values& b);
      friend Logic3 operator!= (const Values& a, const Values& b);

      friend Logic3 operator< (const Values& a, const Values& b);
      friend Logic3 operator<= (const Values& a, const Values& b);

      void Dump (csString& str) const
      {
	static const char* const valueNames[] = {
	  "v0", "v1", "v2", "v3"
	};
	csString valuesStr;
	for (uint v = valueFirst; v <= valueLast; v++)
	{
	  if (valueFlags & (1 << v))
	  {
	    str << valueNames[v-valueFirst];
	    str << ": ";
	    GetValue (v).Dump (valuesStr);
	    str << valuesStr;
	    str << "; ";
	  }
	}
	valueVar.Dump (valuesStr); str.AppendFmt ("var: %s; ", valuesStr.GetData());
	valueTex.Dump (valuesStr); str.AppendFmt ("tex: %s; ", valuesStr.GetData());
	valueBuf.Dump (valuesStr); str.AppendFmt ("buf: %s; ", valuesStr.GetData());
      }

      static void CompactAllocator()
      {
	ValAlloc().Compact();
	CowBlockAllocator::CompactAllocator();
      }
    };
  protected:
    typedef CS::Memory::BlockAllocatorSafe<Values, TempHeapAlloc> ValBlockAlloc;
    DECLARE_STATIC_CLASSVAR_DIRECT(ValAlloc, ValBlockAlloc, ValAllocKill,
      (1024));
    static void ValAllocKill();
      
    static size_t valDeAllocCount;
    CS_DECLARE_STATIC_CLASSVAR (def,
      Def, Values);

    class ValuesWrapper;
    typedef csArray<ValuesWrapper> ValuesWrapperArray;
    class ValuesWrapper
    {
      csRef<Values> values;
      ValuesWrapperArray* subValues;
    public:
      ValuesWrapper () : subValues (0) {}
      ValuesWrapper (const ValuesWrapper& other) : values (other.values),
	subValues (other.subValues ? new ValuesWrapperArray (*other.subValues) : 0)
      {}
    
      bool HasSubValues() const { return subValues != 0; }
      const ValuesWrapperArray* GetSubValues() const
      { return subValues; }
      ValuesWrapperArray* GetSubValues()
      {
	if (subValues == 0) subValues = new ValuesWrapperArray;
	return subValues;
      }

      operator const Values* () const 
      { 
	return values; 
      }
      csRef<Values>& AsRef() 
      { 
	if (values.IsValid() && (values->GetRefCount() > 1))
	{
	  Values* newVals = ValAlloc().Alloc ();
	  *newVals = *values;
	  values.AttachNew (newVals);
	}
	return values; 
      }
      operator csRef<Values>& () { return AsRef(); }
      operator Values* ()
      { 
	return AsRef(); 
      }
    };

    struct ValuesArray
    {
      struct Entry
      {
	CS::ShaderVarStringID n;
	ValuesWrapper v;

	inline operator CS::StringIDValue() const { return n; }
      };
      typedef csArray<Entry, csArrayElementHandler<Entry>,
	TempHeapAlloc,
	csArrayCapacityLinear<csArrayThresholdFixed<4> > > ArrayType;

      ArrayType _array;
      inline operator ArrayType& ()
      {
	return _array;
      }
      inline ValuesWrapper& GetExtend (CS::ShaderVarStringID n) 
      { 
	size_t candidate;
	size_t index = _array.FindSortedKey (csArrayCmp<Entry, CS::StringIDValue> (n),
	  &candidate);
	if (index != csArrayItemNotFound) return _array[index].v;
	Entry newEntry;
	newEntry.n = n;
	_array.Insert (candidate, newEntry);
	return _array[candidate].v;
      }
      inline size_t GetSize () const { return _array.GetSize (); }
      inline ValuesWrapper& GetIndex (size_t n) { return _array.Get (n).v; }
      inline const ValuesWrapper& GetIndex (size_t n) const { return _array.Get (n).v; }
      inline CS::ShaderVarStringID GetIndexName (size_t n) const { return _array.Get (n).n; }
      inline ValuesWrapper& Get (CS::ShaderVarStringID n)
      { 
	size_t index = _array.FindSortedKey (csArrayCmp<Entry, CS::StringIDValue> (n));
	return _array[index].v;
      }
      inline const ValuesWrapper& Get (CS::ShaderVarStringID n) const 
      { 
	size_t index = _array.FindSortedKey (csArrayCmp<Entry, CS::StringIDValue> (n));
	return _array[index].v;
      }
      inline bool Has (CS::ShaderVarStringID n) const
      { 
	size_t index = _array.FindSortedKey (csArrayCmp<Entry, CS::StringIDValue> (n));
	return index != csArrayItemNotFound;
      }
      inline const Entry& operator[] (size_t n) const { return _array[n]; }
      inline void Delete (CS::ShaderVarStringID n)
      {
	size_t index = _array.FindSortedKey (csArrayCmp<Entry, CS::StringIDValue> (n));
	_array.DeleteIndex (index);
      }
      inline void DeleteIndex (size_t index) { _array.DeleteIndex (index); }
    };
    class CowBlockAllocator;
    typedef CS::CowWrapper<ValuesArray, CowBlockAllocator> ValuesArrayWrapper;
    class CowBlockAllocator
    {
    public:
      typedef CS::Memory::FixedSizeAllocatorSafe<ValuesArrayWrapper::allocSize, 
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
    
    static void OrMergeValuesInto (ValuesWrapper& dst, const ValuesWrapper& src)
    {
       // Subarrays may not have all items defined.
      if (dst == 0) return;
      if (src == 0)
      {
	dst.AsRef().Invalidate();
	return;
      }
      
      *dst |= *src;
      if (!dst.HasSubValues()) return;
      size_t numIndices = csMin (dst.GetSubValues()->GetSize(), src.GetSubValues()->GetSize());
      for (size_t s = 0; s < numIndices; s++)
      {
	OrMergeValuesInto (dst.GetSubValues()->Get (s), src.GetSubValues()->Get (s));
      }
      while (dst.GetSubValues()->GetSize() > numIndices)
	dst.GetSubValues()->DeleteIndexFast (numIndices);
    }
    static void AndMergeValuesInto (ValuesWrapper& dst, const ValuesWrapper& src)
    {
       // Subarrays may not have all items defined.
      if (dst == 0)
      {
	dst = src;
	return;
      }
      if (src == 0) return;
      
      *dst &= *src;
      if (!src.HasSubValues()) return;
      size_t numIndices = csMin (dst.GetSubValues()->GetSize(), src.GetSubValues()->GetSize());
      for (size_t s = 0; s < numIndices; s++)
      {
	AndMergeValuesInto (dst.GetSubValues()->Get (s), src.GetSubValues()->Get (s));
      }
      for (size_t s = numIndices; s < src.GetSubValues()->GetSize(); s++)
      {
	dst.GetSubValues()->Push (src.GetSubValues()->Get (s));
      }
    }
  public:
    static void Init() { Values::ValChainAllocInit(); ValAllocInit(); }

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

    Values* GetValues (CS::ShaderVarStringID variable,
		       size_t numIndices, const size_t* indices)
    {
      const size_t* ip = indices;
      ValuesWrapper* vals = &(possibleValues->GetExtend (variable));
      do
      {
	if (!vals->AsRef().IsValid()) vals->AsRef().AttachNew (ValAlloc().Alloc());
	if (numIndices > 0)
	  vals = &(vals->GetSubValues()->GetExtend (*ip++));
      }
      while (numIndices-- > 0);
      return vals->AsRef();
    }
    const Values* GetValues (CS::ShaderVarStringID variable,
			     size_t numIndices, const size_t* indices) const
    {
      const size_t* ip = indices;
      const ValuesWrapper* vals = 0;
      if (possibleValues->Has (variable))
      {
	vals = &(possibleValues->Get (variable));
	while ((vals != 0) && (numIndices-- > 0))
	{
	  const ValuesWrapper* newVals = 0;
	  size_t subInd = *ip++;
	  if (vals->GetSubValues()
	     && (vals->GetSubValues()->GetSize() > subInd))
	    newVals = &(vals->GetSubValues()->Get (subInd));
	  vals = newVals;
	}
      }
      if ((vals != 0) && (*vals != 0))
	return *vals;
      else
	return Def();
    }
    void Dump (csString& out) const
    {
      for (size_t n = 0; n < possibleValues->GetSize(); n++)
      {
	const Values* vals = possibleValues->GetIndex (n);
	if (vals == 0) continue;
	out.AppendFmt ("var %lu: ", 
	  (unsigned long)possibleValues->GetIndexName (n));
	vals->Dump (out);
      }
    }
    friend Variables operator| (const Variables& a, const Variables& b)
    {
      /* "or":
       * If no Values are available it means all values are possible.
       * So if Values are in a but not in b, remove from a.
       * If Values are not in a they'll have all possible values anyway,
       *  don"t bother checking.
       * If Values are in both, merge.
       */
      Variables newVars (a);
      ValuesArray& nva = *newVars.possibleValues;
      const ValuesArray& pvB = *b.possibleValues;
      for (size_t n = 0; n < nva.GetSize(); )
      {
	CS::ShaderVarStringID name = nva[n].n;
	if (pvB.Has (name))
	{
	  const ValuesWrapper& entry = pvB.Get (name);
	  ValuesWrapper& vw = nva.GetIndex (n);
	  OrMergeValuesInto (vw, entry);
	  n++;
	}
	else
	  nva.DeleteIndex (n);
      }
      return newVars;
    }
    friend Variables operator& (const Variables& a, const Variables& b)
    {
      /* "and":
       * If no Values are available it means all values are possible.
       * So if Values are in b but not in a, copy from b to a
       * If Values are not in b leave value in a untouched.
       * If Values are in both, merge.
       */
      Variables newVars (a);
      ValuesArray& nva = *newVars.possibleValues;
      const ValuesArray& pvB = *b.possibleValues;
      for (size_t n = 0; n < pvB.GetSize(); n++)
      {
	const ValuesArray::Entry& entry = pvB[n];
	CS::ShaderVarStringID name = entry.n;
	if (nva.Has (name))
	{
	  ValuesWrapper& vw = nva.Get (name);
	  AndMergeValuesInto (vw, entry.v);
	}
	else
	{
	  ValuesWrapper& vw = nva.GetExtend (name);
	  vw = entry.v;
	}
      }
      return newVars;
    }
  };

  #include "csutil/custom_new_enable.h"
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_VAR_H__
