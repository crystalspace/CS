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

#include "csutil/csstring.h"

NO_MODULE_LOCK(a)
IMPLEMENT_UNKNOWN(csString)

BEGIN_INTERFACE_TABLE(csString)
	IMPLEMENTS_INTERFACE(IString)
END_INTERFACE_TABLE()

STDMETHODIMP csString::xData(DATA **data) {
	*data=Data;
	return S_OK;
}

STDMETHODIMP csString::xLength(unsigned long int *size) {
	*size=Size;
	return S_OK;
}

