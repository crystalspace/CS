/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __SARRAY_H__
#define __SARRAY_H__

#define CS_DECLARE_STATIC_ARRAY(Name,Type)				\
class Name								\
{									\
private:								\
  Type *Map;								\
  int Size;								\
public:									\
  inline Name (int s = 0)						\
    { Map = NULL; Size = 0; Alloc (s); }				\
  inline ~Name ()							\
    { Clear (); }							\
  inline int GetSize () const						\
    { return Size; }							\
  inline Type *GetArray ()						\
    { return Map; }							\
  inline void Clear ()							\
    { if (Map) delete[] Map; Map = NULL; Size = 0; }			\
  inline void Alloc (int s)						\
    { if (Size == s) return; Clear ();					\
      if (s>0) { Map = new Type [s]; Size = s; }			\
    }									\
  inline void Copy (const Name *other)					\
    { Alloc (other->Size);						\
      memcpy (Map, other->Map, sizeof(Type) * Size); }			\
  inline void TakeOver (Name *other, bool DeleteOld = true)		\
    {									\
      TakeOver (other->Map, other->Size, DeleteOld);			\
      other->Map = NULL;						\
      other->Size = 0;							\
    }									\
  inline void TakeOver (Type *NewData, int NewSize, bool DeleteOld = true)	\
    {									\
      if (DeleteOld) Clear ();						\
      Map = NewData;							\
      Size = NewSize;							\
    }									\
}

#endif // __SARRAY_H__
