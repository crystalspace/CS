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
#include "csutil/csvector.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
class csParser;
class csVector3;

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

  csVector3* vertices;
  int num_vertices;
  int max_vertices;

  ltPolygon** polygons;
  int num_polygons;
  int max_polygons;

public:
  ltThing ();
  ~ltThing ();

  void AddVertex (const csVector3& vt);
  //@@@void CompressVertices ();
  int GetVertexCount () const { return num_vertices; }
  const csVector3& GetVertex (int idx) const { return vertices[idx]; }

  ltPolygon* AddPolygon ();
  int GetPolygonCount () const { return num_polygons; }
  ltPolygon* GetPolygon (int idx) const { return polygons[idx]; }

  void SetName (const char* name);
  const char* GetName () const { return name; }
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

  void ParseWorld (csParser* parser, char* buf);
  void ParseSector (csParser* parser, char* buf);
  void ParseMeshObj (csParser* parser, const char* thname, char* buf);
  void ParseThingParams (csParser* parser, const char* thname, char* buf);

public:
  LevTool ();
  ~LevTool ();

  void Main ();
};

#endif // __LEVTOOL_H

