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

#ifndef __CSCLOUDUTILS_PLUGIN_H__
#define __CSCLOUDUTILS_PLUGIN_H__

#include "imesh/clouds.h"
#include <csgeom/vector3.h>
#include <csutil/refcount.h>

//------------------------------------------------------------------------------//

template <typename T>
class csField3 : public csRefCount
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
  csField3<T>() : m_pppArray(NULL), m_iSizeX(0), m_iSizeY(0), m_iSizeZ(0)
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
  //O(1)
  virtual const T GetValueClamp(const int _x, const int _y, const int _z) const
  {
    const UINT x = _x < 0 ? 0 : _x >= static_cast<int>(m_iSizeX) ? static_cast<int>(m_iSizeX) - 1 : _x;
    const UINT y = _y < 0 ? 0 : _y >= static_cast<int>(m_iSizeY) ? static_cast<int>(m_iSizeY) - 1 : _y;
    const UINT z = _z < 0 ? 0 : _z >= static_cast<int>(m_iSizeZ) ? static_cast<int>(m_iSizeZ) - 1 : _z;
    return GetValue(x, y, z);
  }
};

//------------------------------------------------------------------------------//

/**
Both function expect vPos to be scaled on gridsize. Means that it doesn't contain
the REAL position, but coordinates on the voxelgrid
*/
const float GetInterpolatedValue(const csRef<csField3<float>>& rSrc, const csVector3& vPos);
const float GetInterpolatedValue(const csRef<csField3<csVector3>>& rSrc, const csVector3& vPos, const UINT iIndex);

//Implements the straightforward jacobi solver
void JacobiSolver(csRef<csField3<float>> rNew, const csRef<csField3<float>>& rOld, 
                  const csRef<csField3<float>>& rBField, const float fAlpha, const float fInvBeta);

//Interpolates the velocity
inline const csVector3 GetVelocityOfCellCenter(const csRef<csField3<csVector3>>& rField, 
                                               const UINT x, const UINT y, const UINT z)
{
  return 0.5f * csVector3(rField->GetValue(x, y, z).x + rField->GetValue(x + 1, y, z).x,
    rField->GetValue(x, y, z).y + rField->GetValue(x, y + 1, z).y,
    rField->GetValue(x, y, z).z + rField->GetValue(x, y, z + 1).z);
}
inline const csVector3 GetInterpolatedVelocity(const csRef<csField3<csVector3>>& rField, const csVector3& vPos)
{
  csVector3 vVel = csVector3();
  vVel.x = GetInterpolatedValue(rField, csVector3(vPos.x, vPos.y - 0.5f, vPos.z - 0.5f), 0);
  vVel.y = GetInterpolatedValue(rField, csVector3(vPos.x - 0.5f, vPos.y, vPos.z - 0.5f), 1);
  vVel.z = GetInterpolatedValue(rField, csVector3(vPos.x - 0.5f, vPos.y - 0.5f, vPos.z), 2);
  return vVel;
}

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

inline const csVector3 CalcGradient(const csRef<csField3<float>>& rField, const UINT x, const UINT y, const UINT z,
                                    const float dx)
{
  const float fInvdx2 = 1.f / (2.f * dx);
  //Calcs partial derivations in x, y and z direction.
  const float fDerX = (rField->GetValueClamp(x + 1, y, z) - rField->GetValueClamp(x - 1, y, z)) * fInvdx2;
  const float fDerY = (rField->GetValueClamp(x, y + 1, z) - rField->GetValueClamp(x, y - 1, z)) * fInvdx2;
  const float fDerZ = (rField->GetValueClamp(x, y, z + 1) - rField->GetValueClamp(x, y, z - 1)) * fInvdx2;
  return csVector3(fDerX, fDerY, fDerZ);
}

//------------------------------------------------------------------------------//

//Calculates the gradient of the norm of a Vectorfield
inline const csVector3 CalcGradient(const csRef<csField3<csVector3>>& rField, const UINT x, const UINT y, const UINT z,
                                    const float dx)
{
  const float fInvdx2 = 1.f / (2.f * dx);
  //Calcs partial derivations in x, y and z direction.
  const float fDerX = (rField->GetValueClamp(x + 1, y, z).Norm() - rField->GetValueClamp(x - 1, y, z).Norm()) * fInvdx2;
  const float fDerY = (rField->GetValueClamp(x, y + 1, z).Norm() - rField->GetValueClamp(x, y - 1, z).Norm()) * fInvdx2;
  const float fDerZ = (rField->GetValueClamp(x, y, z + 1).Norm() - rField->GetValueClamp(x, y, z - 1).Norm()) * fInvdx2;
  return csVector3(fDerX, fDerY, fDerZ);
}

