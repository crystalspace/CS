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

// ILightMap interface

#include "sysdef.h"
#include "csengine/light/lghtmap.h"
#include "ilghtmap.h"

IMPLEMENT_COMPOSITE_UNKNOWN( csLightMap, LightMap )

STDMETHODIMP ILightMap::GetMap(int nMap, unsigned char** ppResult)
{
    METHOD_PROLOGUE( csLightMap, LightMap )
        
    switch (nMap)
    {
        case 0:
            *ppResult = pThis->GetRealMap ().mapR;
            break;
            
        case 1:
            *ppResult = pThis->GetRealMap ().mapG;
            break;
            
        case 2:
            *ppResult = pThis->GetRealMap ().mapB;
            break;
            
        default:
            //ASSERT(FALSE)
	    ;
    }
    
    return S_OK;
}

IMPLEMENT_GET_PROPERTY( GetWidth, GetWidth (), int, csLightMap, LightMap ) 
IMPLEMENT_GET_PROPERTY( GetHeight, GetHeight (), int, csLightMap, LightMap )
IMPLEMENT_GET_PROPERTY( GetRealWidth, GetRealWidth (), int, csLightMap, LightMap ) 
IMPLEMENT_GET_PROPERTY( GetRealHeight, GetRealHeight (), int, csLightMap, LightMap )
IMPLEMENT_GET_PROPERTY( GetInVideoMemory, in_memory, bool, csLightMap, LightMap )
IMPLEMENT_SET_PROPERTY( SetInVideoMemory, in_memory, bool, csLightMap, LightMap )

IMPLEMENT_GET_PROPERTY_PTR( GetHighColorCache, hicolorcache, HighColorCache_Data*, csLightMap, LightMap )
IMPLEMENT_SET_PROPERTY( SetHighColorCache, hicolorcache, HighColorCache_Data*, csLightMap, LightMap )


