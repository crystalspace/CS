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
struct iDocument;
struct iDocumentNode;
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
  csRef<iDocumentNode> polynode;
  char* name;

  int* vertices;
  int num_vertices;
  int max_vertices;

public:
  ltPolygon (iDocumentNode* polynode);
  ~ltPolygon ();

  void AddVertex (int idx);
  int GetVertexCount () const { return num_vertices; }
  int GetVertex (int idx) const { return vertices[idx]; }
  int* GetVertexIndices () const { return vertices; }

  void RemoveDuplicateVertices ();

  void SetName (const char* name);
  const char* GetName () const { return name; }

  iDocumentNode* GetNode () const { return polynode; }
};

/**
 * A thing.
 */
class ltThing
{
private:
  csRef<iDocumentNode> meshnode;	// The original 'meshobj' node.
  csRef<iDocumentNode> partnode;	// The 'part' or 'params' node.
  char* name;

  ltVertex** vertices;
  int num_vertices;
  int max_vertices;

  ltPolygon** polygons;
  int num_polygons;
  int max_polygons;

  csBox3 bbox;

public:
  ltThing (iDocumentNode* meshnode, iDocumentNode* partnode);
  ~ltThing ();

  void AddVertex (const csVector3& vt);
  int GetVertexCount () const { return num_vertices; }
  const ltVertex& GetVertex (int idx) const { return *vertices[idx]; }

  ltPolygon* AddPolygon (iDocumentNode* polynode);
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

  iDocumentNode* GetMeshNode () const { return meshnode; }
  iDocumentNode* GetPartNode () const { return partnode; }
};

/**
 * Main class.
 */
class LevTool
{
public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> cmdline;
  csVector things;
  csVector thing_nodes;

  // A vector with all strings that represent 'thing' mesh objects.
  csVector thing_plugins;

  void ReportError (const char* description, ...);

  /**
   * This is this appears to be a valid XML world file.
   */
  bool TestValidXML (iDocument* doc);

  /**
   * Analyze the plugins section of the loaded world to see what
   * plugin strings represent thing mesh objects.
   */
  void AnalyzePluginSection (iDocument* doc);

  /**
   * Test if a mesh object is a thing.
   */
  bool IsMeshAThing (iDocumentNode* meshnode);

  /**
   * Scan all sectors and find all thing objects and push them
   * on the things vector.
   */
  void FindAllThings (iDocument* doc);

  /**
   * Parse one part (or params) node for a thing.
   * vector (called by ParseThing()).
   */
  void ParsePart (ltThing* thing, iDocumentNode* partnode,
  	iDocumentNode* meshnode);

  /**
   * Parse one meshobj node and add a thing to the 'things'
   * vector (called by FindAllThings()).
   */
  void ParseThing (iDocumentNode* meshnode);

  /**
   * Clone a node and children.
   */
  void CloneNode (iDocumentNode* from, iDocumentNode* to);

  /**
   * Write the thing back into the XML structure. Perform the
   * required modifications.
   */
  void RewriteThing (ltThing* thing, iDocumentNode* newthing);

public:
  LevTool ();
  ~LevTool ();

  void Main ();
};

#endif // __LEVTOOL_H