//------------------------------------------------------------------------------//

inline const float CalcDivergence(const csRef<csField3<csVector3>>& rField, const UINT x, const UINT y, const UINT z,
                                  const float dx)
{
  const float fInvdx = 1.f / dx;
  //Calcs partial derivations in x, y and z direction.
  const float fDerX = (rField->GetValue(x + 1, y, z).x - rField->GetValue(x, y, z).x) * fInvdx;
  const float fDerY = (rField->GetValue(x, y + 1, z).y - rField->GetValue(x, y, z).y) * fInvdx;
  const float fDerZ = (rField->GetValue(x, y, z + 1).z - rField->GetValue(x, y, z).z) * fInvdx;
  return fDerX + fDerY + fDerZ;
}

//------------------------------------------------------------------------------//

//Calculates the rotation vector for the center of a cell!
inline const csVector3 CalcRotation(const csRef<csField3<csVector3>>& rField, const UINT x, const UINT y, const UINT z,
                                    const float dx)
{
  const float fInvdx = 1.f / dx;
  //Calcs partial derivations in x, y and z direction.
  const float dFzdy = (rField->GetValue(x, y + 1, z).z - rField->GetValue(x, y, z).z) * fInvdx;
  const float dFxdy = (rField->GetValue(x, y + 1, z).x - rField->GetValue(x, y, z).x) * fInvdx;
  const float dFxdz = (rField->GetValue(x, y, z + 1).x - rField->GetValue(x, y, z).x) * fInvdx;
  const float dFydz = (rField->GetValue(x, y, z + 1).y - rField->GetValue(x, y, z).y) * fInvdx;
  const float dFydx = (rField->GetValue(x + 1, y, z).y - rField->GetValue(x, y, z).y) * fInvdx;
  const float dFzdx = (rField->GetValue(x + 1, y, z).z - rField->GetValue(x, y, z).z) * fInvdx;
  return csVector3(dFzdy - dFydz, dFxdz - dFzdx, dFydx - dFxdy);
}

//------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//

/*template <typename T>
class csField2 : public scfImplementation1<csField2<T>, iField2<T>>
{
private:
T**				m_ppArray;
UINT			m_iSizeX;
UINT			m_iSizeY;

//O(n^2)
inline void DeleteField()
{
if(!m_ppArray) return;
for(UINT x = 0; x < m_iSizeX; ++x)
{
delete[] m_pppArray[x];
}
delete[] m_pppArray;
m_pppArray = NULL;
m_iSizeX = m_iSizeY = 0;
}

public:
csField2<T>(iBase* pParent) : m_ppArray(NULL), m_iSizeX(0), m_iSizeY(0), 
scfImplementationType(this, pParent)
{}
~csField2<T>()
{
DeleteField();
}

//O(n^2)
virtual inline void SetSize(const UINT iSizeX, const UINT iSizeY)
{
DeleteField();
m_iSizeX = iSizeX;
m_iSizeY = iSizeY;
//reserve memory
m_ppArray = new T*[m_iSizeX];
for(UINT x = 0; x < m_iSizeX; ++x)
{
m_pppArray[x] = new T[m_iSizeY];
}
}
virtual const UINT GetSizeX() const {return m_iSizeX;}
virtual const UINT GetSizeY() const {return m_iSizeY;}

//O(1)
virtual inline void SetValue(const T& Value, const UINT x, const UINT y)
{
if(m_ppArray) m_ppArray[x][y] = Value;
}

//O(1)
virtual inline const T operator () (const UINT x, const UINT y) const
{
return GetValue(x, y);
}
//O(1)
virtual inline const T GetValue(const UINT x, const UINT y) const
{
return m_pppArray[x][y];
}
//O(1)
virtual const T GetValueClamp(const int _x, const int _y) const
{
const UINT x = _x < 0 ? 0 : _x >= static_cast<int>(m_iSizeX) ? static_cast<int>(m_iSizeX) - 1 : _x;
const UINT y = _y < 0 ? 0 : _y >= static_cast<int>(m_iSizeY) ? static_cast<int>(m_iSizeY) - 1 : _y;
return GetValue(x, y);
}
};*/

#endif // __CSCLOUDUTILS_PLUGIN_H__