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

#ifndef __CSCLOUDSTANDARDS_PLUGIN_H__
#define __CSCLOUDSTANDARDS_PLUGIN_H__

#include "imesh/clouds.h"
#include <csgeom/vector3.h>

//------------------------------------------------------------------------------//

class csStdTemperatureInputField : public scfImplementation1<csStdTemperatureInputField, iField2>
{
private:
  //Variables are not needed for this implementation. The only reason they exists, is
  //for satisfying the interface!
  UINT		m_iSizeX;
  UINT		m_iSizeY;

public:
  csStdTemperatureInputField(iBase* pParent) : scfImplementationType(this, pParent), m_iSizeX(0), m_iSizeY(0)
  {}
  ~csStdTemperatureInputField()
  {
  }

  //O(n^2)
  virtual inline void SetSize(const UINT iSizeX, const UINT iSizeY)
  {
    m_iSizeX = iSizeX;
    m_iSizeY = iSizeY;
  }
  virtual const UINT GetSizeX() const {return m_iSizeX;}
  virtual const UINT GetSizeY() const {return m_iSizeY;}

  //O(1)
  virtual inline const float operator () (const UINT x, const UINT y) const
  {
    return GetValue(x, y);
  }
  //O(1)
  virtual inline const float GetValue(const UINT x, const UINT y) const
  {
    //Some periodic value generation
    return 20.f * ::cosf(x * ::fmodf(::rand(), 3.14f) + y * ::fmodf(::rand(), 3.14f)) + 285.f;
  }
  //O(1)
  virtual const float GetValueClamp(const int x, const int y) const
  {
    return GetValue(x, y);
  }
};

//------------------------------------------------------------------------------//

class csStdWaterVaporInputField : public scfImplementation1<csStdWaterVaporInputField, iField2>
{
private:
  /**
  Variables are not needed for this implementation. The only reason they exists, is
  for satisfying the interface!
  */
  UINT		m_iSizeX;
  UINT		m_iSizeY;

public:
  csStdWaterVaporInputField(iBase* pParent) : scfImplementationType(this, pParent), m_iSizeX(0), m_iSizeY(0)
  {}
  ~csStdWaterVaporInputField()
  {
  }

  //O(n^2)
  virtual inline void SetSize(const UINT iSizeX, const UINT iSizeY)
  {
    m_iSizeX = iSizeX;
    m_iSizeY = iSizeY;
  }
  virtual const UINT GetSizeX() const {return m_iSizeX;}
  virtual const UINT GetSizeY() const {return m_iSizeY;}

  //O(1)
  virtual inline const float operator () (const UINT x, const UINT y) const
  {
    return GetValue(x, y);
  }
  //O(1)
  virtual inline const float GetValue(const UINT x, const UINT y) const
  {
    //Some periodic value generation
    return ::fabsf(::cosf(x * ::fmodf(::rand(), 3.14f) + y * ::fmodf(::rand(), 3.14f)));
  }
  //O(1)
  virtual const float GetValueClamp(const int x, const int y) const
  {
    return GetValue(x, y);
  }
};


#endif // __CSCLOUDSTANDARDS_PLUGIN_H__