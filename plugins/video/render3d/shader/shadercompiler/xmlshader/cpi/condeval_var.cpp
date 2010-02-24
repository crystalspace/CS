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

#include "condeval_var.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  CS_IMPLEMENT_STATIC_CLASSVAR_REF(Variables::Values, def,
    Def, ValueSet, ());
  IMPLEMENT_STATIC_CLASSVAR_DIRECT(Variables::Values, ValChainAlloc);

  void Variables::Values::ValChainKill()
  {
    ValChainAlloc().~csBlockAllocator();
  }

  ValueSet& Variables::Values::GetMultiValue (uint num)
  {
    ValueSetChain* p = multiValues;
    while (num-- > 0) p = p->nextPlease;
    return p->vs;
  }

  const ValueSet& Variables::Values::GetMultiValue (uint num) const
  {
    ValueSetChain* p = multiValues;
    while (num-- > 0) p = p->nextPlease;
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
	*d = ValChainAlloc().Alloc();
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

  IMPLEMENT_STATIC_CLASSVAR_DIRECT(Variables, ValAlloc);
  void Variables::ValAllocKill()
  {
    ValAlloc().~ValBlockAlloc();
  }

  CS_IMPLEMENT_STATIC_CLASSVAR(Variables, def,
    Def, Variables::Values, ());
  CS_IMPLEMENT_STATIC_CLASSVAR_REF(Variables::CowBlockAllocator, 
    allocator, Allocator, Variables::CowBlockAllocator::BlockAlloc, (256));

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
