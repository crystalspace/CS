/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/symtable.h"
#include "csgfx/shadervar.h"

csSymbolTable::csSymbolTable (const csSymbolTable &other, int size)
  : Hash (size), Parent (0)
{
  csGlobalHashIterator iter (& other.Hash);
  while (iter.HasNext ())
  {
    Stack *s = (Stack *) iter.NextConst ();
    if (s->Vals[0].Owner == & other)
    {
      Hash.Put (s->Name,
        (csHashObject) new Stack (s->Name, s->Vals[0].Val, this));
      return;
    }
  }
}

csSymbolTable::~csSymbolTable ()
{
  csGlobalHashIterator iter (& Hash); 
  while (iter.HasNext ()) delete (Stack *) iter.Next (); 
}

inline csSymbolTable::Stack::Stack (csStringID id) : Vals (0, 1), Name (id) {}

inline csSymbolTable::Stack::Stack (csStringID id, const csArray<Symbol> &vals)
  : Vals (vals), Name (id) {}

inline csSymbolTable::Stack::Stack (csStringID id, csShaderVariable *value,
  csSymbolTable *owner) : Vals (1, 1), Name (id)
{
  Vals.Push (Symbol (value, owner));
}

inline csSymbolTable::Stack::Symbol::Symbol (csShaderVariable *value,
  csSymbolTable *owner) : Owner (owner), Val (value) {}

inline csSymbolTable::Stack::Symbol::Symbol (const Symbol &other)
  : Owner (other.Owner), Val (other.Val) {}

void csSymbolTable::AddChild (csSymbolTable *child)
{
  Children.Push (child);
  child->Parent = this;
  csGlobalHashIterator i (& Hash);
  while (i.HasNext ())
  {
    Stack *src = (Stack *) i.Next ();
    Stack *dst = (Stack *) child->Hash.Get (src->Name);
    if (dst)
      for (int i = 0; i < src->Vals.Length (); i++)
        dst->Vals.Push (Stack::Symbol (src->Vals[i]));
    else
      child->Hash.Put (src->Name,
        (csHashObject) (dst = new Stack (src->Name, src->Vals)));
    child->Propagate (dst);
  }
}

void csSymbolTable::AddChildren (csArray<csSymbolTable*> &children)
{
  for (int i = 0; i < children.Length (); i++)
    AddChild (children[i]);
}

csArray<csSymbolTable *> csSymbolTable::GetChildren ()
{
  return Children;
}

void csSymbolTable::Propagate (const Stack *stack)
{
  for (int i = 0; i < Children.Length (); i++)
  {
    Stack *child = (Stack *) Children[i]->Hash.Get (stack->Name);
    if (child)
    {
      if (child->Vals[0].Owner == Children[i]) child->Vals.Truncate (1);
      else child->Vals.Empty ();
    }
    else Children[i]->Hash.Put (stack->Name, child = new Stack (stack->Name));

    child->Vals.SetCapacity (child->Vals.Length () + stack->Vals.Length ());
    for (int j = 0; j < stack->Vals.Length (); j++)
      child->Vals.Push (stack->Vals[j]);

    Children[i]->Propagate (child);
  }
}

void csSymbolTable::SetSymbol (csStringID name, csShaderVariable *value)
{
  Stack *s = (Stack *) Hash.Get (name);
  if (s)
  {
    if (s->Vals[0].Owner == this)
      s->Vals[0].Val = value;
    else
      s->Vals.Insert (0, Stack::Symbol (value, this));
  }
  else
  {
    s = new Stack (name, value, this);
    Hash.Put (name, s);
  }
  Propagate (s);
}

void csSymbolTable::SetSymbols (const csArray<csStringID> &names,
  csArray<csShaderVariable *> &values)
{
  CS_ASSERT (names.Length () == values.Length ());
  for (int i = 0; i < names.Length (); i++)
    SetSymbol (names[i], values[i]);
}

bool csSymbolTable::DeleteSymbol (csStringID name)
{
  Stack *s = (Stack *) Hash.Get (name);
  if (s && s->Vals[0].Owner == this)
  {
    if (s->Vals.Length () <= 1)
      Hash.DeleteAll (name);
    else
      s->Vals.DeleteIndex (0);

    Propagate (s);
    return true;
  }
  else return false;
}

bool csSymbolTable::DeleteSymbols (const csArray<csStringID> &names)
{
  bool ok = true;
  for (int i = 0; i < names.Length (); i++)
    if (! DeleteSymbol (names[i])) ok = false;
  return ok;
}

csShaderVariable* csSymbolTable::GetSymbol (csStringID name)
{
  Stack *s = (Stack *) Hash.Get (name);
  if (s) return s->Vals.Top ().Val;
  else return 0;
}

csArray<csShaderVariable *> csSymbolTable::GetSymbols
  (const csArray<csStringID> &names)
{
  csArray<csShaderVariable *> values;
  for (int i = 0; i < names.Length (); i++)
  {
    Stack *s = (Stack *) Hash.Get (names[i]);
    if (s) values.Push (s->Vals.Top ().Val);
    else values.Push (0);
  }
  return values;
}

csArray<csShaderVariable *> csSymbolTable::GetSymbols ()
{
  csArray<csShaderVariable *> values;
  csGlobalHashIterator iter (& Hash);
  while (iter.HasNext ())
    values.Push (((Stack *) iter.Next ())->Vals.Top ().Val);
  return values;
}

bool csSymbolTable::SymbolExists (csStringID name) const
{
  return Hash.Get (name);
}

bool csSymbolTable::SymbolsExist (const csArray<csStringID> &names) const
{
  for (int i = 0; i < names.Length (); i++)
    if (! Hash.Get (names[i])) return false;
  return true;
}
