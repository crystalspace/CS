/*
    Crystal Space String interface
    Copyright (C) 1999 by Brandon Ehle (Azverkan)

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
#include "csutil/scfstr.h"

SCF_IMPLEMENT_IBASE (scfString)
  SCF_IMPLEMENTS_INTERFACE (iString)
SCF_IMPLEMENT_IBASE_END

void scfString::SetCapacity (size_t NewSize)
{ s.SetCapacity (NewSize); }

size_t scfString::GetCapacity() const
{ return s.GetCapacity(); }

void scfString::SetGrowsBy(size_t n)
{ s.SetGrowsBy(n); }

size_t scfString::GetGrowsBy() const
{ return s.GetGrowsBy(); }

void scfString::Truncate (size_t iPos)
{ s.Truncate (iPos); }

void scfString::ShrinkBestFit ()
{ s.Reclaim (); }

void scfString::Empty ()
{ s.Clear (); }

csRef<iString> scfString::Clone () const
{ return csPtr<iString>(new scfString (*this)); }

char const* scfString::GetData () const
{ return s.GetData (); }

char* scfString::GetData ()
{ return CS_CONST_CAST(char*, s.GetData()); }

size_t scfString::Length () const
{ return s.Length (); }

bool scfString::IsEmpty () const
{ return !Length (); }

char& scfString::operator [] (size_t iPos)
{ return s[iPos]; }

char scfString::operator [] (size_t iPos) const
{ return s[iPos]; }

void scfString::SetAt (size_t iPos, char iChar)
{ s.SetAt (iPos, iChar); }

char scfString::GetAt (size_t iPos) const
{ return s.GetAt (iPos); }

void scfString::Insert (size_t iPos, iString const* iStr)
{ s.Insert (iPos, iStr->GetData ()); }

void scfString::Overwrite (size_t iPos, iString const* iStr)
{ s.Overwrite (iPos, iStr->GetData ()); }

void scfString::Append (const char* iStr, size_t iCount)
{ s.Append (iStr, iCount); }

void scfString::Append (iString const* iStr, size_t iCount)
{ s.Append (iStr->GetData (), iCount); }

csRef<iString> scfString::Slice(size_t start, size_t len) const
{
  csString const tmp(s.Slice(start, len));
  return csPtr<iString>(new scfString(tmp));
}

void scfString::SubString (iString* sub, size_t start, size_t len) const
{
  csString tmp;
  s.SubString(tmp, start, len);
  sub->Truncate(0);
  sub->Append(tmp.GetData(), tmp.Length());
}

size_t scfString::FindFirst (const char c, size_t p) const
{ return s.FindFirst(c, p); }

size_t scfString::FindLast (const char c, size_t p) const
{ return s.FindLast(c, p); }

size_t scfString::Find (const char* t, size_t p) const
{ return s.Find(t, p); }

void scfString::ReplaceAll (const char* search, const char* replacement)
{ s.ReplaceAll(search, replacement); }

void scfString::Format (const char* format, ...)
{
  va_list args;
  va_start (args, format);
  FormatV (format, args);
  va_end (args);
}

void scfString::FormatV (const char* format, va_list args)
{ s.FormatV (format, args); }

void scfString::Replace (const iString* iStr, size_t iCount)
{ s.Replace (iStr->GetData (), iCount); }

bool scfString::Compare (const iString* iStr) const
{ return s.Compare (iStr->GetData ()); }

bool scfString::CompareNoCase (const iString* iStr) const
{ return s.CompareNoCase (iStr->GetData ()); }

void scfString::operator += (const iString& iStr)
{ Append (&iStr); }

void scfString::operator += (const char* iStr)
{ Append (iStr); }

csRef<iString> scfString::operator + (const iString& iStr) const
{
  csRef<iString> tmp(Clone());
  tmp->Append(&iStr);
  return tmp;
}

scfString::operator char const* () const
{ return GetData (); }

bool scfString::operator == (const iString& iStr) const
{ return Compare (&iStr); }

bool scfString::operator != (const iString& iStr) const
{ return !Compare (&iStr); }

void scfString::Downcase()
{ s.Downcase(); }

void scfString::Upcase()
{ s.Upcase(); }
