/*
    Crystal Space Virtual File System SCF interface
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include "sysdef.h"
#include "csutil/scfstrv.h"

scfStrVector::scfStrVector (int ilimit, int ithreshold) :
  csStrVector (ilimit, ithreshold) { CONSTRUCT_IBASE(NULL); }
int scfStrVector::Length() const { return superclass::Length(); }
char* scfStrVector::Get (int n) const { return (char*)superclass::Get(n); }
int scfStrVector::Find (const char* s) const
  { return superclass::Find(csSome(s)); }
int scfStrVector::Push (char* s) { return superclass::Push(s); }
char* scfStrVector::Pop() { return (char*)superclass::Pop(); }
