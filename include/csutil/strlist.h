/*
    Copyright (C) 1998 by Jorrit Tyberghein
		CSScript module created by Brandon Ehle (Azverkan)
  
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

#ifndef __STRLIST_H__
#define __STRLIST_H__

#include "csutil/csstring.h"

#define csSTRList csStringList

class csStringList:private csVector {
public:
	int Add(csSTR& Name) { return Push(new csSTR(Name)); }
	int Add(const char *Name) { return Push(new csSTR(Name)); }

	void Delete(int n) { delete (csSTR*)csVector::Get(n); }
	void Clear() { DeleteAll(); }

	csSTR& Get(int index) { return *((csSTR*)(csVector::Get(index))); }
	csSTR& operator[](int index) { return Get(index); }

	int Length() { return csVector::Length(); }
};

#endif
