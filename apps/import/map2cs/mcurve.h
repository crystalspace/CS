/*
    Map2cs a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __MCURVE_H__
#define __MCURVE_H__

#include "contain.h"
#include "csutil/ref.h"

//Some forward declarations needed in class declaration.
class CMapParser;
class CMapTexturedPlane;
class CMapFile;
class CdVector3;
class CCSWorld;
struct iDocumentNode;

class CMapCurvePoint
{
public:
  CMapCurvePoint(double x, double y, double z, double u, double v)
   : m_Pos(x,y,z), m_u(u), m_v(v) {}
  CdVector3 m_Pos;
  double m_u;
  double m_v;
};

/**
  *
  */
class CMapCurve
{
public:
  /**
    * The constructor as usual
    */
  CMapCurve();

  /**
    * The destuctor as usual.
    */
  ~CMapCurve();

  /**
    * Reads an entire Curve, until the terminating "}" or until an error
    * occurs. returns false if an error occured. Parsing of the file should
    * then stop. The error message has then aready been generated.
    */
  bool Read      (CMapParser* pParser, CMapFile* pMap);

  /// Retrieve a special point
  CMapCurvePoint* GetPoint(int Row, int Col) {return m_Points[Row*m_Cols + Col];}

  /// Get the size of the Curve
  int GetNumRows() {return m_Rows;}
  int GetNumCols() {return m_Cols;}

  /// Set the name of the curve
  void SetName(const char* name) {m_Name = name;}

  /// Get the name of the curve
  const char* GetName()          {return m_Name;}

  CTextureFile* GetTexture() {return m_pTexture;}

  bool Write(csRef<iDocumentNode> node, CCSWorld* pWorld);

protected:

  /**
    * The number of the line, where the curve definition starts. (for error
    * messages)
    */
  int m_Line;

  int m_Rows;

  int m_Cols;

  CMapCurvePointVector m_Points;

  CTextureFile*        m_pTexture;

  csString             m_Name;
};

#endif // __MCURVE_H__

