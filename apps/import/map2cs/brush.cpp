/*
    Map2cs: a convertor to convert the frequently used MAP format, into
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

#include "cssysdef.h"
#include "mapstd.h"
#include "brush.h"
#include "mparser.h"
#include "map.h"
#include "mpoly.h"
#include "texplane.h"
#include <ctype.h>

void CMapBrushBoundingBox::Extend(CMapPolygon* pPoly)
{
  assert(pPoly);
  size_t i;
  for (i=0; i<pPoly->GetVertexCount(); i++)
  {
    CdVector3 v = pPoly->GetVertex(i);
    if (!m_Defined)
    {
      m_Defined = true;

      m_x1 = m_x2 = v.x;
      m_y1 = m_y2 = v.y;
      m_z1 = m_z2 = v.z;
    }
    else
    {
      //Extend the bounding box, if needed.
      if (v.x < m_x1) m_x1 = v.x;
      if (v.y < m_y1) m_y1 = v.y;
      if (v.z < m_z1) m_z1 = v.z;

      if (v.x > m_x2) m_x2 = v.x;
      if (v.y > m_y2) m_y2 = v.y;
      if (v.z > m_z2) m_z2 = v.z;
    }
  }
}

bool CMapBrushBoundingBox::Intersects(CMapBrushBoundingBox* pOtherBox)
{
  assert(pOtherBox);

  if (!m_Defined || !pOtherBox->m_Defined) return false;

  if (m_x1 > pOtherBox->m_x2) return false;
  if (m_x2 < pOtherBox->m_x1) return false;
  if (m_y1 > pOtherBox->m_y2) return false;
  if (m_y2 < pOtherBox->m_y1) return false;
  if (m_z1 > pOtherBox->m_z2) return false;
  if (m_z2 < pOtherBox->m_z1) return false;

  return true;
}


CMapBrush::CMapBrush(CMapEntity* pEntity)
{
  m_Line    = 0;
  m_pEntity = pEntity;
}

CMapBrush::~CMapBrush()
{
  size_t i;
  for (i=0; i<m_Polygons.Length(); i++)
  {
    delete m_Polygons[i];
  }
}

bool CMapBrush::Read(CMapParser* pParser, CMapFile* pMap)
{
  csString Buffer;
  csString TextureName;
  bool finished = false;
  m_Line = pParser->GetCurrentLine();

  if (!pParser->GetSafeToken(Buffer)) return false;

  while (!finished)
  {
    if (strcmp(Buffer, "}") == 0)
    {
      //OK, we are done with this entity and it looks like
      //everything is ok
      finished = true;
    }
    else
    {
      if (strcmp(Buffer, "(") != 0)
      {
        pParser->ReportError("Format error. Expected either \"(\" or \"}\""
                             ", Found\"%s\"", Buffer.GetData());
        return false;
      }

      //if this brush is not finished, then another plane must follow.
      CdVector3 v1, v2, v3;
      double x_off     = 0;
      double y_off     = 0;
      double rot_angle = 0;
      double x_scale = 1.0;
      double y_scale = 1.0;
      CdVector3 v_tx_right, v_tx_up;

      //Read the three vectors, that define the position of the plane
      if (!ReadVector(pParser, v1))       return false;
      if (!pParser->ExpectToken(")"))     return false;

      if (!pParser->ExpectToken("("))     return false;
      if (!ReadVector(pParser, v2))       return false;
      if (!pParser->ExpectToken(")"))     return false;

      if (!pParser->ExpectToken("("))     return false;
      if (!ReadVector(pParser, v3))       return false;
      if (!pParser->ExpectToken(")"))     return false;

      //Get the name of the Texture
      if (!pParser->GetTextToken(Buffer)) return false;
      TextureName.Replace (Buffer);

      //Get Texture coordinates:

      // Texture coordinated in MAP files are a bit of a mess.
      // It seems like id-software didn't do a good job, when
      // laying the basics for this, because now, quite a few
      // editors and manufacturors have done additions to this.
      // map2cs will _try_ to manage the different formats
      // automatically.

      bool WC3MAP           = false;
      bool QuarkModeTexture = false;
      bool QuarkMirrored    = true;

      // First we check for the WorldCraft 3.3 MAP format
      // (The new official map format by valve software, and so
      // the new format for Half-Life and all its derivates)
      // Code for handling this format has been provided by
      // Tim Pike <wildman@spiderweb.com.au>.

      // Get Texture coordinates.
      if (!pParser->PeekNextToken(Buffer)) return false;
      if ((strcmp(Buffer, "[") == 0))
      {
        // This is for MAP's exported from Worldcraft 3.3
        WC3MAP = true;

        // Warning!!!!
        // I have not yet seen an explanation of the new
        // format introduced by Worldcraft 3.3, so this is
        // mainly a hack, to get map2cs load these new maps
        // again. Probably, the result will just be wrong
        // with texture alignmets, that are not trivial.
        // Thomas Hieber 2000-06-12

        // Read the X Offset (3D Vector & offset)
        if (!pParser->ExpectToken ("["))     return false;
        if (!ReadVector(pParser, v_tx_right))   return false;
        if (!pParser->GetFloatToken (x_off)) return false;
        if (!pParser->ExpectToken ("]"))     return false;

        // Read the Y Offset (3D Vector & offset)
        if (!pParser->ExpectToken ("["))     return false;
        if (!ReadVector(pParser, v_tx_up))   return false;
        if (!pParser->GetFloatToken (y_off)) return false;
        if (!pParser->ExpectToken ("]"))     return false;
      }
      else
      {
        // Standard Quake MAP format
        if (!pParser->GetFloatToken(x_off))  return false;
        if (!pParser->GetFloatToken(y_off))  return false;
      }

      // I assume, these three values have the same meaning in
      // all MAP formats, but I could be wrong here.
      // At least the work with old Half-Life style maps.

      if (!pParser->GetFloatToken(rot_angle)) return false;
      if (!pParser->GetFloatToken(x_scale))   return false;
      if (!pParser->GetFloatToken(y_scale))   return false;

      if (!pParser->GetSafeToken(Buffer)) return false;

      // Check for additions to the map format, introduced by QuArK.
      // (A great freeware Quake editor)

      if (strcmp(Buffer, "//TX1") == 0 ||
          strcmp(Buffer, "//TX2") == 0 )
      {
        QuarkModeTexture = true;
        if (strcmp(Buffer, "//TX1") == 0)
        {
          QuarkMirrored = false;
        }
        else
        {
          QuarkMirrored = true;
        }
        if (!pParser->GetSafeToken(Buffer)) return false;
      }
      else
      {
        if (strcmp(Buffer, "(") != 0 && strcmp(Buffer, "}") != 0)
        {
          // Looks like a Quake3 Arena Map. I don't know the meaning of these
          // numbers, but there are always three of them
          for (int i=0; i<3; i++)
          {
	    bool malformed = false;
	    bool intseen = false;
	    char const* q = Buffer;
	    while (*q != '\0' && isspace(*q))
	      q++;
	    while (*q != '\0' && isdigit(*q))
	    {
	      q++;
	      intseen = true;
	    }
	    malformed = (*q != '\0' && !isspace(*q));

	    if (malformed || !intseen)
	    {
	      pParser->ReportError(
		"Invalid Numeric format. Expected int, found \"%s\"", 
		Buffer.GetData());
		return false;
	    }

            if (!pParser->GetSafeToken(Buffer))
	      return false;
          }
        }
      }

      // todo: We would need to handle WC3MAP here to, to get proper
      //       texture alingment, but I don't know the new format...

      // In some rare cases, worldcraft will write illegal planes to the
      // map file. If we just accept these planes, we will produce follow
      // up errors. So we will not add that plane, and issue a warning
      // instead.
      if (((v1-v2)%(v1-v3)).Norm() == 0.0)
      {
        pParser->ReportError("The three given points don't form a plane. Plane ignored.");
      }
      else
      {
        // Get a pointer to the plane. This can be either a new plane, or
        // a pointer to a similar plane, that already existed
        CMapTexturedPlane* pPlane = pMap->AddPlane(v1, v2, v3, TextureName,
                                                   x_off, y_off, rot_angle,
                                                   x_scale, y_scale,
						   v_tx_right, v_tx_up,
                                                   QuarkModeTexture, QuarkMirrored,
						   WC3MAP);
        assert(pPlane);

        //Add that plane to the planes list.
        m_Planes.Push(pPlane);
      }
    }
  }
  return true;
}

bool CMapBrush::ReadVector(CMapParser* pParser, CdVector3& v)
{
  //read three integer values, that define the position of the vector.
  double value = 0;
  if (!pParser->GetFloatToken(value)) return false;
  v.x = value;
  if (!pParser->GetFloatToken(value)) return false;
  v.y = value;
  if (!pParser->GetFloatToken(value)) return false;
  v.z = value;
  return true;
}

void CMapBrush::CreatePolygons()
{
  CMapPolygon* pPoly = new CMapPolygon;
  size_t NumPlanes = m_Planes.Length();
  size_t i;
  for (i = 0; i<NumPlanes; i++)
  {
    pPoly->SetErrorInfo(m_Line, i);
    pPoly->Create(m_Planes[i], m_Planes, this);
    if (!pPoly->IsEmpty())
    {
      m_Polygons.Push(pPoly);
      m_BoundingBox.Extend(pPoly); //extend the bounding box with this poly.
      pPoly = new CMapPolygon;
    }
  }

  delete pPoly;
}

bool CMapBrush::IsInside(CdVector3& v)
{
  size_t k, NumPlanes = m_Planes.Length();
  for (k=0; k<NumPlanes; k++)
  {
    CMapTexturedPlane* pPlane = m_Planes[k];

    if (pPlane->Classify(v) < (-SMALL_EPSILON))
    {
      return false;
    }
  }  //for (k)

  return true;
}

/// Get the resulting polygon, if the plane cuts through the brush
void CMapBrush::IntersectWithPlane(CMapTexturedPlane* pIntersectplane,
                                   CMapPolygon& Poly)
{
  Poly.SetErrorInfo(m_Line, (size_t)-1);
  Poly.Create(pIntersectplane, m_Planes, this);
}

bool CMapBrush::IsVisible()
{
  size_t k, NumPlanes = m_Planes.Length();
  for (k=0; k<NumPlanes; k++)
  {
    CMapTexturedPlane* pPlane = m_Planes[k];
    if (pPlane->GetTexture()->IsVisible()) return true;
  }  //for (k)

  return false;
}
