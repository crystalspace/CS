/*
    Copyright (C) 1998 by Jorrit Tyberghein
    
    Key Value container written by Thomas Hieber
  
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

#ifndef KEYVAL_H
#define KEYVAL_H

#include "csgeom/math3d.h"
#include "csobject/csobject.h"
#include "csobject/nobjvec.h"

class csSector;

/**
 * A Key Value Pair.
 */
class csKeyValuePair : public csObject
{
public:
  csKeyValuePair(const char* Key, const char* Value);
  ~csKeyValuePair();

  const char* GetValue() const {return m_Value;}
private:
  char* m_Value;
  CSOBJTYPE;
};

class csMapNode : public csObject
{
public:
  csMapNode(const char* Name);
  ~csMapNode();

  void      SetPosition(csVector3 pos)   {m_Position = pos;}
  csVector3 GetPosition()                {return m_Position;}

  void      SetAngle(float Angle)        {m_Angle = Angle;}
  float     GetAngle()                   {return m_Angle;}
  
  void      SetSector(csSector* pSector) {m_pSector = pSector;}
  csSector* GetSector()                  {return m_pSector;}

private:
  csSector* m_pSector;
  csVector3 m_Position;
  float     m_Angle;
  CSOBJTYPE;
};

#endif /*KEYVAL_H*/
