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
#include <csgeom/vector3.h>

template <typename T>
class csField3 : public scfImplementation1<csField3<T>, iField3<T>>
{
private:
	T***			m_pppArray;
	UINT			m_iSizeX;
	UINT			m_iSizeY;
	UINT			m_iSizeZ;

	//O(n^2)
	inline void DeleteField()
	{
		if(!m_pppArray) return;
		for(UINT x = 0; x < m_iSizeX; ++x)
		{
			for(UINT y = 0; y < m_iSizeY; ++y) delete[] m_pppArray[x][y];
			delete[] m_pppArray[x];
		}
		delete[] m_pppArray;
		m_pppArray = NULL;
		m_iSizeX = m_iSizeY = m_iSizeZ = 0;
	}

public:
	csField3<T>(iBase* pParent) : m_pppArray(NULL), m_iSizeX(0), m_iSizeY(0), m_iSizeZ(0), 
		scfImplementationType(this, pParent)
	{}
	~csField3<T>()
	{
		DeleteField();
	}

	//O(n^2)
	virtual inline void SetSize(const UINT iSizeX, const UINT iSizeY, const UINT iSizeZ)
	{
		DeleteField();
		m_iSizeX = iSizeX;
		m_iSizeY = iSizeY;
		m_iSizeZ = iSizeZ;
		//reserve memory
		m_pppArray = new T**[m_iSizeX];
		for(UINT x = 0; x < m_iSizeX; ++x)
		{
			m_pppArray[x] = new T*[m_iSizeY];
			for(UINT y = 0; y < m_iSizeY; ++y)
			{
				m_pppArray[x][y] = new T[m_iSizeZ];
			}
		}
	}
	virtual const UINT GetSizeX() const {return m_iSizeX;}
	virtual const UINT GetSizeY() const {return m_iSizeY;}
	virtual const UINT GetSizeZ() const {return m_iSizeZ;}

	//O(1)
	virtual inline void SetValue(const T& Value, const UINT x, const UINT y, const UINT z)
	{
		if(m_pppArray) m_pppArray[x][y][z] = Value;
	}

	//O(1)
	virtual inline const T operator () (const UINT x, const UINT y, const UINT z) const
	{
		return GetValue(x, y, z);
	}
	//O(1)
	virtual inline const T GetValue(const UINT x, const UINT y, const UINT z) const
	{
		return m_pppArray[x][y][z];
	}
};

//------------------------------------------------------------------------------//

inline const csVector3 Clamp(const csVector3& vPos, const UINT x, const UINT y, const UINT z)
{
	csVector3 vNew = vPos;
	if(vNew.x < 0.f) vNew.x = 0.f;
	if(vNew.y < 0.f) vNew.y = 0.f;
	if(vNew.z < 0.f) vNew.z = 0.f;
	if(static_cast<UINT>(vNew.x) >= x) vNew.x = static_cast<float>(x - 1);
	if(static_cast<UINT>(vNew.y) >= y) vNew.y = static_cast<float>(y - 1);
	if(static_cast<UINT>(vNew.z) >= z) vNew.z = static_cast<float>(z - 1);
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
//O(n^3)
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

inline const csVector3 CalcGradient(const iField3<float>& rField, const UINT x, const UINT y, const UINT z,
									const float dx)
{
	const float dx2 = 2.f * dx;
	//Calcs partial derivations in x, y and z direction.
	//Some special cases are those for which x, y, z are border-indizes!
	const float fDerX = x + 1 < rField.GetSizeX() && x > 0 ? (rField.GetValue(x + 1, y, z) - rField.GetValue(x - 1, y, z)) / dx2 : x > 0 ? (rField.GetValue(x, y, z) - rField.GetValue(x - 1, y, z)) / dx : (rField.GetValue(x + 1, y, z) - rField.GetValue(x, y, z)) / dx;
	const float fDerY = y + 1 < rField.GetSizeY() && y > 0 ? (rField.GetValue(x, y + 1, z) - rField.GetValue(x, y - 1, z)) / dx2 : y > 0 ? (rField.GetValue(x, y, z) - rField.GetValue(x, y - 1, z)) / dx :	(rField.GetValue(x, y + 1, z) - rField.GetValue(x, y, z)) / dx;
	const float fDerZ = z + 1 < rField.GetSizeZ() && z > 0 ? (rField.GetValue(x, y, z + 1) - rField.GetValue(x, y, z - 1)) / dx2 : z > 0 ? (rField.GetValue(x, y, z) - rField.GetValue(x, y, z - 1)) / dx :	(rField.GetValue(x, y, z + 1) - rField.GetValue(x, y, z)) / dx;

	return csVector3(fDerX, fDerY, fDerZ);
}

//------------------------------------------------------------------------------//

inline const float CalcDivergence(const iField3<csVector3>& rField, const UINT x, const UINT y, const UINT z,
								  const float dx)
{
	const float dx2 = 2.f * dx;
	//Calcs partial derivations in x, y and z direction.
	//Some special cases are those for which x, y, z are border-indizes!
	const float fDerX = x + 1 < rField.GetSizeX() && x > 0 ? (rField.GetValue(x + 1, y, z).x - rField.GetValue(x - 1, y, z).x) / dx2 : x > 0 ? (rField.GetValue(x, y, z).x - rField.GetValue(x - 1, y, z).x) / dx : (rField.GetValue(x + 1, y, z).x - rField.GetValue(x, y, z).x) / dx;
	const float fDerY = y + 1 < rField.GetSizeY() && y > 0 ? (rField.GetValue(x, y + 1, z).y - rField.GetValue(x, y - 1, z).y) / dx2 : y > 0 ? (rField.GetValue(x, y, z).y - rField.GetValue(x, y - 1, z).y) / dx :	(rField.GetValue(x, y + 1, z).y - rField.GetValue(x, y, z).y) / dx;
	const float fDerZ = z + 1 < rField.GetSizeZ() && z > 0 ? (rField.GetValue(x, y, z + 1).z - rField.GetValue(x, y, z - 1).z) / dx2 : z > 0 ? (rField.GetValue(x, y, z).z - rField.GetValue(x, y, z - 1).z) / dx :	(rField.GetValue(x, y, z + 1).z - rField.GetValue(x, y, z).z) / dx;

	return fDerX + fDerY + fDerZ;
}

//------------------------------------------------------------------------------//

#endif // __CSCLOUDRENDERER_PLUGIN_H__