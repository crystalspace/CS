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
#include "map.h"
#include "entity.h"
#include "mpoly.h"
#include "csector.h"
#include "cthing.h"
#include "texplane.h"
#include "zipfile.h"
#include "mcurve.h"
#include "xworld.h"

#define TEMPWORLD "map2cs2.$$$"

CXmlWorld::CXmlWorld()
{
}

CXmlWorld::~CXmlWorld()
{
}

// added by Petr Kocmid, do not be harsh on Thomas when something goes wrong with it
bool CXmlWorld::Write(const char * filename, CMapFile * pMap, const char * sourcename) 
{
  if (!PrepareData(TEMPWORLD, pMap)) return false;

  // let's go out!
  fprintf(m_fd, "<worldmap file=\"%s\" source=\"%s\" generator=\"map2cs\" "
    "generator_version=\"0.8x\">\n", filename, sourcename); // just some reminders

  // top level entities collection
  Indent(); WriteIndent(); fprintf(m_fd, "<entities>\n"); Indent();
  unsigned int i, ilim = GetNumEntities();
  for (i=0; i < ilim; i++) {
    CMapEntity* pEntity = GetEntity(i);
    WriteIndent();  fprintf(m_fd, "<entity classname=\"%s\" name=\"%s\">\n",
      pEntity->GetClassname(), pEntity->GetName());

    // if the origin does exist, take it
    // having separate tag name for origin allows to very easy move stuff around with xml processing
    CdVector3 origin; if (pEntity->GetOrigin(origin)) {
      WriteIndent(); fprintf(m_fd, "<origin x=\"%g\" y=\"%g\" z=\"%g\"/>\n", origin.x, origin.y, origin.z);
    }

    // we are writing entity keys as a separate data tags, because it can contain upper case or characters, which
    // may not not pass certain xml validation 
    Indent();
    unsigned int j, jlim = pEntity->GetNumberOfKeyValuePairs();
    for (j=0; j < jlim; j++) {
      CMapKeyValuePair * pPair = pEntity->GetKeyValuePair(j);
      WriteIndent();  fprintf (m_fd, "<key name=\"%s\" value=\"%s\"/>\n",
        pPair->GetKey(), pPair->GetValue());
      // of course, we could ignore some info which is already out, such as 
      // origin key or classname key. to be done.
    }
    
    // now, let's take a look on geometry 
    if (pEntity->GetNumBrushes()) { // yes, there's something
      WriteIndent(); fprintf(m_fd, "<brushes>\n"); Indent();
      jlim = pEntity->GetNumBrushes();  
      for (j=0; j < jlim; j++) {        
        CMapBrush * pBrush = pEntity->GetBrush(j);        
        WriteIndent(); fprintf(m_fd,  "<brush visible=\"%s\">\n", pBrush->IsVisible() ? "true" : "false");
        // climb down to polygons
        Indent();
	unsigned int k, klim = pBrush->GetPolygonCount();
        for (k=0; k < klim; k++) {
          CMapPolygon * pPoly = pBrush->GetPolygon(k);
          CMapTexturedPlane* pTexPlane = pPoly->GetBaseplane();

          WriteIndent(); fprintf(m_fd, "<polygon ");
          if (pTexPlane->GetName()) { // I got a NULL pointer here, think should be an empty string instead
            fprintf(m_fd, "name=\"%s\"", pTexPlane->GetName());
          }
            
          fprintf(m_fd, "vertexcount=\"%u\" texturename=\"%s\">\n",
             pPoly->GetVertexCount(), pTexPlane->GetTextureName() );
          Indent(); 
          WriteIndent(); fprintf(m_fd, "<texturealign />");   // still incomplete

          // we are at polygon vertices
          unsigned int l, llim = pPoly->GetVertexCount();
	  for (l=0; l < llim; l++) {
            CdVector3 vertex = pPoly->GetVertex(l);
            WriteIndent(); fprintf(m_fd, "<vertex x=\"%g\" y=\"%g\" z=\"%g\"/>\n",
              vertex.x, vertex.y, vertex.z );

          }
          WriteIndent(); fprintf(m_fd, "</polygon>\n");
        }               
        Unindent(); WriteIndent(); fprintf(m_fd, "</brush>\n");
      }
      Unindent(); WriteIndent(); fprintf(m_fd, "</brushes>\n");
    }
    Unindent(); WriteIndent();  fprintf(m_fd, "</entity>\n");   
  }
  Unindent(); WriteIndent(); fprintf(m_fd, "</entities>\n");

  /*
  WriteIndent();  fprintf(m_fd, "<planes>\n");
  Indent();
  for (i=0; i<pMap->GetPlaneCount(); i++) {
    CMapTexturedPlane* pPlane = pMap->GetPlane(i);
    WriteIndent();
    fprintf(m_fd, "<plane>\n");
    
    WriteIndent();
    fprintf(m_fd, "</plane>\n");    
  }
  Unindent(); WriteIndent(); fprintf(m_fd, "</planes>\n");
  */

  Unindent(); WriteIndent();  fprintf(m_fd, "</worldmap>\n"); 
  fclose(m_fd);
  m_fd = NULL;

  remove (filename);
  rename (TEMPWORLD, filename);
  return true;
}

CIThing* CXmlWorld::CreateNewThing(CMapEntity* pEntity)
{
  return new CXmlThing(pEntity);
}

CISector* CXmlWorld::CreateNewSector(CMapBrush* pBrush)
{
  return new CXmlSector(pBrush);
}
