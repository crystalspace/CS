/*
  Copyright (C) 2008 by Julian Mautner

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this application; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSCLOUDRENDERER_PLUGIN_H__
#define __CSCLOUDRENDERER_PLUGIN_H__

#include "imesh/clouds.h"
#include <csutil/array.h>
#include <csgeom/vector3.h>

template <typename T>
class csField3 : public scfImplementation1<csField3<T>, iField3<T>>
{
private:
	csArray<csArray<csArray<T>>>		m_aaaArray;
	UINT								m_iSizeX;
	UINT								m_iSizeY;
	UINT								m_iSizeZ;

public:
	csField3<T>(iBase* pParent);
	~csField3<T>();

	virtual inline void SetSize(const UINT iSizeX, const UINT iSizeY, const UINT iSizeZ)
	{

	}
	virtual const UINT GetSizeX() const {return m_iSizeX;}
	virtual const UINT GetSizeY() const {return m_iSizeY;}
	virtual const UINT GetSizeZ() const {return m_iSizeZ;}

	//O(1)
	virtual inline void SetValue(const T& Value, const UINT x, const UINT y, const UINT z)
	{
		m_aaaArray[x][y][z] = Value;
	}

	//O(1)
	virtual inline const T operator () (const UINT x, const UINT y, const UINT z) const
	{
		return GetValue(x, y, z);
	}
	virtual inline const T GetValue(const UINT x, const UINT y, const UINT z) const
	{
		return m_aaaArray[x][y][z];
	}
};

//------------------------------------------------------------------------------//

inline const csVector3 Clamp(const csVector3& vPos, const UINT x, const UINT y, const UINT z)
{
	csVector3 vNew = vPos;
	if(vNew.x < 0.f) vNew.x = 0.f;
	if(vNew.y < 0.f) vNew.y = 0.f;
	if(vNew.z < 0.f) vNew.z = 0.f;
	if(static_cast<UINT>(vNew.x) >= x) vNew.x = static_cast<double>(x - 1);
	if(static_cast<UINT>(vNew.y) >= y) vNew.y = static_cast<double>(y - 1);
	if(static_cast<UINT>(vNew.z) >= z) vNew.z = static_cast<double>(z - 1);
	return vNew;
}

//------------------------------------------------------------------------------//

template <typename T>
const T TrilinearInterpolation(const iField3<T>& rSrc, const csVector3& vPos)
{
	//TODO!
	return rSrc(static_cast<UINT>(vPos.x), static_cast<UINT>(vPos.y), static_cast<UINT>(vPos.z));
}

//------------------------------------------------------------------------------//

//Advection of a field through a velocity-field
//O(n³)
template <typename T>
const bool SemiLagrangianAdvection(const iField3<T>& rSrc, iField3<T> pDest, const iField3<csVector3>& rVelField,
								   const double& dGridScaleInv, const double& dTimeStep)
{
	//Check if precondition are hold: All field have the same size!
	if(rSrc.GetSizeX() != pDest.GetSizeX() || rSrc.GetSizeX() != rVelField.GetSizeX() ||
	   rSrc.GetSizeY() != pDest.GetSizeY() || rSrc.GetSizeY() != rVelField.GetSizeY() ||
	   rSrc.GetSizeZ() != pDest.GetSizeZ() || rSrc.GetSizeZ() != rVelField.GetSizeZ()) return false;

	for(UINT x = 0; x < pDest.GetSizeX(); ++x)
	{
		for(UINT y = 0; y < pDest.GetSizeY(); ++y)
		{
			for(UINT z = 0; z < pDest.GetSizeZ(); ++z)
			{
				const csVector3 vCurrPos	= csVector3(x, y, z);
				const csVector3 vPos		= Clamp(vCurrPos - dGridScaleInv * dTimeStep * rVelField(x, y, z),
													pDest->GetSizeX(), pDest->GetSizeY(), pDest->GetSizeZ());
				pDest.SetValue(TrilinearInterpolation(rSrc, vPos), x, y, z);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------//

#endif // __CSCLOUDRENDERER_PLUGIN_H__