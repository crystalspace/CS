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

csSymbolTable::csSymbolTable (const csSymbolTable &other, int size)
 : Hash (size), Parent (0)
{
  csGlobalHashIterator iter (&other.Hash);
  while (iter.HasNext ())
  {
    Symbol *s = (Symbol *) iter.NextConst ();
    if (s->Auth)
      Hash.Put (s->Name, (csHashObject) new Symbol (s->Name, s->Val, true));
  }
}

csSymbolTable::~csSymbolTable ()
{
  csGlobalHashIterator iter (&Hash); 
  while (iter.HasNext ()) 
    delete (Symbol *) iter.Next (); 
}

inline void csSymbolTable::SetParent (csSymbolTable *p)
{ 
  Parent = p;
  csGlobalHashIterator i (& Parent->Hash);
  while (i.HasNext ())
  {
    Symbol *s = (Symbol *) i.Next ();
    SetSymbolSafe (s->Name, s->Val);
  }
}

inline void csSymbolTable::PropagateSymbol (csStringID name, void *value)
{
  for (int i = 0; i < Children.Length (); i++)
    Children[i]->SetSymbolSafe (name, value);
}

inline void csSymbolTable::PropagateDelete (csStringID name)
{
  for (int i = 0; i < Children.Length (); i++)
    Children[i]->DeleteSymbolSafe (name);
}

inline void csSymbolTable::SetSymbolSafe (csStringID name, void *value)
{
  Symbol *sym = (Symbol *) Hash.Get (name);
  if (sym)
  {
    if (! sym->Auth)
      SetSymbol (name, value);//@@@ Wasteful: SetSymbol will do Hash.Get again
  }
  else
    SetSymbol (name, value);//@@@ Wasteful: SetSymbol will do Hash.Get again
}

inline void csSymbolTable::DeleteSymbolSafe (csStringID name)
{
  Symbol *sym = (Symbol *) Hash.Get (name);
  if (sym)
  {
    if (! sym->Auth)
      DeleteSymbol (name);//@@@ Wasteful: DeleteSymbol will do Hash.Get again
  }
  else
    DeleteSymbol (name);//@@@ Wasteful: DeleteSymbol will do Hash.Get again
}

void csSymbolTable::AddChild (csSymbolTable *child)
{
  Children.Push (child);
  child->SetParent (this);
}

void csSymbolTable::AddChildren (csArray<csSymbolTable*> &children)
{
  for (int i = 0; i < children.Length (); i++)
  {
    csSymbolTable *child = children[i];
    Children.Push (child);
    child->SetParent (this);
  }
}

void csSymbolTable::SetSymbol (csStringID name, void *value)
{
  Symbol *s = (Symbol *) Hash.Get (name);
  if (s)
  {
    s->Val = value;
    s->Auth = true;
  }
  else
    Hash.Put (name, (csHashObject) new Symbol (name, value, true));

  PropagateSymbol (name, value);
}

void csSymbolTable::SetSymbols (const csArray<csStringID> &names, csArray<void*> &values)
{
  for (int i = 0; i < names.Length (); i++)
  {
    csStringID name = names[i];
    void *value = values[i];

    Symbol *s = (Symbol *) Hash.Get (name);
    if (s)
    {
      s->Val = value;
      s->Auth = true;
    }
    else
      Hash.Put (name, (csHashObject) new Symbol (name, value, true));

    PropagateSymbol (name, value);
  }
}

bool csSymbolTable::DeleteSymbol (csStringID name)
{
  Symbol *s = (Symbol *) Hash.Get (name);
  if (s && s->Auth)
  {
    Hash.DeleteAll (name);
    PropagateDelete (name);
    return true;
  }
  return false;
}

bool csSymbolTable::DeleteSymbols (const csArray<csStringID> &names)
{
  bool ok = true;
  for (int i = 0; i < names.Length (); i++)
  {
    csStringID name = names[i];
    Symbol *s = (Symbol *) Hash.Get (name);
    if (s && s->Auth)
    {
      Hash.DeleteAll (name);
      PropagateDelete (name);
    }
    else
      ok = false;
  }
  return ok;
}

void* csSymbolTable::GetSymbol (csStringID name)
{
  Symbol *s = (Symbol *) Hash.Get (name);
  if (s) return s->Val;
  else return 0;
}

csArray<void*> csSymbolTable::GetSymbols (const csArray<csStringID> &names)
{
  csArray<void*> values;
  for (int i = 0; i < names.Length (); i++)
  {
    csStringID name = names[i];
    Symbol *s = (Symbol *) Hash.Get (name);
    if (s) values.Push (s->Val);
    else values.Push (0);
  }
  return values;
}

bool csSymbolTable::SymbolExists (csStringID name)
{
  return Hash.Get (name);
}

bool csSymbolTable::SymbolsExist (const csArray<csStringID> &names)
{
  for (int i = 0; i < names.Length (); i++)
    if (! Hash.Get (names[i])) return false;
  return true;
}
