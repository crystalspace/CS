/*
    Crystal Space Shared String Vector class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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
#include "csutil/scfstrv.h"

SCF_IMPLEMENT_IBASE (scfStrVector)
  SCF_IMPLEMENTS_INTERFACE (iStrVector)
SCF_IMPLEMENT_IBASE_END

int scfStrVector::Length () const
{ return v.Length (); }

void scfStrVector::Push (char *iValue)
{ v.Push (iValue); }

char *scfStrVector::Pop ()
{ return (char *)v.Pop (); }

char *scfStrVector::Get (int iIndex) const
{ return (char *)v.Get (iIndex); }

int scfStrVector::Find (const char *iValue) const
{ return v.FindKey (iValue); }

int scfStrVector::FindSorted (const char *iValue) const
{ return v.FindSortedKey ((csConstSome)iValue); }

void scfStrVector::QuickSort ()
{ v.QuickSort (); }

void scfStrVector::Delete (int iIndex)
{ v.Delete (iIndex); }

void scfStrVector::Insert (int iIndex, char *iValue)
{ v.Insert (iIndex, iValue); }

void scfStrVector::DeleteAll ()
{ v.DeleteAll (); }
