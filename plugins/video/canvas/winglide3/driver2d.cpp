/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include <stdlib.h>
#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/winglide3/g2d.h"

static unsigned int gb_cRef = 0;

// Implementation of the csGraphics2DGlide3x factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE( csGraphics2DGlide3xFactory )

BEGIN_INTERFACE_TABLE( csGraphics2DGlide3xFactory )
	IMPLEMENTS_INTERFACE( IGraphics2DFactory )
END_INTERFACE_TABLE()

STDMETHODIMP csGraphics2DGlide3xFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
	if (!piSystem)
	{
		*ppv = 0;
		return E_INVALIDARG;
	}

	csGraphics2DGlide3x* pNew = new csGraphics2DGlide3x(piSystem);
	if (!pNew)
	{
		*ppv = 0;
		return E_OUTOFMEMORY;
	}
		
	return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DGlide3xFactory::LockServer(BOOL bLock)
{
	if (bLock)
		gb_cRef++;
	else
		gb_cRef--;

	return S_OK;
}
