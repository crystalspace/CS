/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __LEVTOOL_H
#define __LEVTOOL_H

#include <stdarg.h>
#include "igeom/polymesh.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/csvector.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
class csParser;
class csVector3;

/**
 * A vertex with connectivity information.
 */
class ltVertex : public csVector3
{
private:
  int* polygons;
  int num_polygons;
  int max_polygons;

public:
  ltVertex ();
  ltVertex (const ltVertex& vt);
  ~ltVertex ();

  void AddPolygon (int idx);
  int GetPolygonCount () const { return num_polygons; }
  int GetPolygon (int idx) const { return polygons[idx]; }
  int* GetPolygonIndices () const { return polygons; }
};

/**
 * A polygon.
 */
class ltPolygon
{
private:
  char* name;

  int* vertices;
  int num_vertices;
  int max_vertices;

public:
  ltPolygon ();
  ~ltPolygon ();

  void AddVertex (int idx);
  int GetVertexCount () const { return num_vertices; }
  int GetVertex (int idx) const { return vertices[idx]; }
  int* GetVertexIndices () const { return vertices; }

  void RemoveDuplicateVertices ();

  void SetName (const char* name);
  const char* GetName () const { return name; }
};

/**
 * A thing.
 */
class ltThing
{
private:
  char* name;

  ltVertex** vertices;
  int num_vertices;
  int max_vertices;

  ltPolygon** polygons;
  int num_polygons;
  int max_polygons;

  csBox3 bbox;

public:
  ltThing ();
  ~ltThing ();

  void AddVertex (const csVector3& vt);
  int GetVertexCount () const { return num_vertices; }
  const csVector3& GetVertex (int idx) const { return *vertices[idx]; }

  ltPolygon* AddPolygon ();
  int GetPolygonCount () const { return num_polygons; }
  ltPolygon* GetPolygon (int idx) const { return polygons[idx]; }

  void SetName (const char* name);
  const char* GetName () const { return name; }

  /**
   * Warning! Only call this function BEFORE CreateVertexInfo()!
   * This function will remove all vertices that are duplicates (i.e.
   * close in space).
   */
  void CompressVertices ();
  /**
   * Warning! Only call this function BEFORE CreateVertexInfo()!
   * This function will remove all vertices that are not used by
   * polygons.
   */
  void RemoveUnusedVertices ();
  /**
   * Warning! Only call this function BEFORE CreateVertexInfo()!
   * This function will remove all duplicate vertices in polygons.
   */
  void RemoveDuplicateVertices ();

  void CreateBoundingBox ();

  /**
   * Warning! Don't call this function BEFORE CompressVertices(),
   * RemoveUnusedVertices(), or RemoveDuplicateVertices().
   * This function will update all vertices so that they contain
   * a table of all polygon indices that use these vertices.
   */
  void CreateVertexInfo ();
};

/**
 * Main class.
 */
class LevTool
{
public:
  iObjectRegistry* object_reg;
  iVFS* vfs;
  iCommandLineParser* cmdline;
  csVector things;

  void ReportError (const char* description, ...);

  void ParseWorld (csParser* parser, iFile* fout, char* buf);
  void ParseSector (csParser* parser, iFile* fout, char* buf);
  void ParseMeshObj (csParser* parser, iFile* fout,
  	const char* thname, char* buf);
  void ParseThingParams (csParser* parser, iFile* fout,
  	const char* thname, char* buf);
  void ParsePolygonParams (csParser* parser, iFile* fout,
  	ltPolygon* polygon, char* buf);

public:
  LevTool ();
  ~LevTool ();

  void Main ();
};

#endif // __LEVTOOL_H

