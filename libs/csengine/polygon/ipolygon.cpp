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

#include "sysdef.h"
#include "cscom/com.h"
#include "csengine/polygon.h"
#include "csengine/polytext.h"
#include "csengine/lghtmap.h"
#include "csengine/polyset.h"
#include "csobject/nameobj.h"
#include "ipolygon.h"

IMPLEMENT_COMPOSITE_UNKNOWN( csPolygonSet, PolygonSet )

STDMETHODIMP IPolygonSet::GetName(const char** szName)
{
    METHOD_PROLOGUE( csPolygonSet, PolygonSet );
    *szName = csNameObject::GetName(*pThis);
    return S_OK;
}

IMPLEMENT_COMPOSITE_UNKNOWN( csPolygon3D, Polygon3D )

STDMETHODIMP IPolygon3D::GetName(const char** szName)
{
    METHOD_PROLOGUE( csPolygon3D, Polygon3D );
    *szName = csNameObject::GetName(*pThis);
    return S_OK;
}

STDMETHODIMP IPolygon3D::GetParent(IPolygonSet** retval)
{
    METHOD_PROLOGUE( csPolygon3D, Polygon3D );
    csPolygonSet* pPolySet = (csPolygonSet*)pThis->GetParent ();
    
	*retval = GetIPolygonSetFromcsPolygonSet(pPolySet);
	(*retval)->AddRef();

    return S_OK;
}

STDMETHODIMP IPolygon3D::GetLightMap(ILightMap** retval)
{
	METHOD_PROLOGUE( csPolygon3D, Polygon3D );
	if (pThis->lightmap)
	{
	  *retval = GetILightMapFromcsLightMap (pThis->lightmap);
	  (*retval)->AddRef();
	}
	else *retval = NULL;

	return S_OK;
}

STDMETHODIMP IPolygon3D::GetAlpha(int& retval)
{
	METHOD_PROLOGUE( csPolygon3D, Polygon3D );
	retval = pThis->GetAlpha ();
	return S_OK;
}

STDMETHODIMP IPolygon3D::UsesMipMaps(void)
{
	METHOD_PROLOGUE( csPolygon3D, Polygon3D );
	return pThis->CheckFlags (CS_POLY_MIPMAP) ? S_OK : S_FALSE;
}

STDMETHODIMP IPolygon3D::GetTexture(int mipmap, IPolygonTexture** retval)
{
	METHOD_PROLOGUE( csPolygon3D, Polygon3D );
	*retval = GetIPolygonTextureFromcsPolyTexture(pThis->GetPolyTex (mipmap));
	(*retval)->AddRef();

	return S_OK;
}

STDMETHODIMP IPolygon3D::GetCameraVector(int v, ComcsVector3* retval)
{
	METHOD_PROLOGUE( csPolygon3D, Polygon3D );
	csVector3 v3 = pThis->Vcam(v);

	//ASSERT( retval );
	retval->x = v3.x;
	retval->y = v3.y;
	retval->z = v3.z;

	return S_OK;
}


// IPolygonTexture implementation for csPolyTexture

IMPLEMENT_COMPOSITE_UNKNOWN( csPolyTexture, PolygonTexture )

IMPLEMENT_GET_PROPERTY( GetFDU, fdu, float, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetFDV, fdv, float, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetWidth, w, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetHeight, h, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetMipmapLevel, mipmap_level, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetSize, size, long, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetIMinU, Imin_u, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetIMinV, Imin_v, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetMipMapSize, mipmap_size, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetMipMapShift, mipmap_shift, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetNumPixels, size, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetOriginalWidth, w_orig, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetShiftU, shf_u, int, csPolyTexture, PolygonTexture )
IMPLEMENT_GET_PROPERTY( GetNumberDirtySubTex, CountDirtySubtextures(), int, csPolyTexture, PolygonTexture ) 
IMPLEMENT_GET_PROPERTY( GetSubtexSize, subtex_size, int, csPolyTexture, PolygonTexture ) 
IMPLEMENT_GET_PROPERTY( GetDynlightOpt, subtex_dynlight, bool, csPolyTexture, PolygonTexture ) 

IMPLEMENT_METHOD( CreateDirtyMatrix, CreateDirtyMatrix, csPolyTexture, PolygonTexture )
IMPLEMENT_METHOD( MakeAllDirty, MakeAllDirty, csPolyTexture, PolygonTexture )

STDMETHODIMP IPolygonTexture::CleanIfDirty (int lu, int lv, bool& retval)
{
  METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
  int idx = pThis->dirty_w*lv + lu;
  retval = (bool)(pThis->dirty_matrix[idx]);
  if (retval)
  {
    pThis->dirty_matrix[idx] = 0;
    pThis->dirty_cnt--;
  }
  return S_OK;
}

STDMETHODIMP IPolygonTexture::RecalculateDynamicLights(bool& recalc)
{
  METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
  recalc = pThis->RecalcDynamicLights ();
  return S_OK;
}

STDMETHODIMP IPolygonTexture::GetLightMap(ILightMap** retval)
{
	METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
	if (pThis->lm != NULL)
	{
		*retval = GetILightMapFromcsLightMap (pThis->lm);
		(*retval)->AddRef();
	}
	else *retval = NULL;

	return S_OK;
}

STDMETHODIMP IPolygonTexture::SetTCacheData(void* newval)
{
	METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
	pThis->tcache_data = newval;
	return S_OK;
}

STDMETHODIMP IPolygonTexture::GetTCacheData(void** retval)
{
	METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
	*retval = pThis->tcache_data;
	return S_OK;
}

STDMETHODIMP IPolygonTexture::GetPolygon(IPolygon3D** retval)
{
	METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
	*retval = GetIPolygon3DFromcsPolygon3D( pThis->polygon );
	(*retval)->AddRef();

	return S_OK;
}

STDMETHODIMP IPolygonTexture::GetTextureHandle (ITextureHandle** retval)
{
  METHOD_PROLOGUE (csPolyTexture, PolygonTexture);
  *retval = pThis->polygon->GetTextureHandle ();
  return S_OK;
}

STDMETHODIMP IPolygonTexture::GetTextureBox( float& fMinU, float& fMinV, float& fMaxU, float& fMaxV )
{
	METHOD_PROLOGUE( csPolyTexture, PolygonTexture );
	
	fMinU = pThis->Fmin_u;
	fMaxU = pThis->Fmax_u;
	fMinV = pThis->Fmin_v;
	fMaxV = pThis->Fmax_v;

	return S_OK;
}

