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

#ifndef __CSSDEFS_H__
#define __CSSDEFS_H__

/// Generates a default Get<interface>() function
#define QUERY_DERIVED(ClassName) \
ClassName* Get##ClassName () { \
	ClassName *i=NULL; \
	QueryInterface(IID_##ClassName, (void**)&i); \
	return i; \
}

#define DEFAULT_COM(ClassName) \
	DECLARE_IUNKNOWN(); \
  DECLARE_INTERFACE_TABLE(cs##ClassName); \
	QUERY_DERIVED(I##ClassName);

#define IMPLEMENT_DEFAULT_COM(ClassName) \
	IMPLEMENT_UNKNOWN(cs##ClassName) \
	BEGIN_INTERFACE_TABLE(cs##ClassName) \
	END_INTERFACE_TABLE()

#define GET_PARENT(ClassName, a)  ((cs##ClassName*)((size_t)a - offsetof(cs##ClassName, m_x##ClassName)))

#endif
