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
#include "csutil/csstring.h"

#include "mapstd.h"
#include "mcurve.h"
#include "mparser.h"
#include "map.h"
#include "mpoly.h"
#include "texplane.h"
#include "cworld.h"

#include "dochelp.h"

static int CurveCounter = 0;

CMapCurve::CMapCurve()
{
  m_Line = 0;
  m_Rows = 0;
  m_Cols = 0;
  m_pTexture = 0;
}

CMapCurve::~CMapCurve()
{
  DELETE_VECTOR_MEMBERS(m_Points);
}

bool CMapCurve::Read(CMapParser* pParser, CMapFile* pMap)
{
  csString Buffer;
  int    iDummy;

  m_Line = pParser->GetCurrentLine();

  if (!pParser->ExpectToken("patchDef2")) return false;
  if (!pParser->ExpectToken("{"))         return false;

  if (!pParser->GetSafeToken(Buffer)) return false;
  m_pTexture = pMap->GetTexture(Buffer);

  if (!m_pTexture)
  {
    pParser->ReportError("Texture \"%s\" not found!", 
      Buffer.GetData());
    return false;
  }

  if (!pParser->ExpectToken("("))     return false;

  if (!pParser->GetIntToken(m_Rows)) return false;
  if (!pParser->GetIntToken(m_Cols)) return false;
  if (!pParser->GetIntToken(iDummy)) return false;
  if (!pParser->GetIntToken(iDummy)) return false;
  if (!pParser->GetIntToken(iDummy)) return false;

  if (!pParser->ExpectToken(")"))     return false;

  if (!pParser->ExpectToken("("))     return false;

  int y, x;
  for (y=0; y<m_Rows; y++)
  {
    if (!pParser->ExpectToken("("))     return false;

    for (x=0; x<m_Cols; x++)
    {
      if (!pParser->ExpectToken("(")) return false;
      double x,y,z,u,v;
      if (!pParser->GetFloatToken(x)) return false;
      if (!pParser->GetFloatToken(y)) return false;
      if (!pParser->GetFloatToken(z)) return false;
      if (!pParser->GetFloatToken(u)) return false;
      if (!pParser->GetFloatToken(v)) return false;

      if (!pParser->ExpectToken(")")) return false;

      CMapCurvePoint* pPoint = new CMapCurvePoint(x,y,z,u,v);
      m_Points.Push(pPoint);
    }

    if (!pParser->ExpectToken(")"))     return false;
  }
  if (!pParser->ExpectToken(")"))     return false;
  if (!pParser->ExpectToken("}"))     return false;
  if (!pParser->ExpectToken("}"))     return false;

  csString Name;
  Name.Format ("%d", CurveCounter++);
  m_Name = Name;

  return true;
}

bool CMapCurve::Write(csRef<iDocumentNode> node, CCSWorld* pWorld)
{
  //Thomas Hieber, 16.5.2001: I know this method is wrong, but I don't know what format
  //                          CS expects curves after the change in the map format.
  //                          happy hacking stranger, whoever you are....

  assert(pWorld);

  CMapFile* pMap         = pWorld->GetMap();

  assert(pMap);

  CMapCurve*    pCurve   = this;
  CTextureFile* pTexture = pCurve->GetTexture();

  if (pTexture->IsVisible())
  {
    DocNode meshfact = CreateNode (node, "meshfact");
    meshfact->SetAttribute ("name", 
      csString().Format ("curve_%s", (const char*) m_Name));
    CreateNode (meshfact, "plugin", "bezierFact");
    DocNode params = CreateNode (meshfact, "params");
    int row, col;
    CdVector3 Center(0,0,0);
    double    NumVect = 0.0;

    DocNode centerN = CreateNode (params, "curvecenter");
    CreateNode (params, "curvescale", 80);
    for (row=0; row<pCurve->GetNumRows(); row++)
    {
      for (col=0; col<pCurve->GetNumCols(); col++)
      {
        CMapCurvePoint* pPoint = pCurve->GetPoint(row, col);
	DocNode cc = CreateNode (params, "curvecontrol");
	cc->SetAttributeAsFloat ("x", pPoint->m_Pos.x*pWorld->GetScalefactor());
	cc->SetAttributeAsFloat ("y", pPoint->m_Pos.z*pWorld->GetScalefactor());
	cc->SetAttributeAsFloat ("z", pPoint->m_Pos.y*pWorld->GetScalefactor());
	cc->SetAttributeAsFloat ("u", pPoint->m_u);
	cc->SetAttributeAsFloat ("v", pPoint->m_v);
        NumVect += 1.0;
        Center  += pPoint->m_Pos;
      }
    }

    if (NumVect>0)
    {
      Center *= 1.0/NumVect;
    }

    centerN->SetAttributeAsFloat ("x", Center.x*pWorld->GetScalefactor());
    centerN->SetAttributeAsFloat ("y", Center.z*pWorld->GetScalefactor());
    centerN->SetAttributeAsFloat ("z", Center.y*pWorld->GetScalefactor());

    int y, x;
    for (y=0; y<(pCurve->GetNumRows()-1)/2; y++)
    {
      for (x=0; x<(pCurve->GetNumCols()-1)/2; x++)
      {
	csString tmp;
	tmp.Format ("bez_%s_%d_%d", (const char*) m_Name, y, x);

        DocNode curve = CreateNode (params, "curve");
	curve->SetAttribute ("name", tmp);

	CreateNode (curve, "material", pTexture->GetTexturename());

        int numcols = pCurve->GetNumCols();
        int row1 = y*2+2;
        int row2 = y*2+1;
        int row3 = y*2+0;
        int col1 = x*2+0;
        int col2 = x*2+1;
        int col3 = x*2+2;

        CreateNode (curve, "v", col1 + row1*numcols);
        CreateNode (curve, "v", col2 + row1*numcols);
        CreateNode (curve, "v", col3 + row1*numcols);

        CreateNode (curve, "v", col1 + row2*numcols);
        CreateNode (curve, "v", col2 + row2*numcols);
        CreateNode (curve, "v", col3 + row2*numcols);

        CreateNode (curve, "v", col1 + row3*numcols);
        CreateNode (curve, "v", col2 + row3*numcols);
        CreateNode (curve, "v", col3 + row3*numcols);
      }
    }

  }

  return true;
}


