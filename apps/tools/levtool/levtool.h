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

#ifndef __LEVTOOL_H__
#define __LEVTOOL_H__

#include <stdarg.h>
#include "igeom/polymesh.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/parray.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
struct iDocument;
struct iDocumentNode;
class csVector3;

/**
 * A texture plane.
 */
class ltPlane
{
public:
  csVector3 orig, first, second;
  float firstlen, secondlen;
  char* name;

public:
  ltPlane () : orig (0), first (0), second (0), firstlen (1), secondlen (1),
  	name (0) { }
  ~ltPlane () { delete[] name; }
};

/**
 * A vertex with connectivity information.
 */
class ltVertex : public csVector3
{
private:
  int* polygons;
  int num_polygons;
  int max_polygons;
  int obj_number;

public:
  ltVertex ();
  ltVertex (const ltVertex& vt);
  ~ltVertex ();

  void AddPolygon (int idx);
  int GetPolygonCount () const { return num_polygons; }
  int GetPolygon (int idx) const { return polygons[idx]; }
  int* GetPolygonIndices () const { return polygons; }

  void SetObjectNumber (int obj_number)
  {
    ltVertex::obj_number = obj_number;
  }
  int GetObjectNumber () const { return obj_number; }

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

  // When splitting an object this will indicate the object
  // this polygon belongs too.
  int obj_number;

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

  void SetObjectNumber (int obj_number)
  {
    ltPolygon::obj_number = obj_number;
  }
  int GetObjectNumber () const { return obj_number; }

  iDocumentNode* GetNode () const { return polynode; }
};

/**
 * iDocumentNode wrapper.
 */
class ltDocNodeWrap
{
public:
  csRef<iDocumentNode> node;
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

  int max_obj_number;

  /**
   * Used by SplitThingSeparateUnits(). Will fill the given object number
   * in all polygons connected to this one.
   */
  void PaintConnectedPolygons (ltPolygon* sweep, int obj_number);

public:
  ltThing (iDocumentNode* meshnode, iDocumentNode* partnode);
  ~ltThing ();

  void AddVertex (const csVector3& vt);
  int GetVertexCount () const { return num_vertices; }
  const ltVertex& GetVertex (int idx) const { return *vertices[idx]; }
  ltVertex& GetVertex (int idx) { return *vertices[idx]; }

  ltPolygon* AddPolygon (iDocumentNode* polynode);
  int GetPolygonCount () const { return num_polygons; }
  ltPolygon* GetPolygon (int idx) const { return polygons[idx]; }

  void SetName (const char* name);
  const char* GetName () const { return name; }

  int GetMaxObjectNumber () const { return max_obj_number; }

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

  /**
   * Warning! Don't call this function BEFORE CompressVertices(),
   * RemoveUnusedVertices(), or RemoveDuplicateVertices().
   * This function will update all vertices so that they contain
   * a table of all polygon indices that use these vertices.
   */
  void CreateVertexInfo ();

  /**
   * Duplicate all vertices so that all polygons use distinct
   * vertices (no longer connected to each other).
   */
  void DuplicateSharedVertices ();

  /// Create a bounding box for this thing.
  void CreateBoundingBox ();
  /// Get the bounding box.
  const csBox3& GetBoundingBox () const { return bbox; }

  /**
   * Warning! Call this function AFTER DuplicateSharedVertices().
   * This function will divide this object in 8 sub-objects by
   * taking the center point and then distributing all polygons
   * according to that center point.
   */
  void SplitThingInCenter ();

  /**
   * Warning! Call this function AFTER CreateVertexInfo().
   * This function will split this thing in separate units.
   * A unit is defined as a group of polygons that is not
   * connected to other groups of polygons in the thing.
   * This splitting will happen by setting obj_number variable
   * to the appropriate sub-part.
   */
  void SplitThingSeparateUnits ();

  /**
   * Use this instead of SplitThingSeparateUnits() if you don't want to
   * split.
   */
  void DoNotSplitThingSeparateUnits ();

  /**
   * Create a mapping to map the current vertex id's to the
   * vertex id's relevant for the given object number.
   * Delete this array after using. This table will be as big
   * as the total number of vertices in this thing. The mapping
   * table will return -1 for vertices that are not in the
   * sub-object with number 'obj_number'.
   */
  int* CreateUnitMapping (int obj_number);

  iDocumentNode* GetMeshNode () const { return meshnode; }
  iDocumentNode* GetPartNode () const { return partnode; }
};

class csString;

/**
 * Main class.
 */
class LevTool
{
public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> cmdline;
  csPDelArray<ltThing> things;
  csPDelArray<ltDocNodeWrap> thing_nodes;
  csPDelArray<ltPlane> planes;

  // A vector with all strings that represent 'thing' mesh objects.
  csPDelArray<csString> thing_plugins;
  // A vector with all strings that represent 'plane' addons.
  csPDelArray<csString> plane_plugins;

  void ReportError (const char* description, ...);
  void Report (int severity, const char* description, ...);

  /**
   * This is this appears to be a valid XML world file.
   */
  bool TestValidXML (iDocument* doc);

  /**
   * Analyze the plugins section of the loaded world to see what
   * plugin strings represent thing mesh objects. Also find out about
   * plane objects.
   */
  void AnalyzePluginSection (iDocument* doc, bool meshfacts = false);

  /**
   * Test if a mesh object is a thing.
   */
  bool IsMeshAThing (iDocumentNode* meshnode);

  /**
   * Test if an addon object is a plane.
   */
  bool IsAddonAPlane (iDocumentNode* addonnode);

  /**
   * Test if a mesh object is movable.
   */
  bool IsMeshMovable (iDocumentNode* meshnode);

  /**
   * Scan all sectors and find all thing objects and push them
   * on the things vector.
   */
  void FindAllThings (iDocument* doc,
  	bool meshfacts = false, bool movable = false);

  /**
   * Scan all sectors and find all plane addons and push them
   * on the planes vector.
   */
  void FindAllPlanes (iDocument* doc);

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
   * Write out a polygon block.
   * The mapping table is used to map vertices in the polygon to
   * new vertices.
   */
  void WriteOutPolygon (iDocumentNode* poly_node, ltPolygon* p, int* mapping);

  /**
   * Write out a params block. Only write out polygons belonging
   * to the given obj_number.
   */
  void WriteOutThing (iDocumentNode* params_node, ltThing* th,
  	int obj_number);

  /**
   * Split a thing and output on the given parent node (a sector node).
   */
  void SplitThing (iDocumentNode* meshnode, iDocumentNode* parentnode);

  /**
   * Clone a document but move all planes in the process.
   */
  void CloneAndMovePlanes (iDocumentNode* node, iDocumentNode* newnode);

  /**
   * Clone a document but move all planes in the process.
   */
  void CloneAndMovePlanes (iDocument* doc, iDocument* newdoc);

  /**
   * Clone a document but split all things in the process.
   */
  void CloneAndSplitDynavis (iDocumentNode* node, iDocumentNode* newnode,
  	bool is_dynavis);

  /**
   * Clone a document but split all things in the process.
   */
  void CloneAndSplitDynavis (iDocument* doc, iDocument* newdoc,
  	bool is_dynavis);

  /**
   * Clone a document but change flags of all things in the process.
   */
  void CloneAndChangeFlags (iDocumentNode* node, iDocumentNode* newnode,
  	int op, int minsize, int maxsize, int minpoly, int maxpoly,
	float global_area);

  /**
   * Clone a document but change flags of all things in the process.
   */
  void CloneAndChangeFlags (iDocument* doc, iDocument* newdoc,
  	int op, int minsize, int maxsize, int minpoly, int maxpoly,
	float global_area);

  /**
   * Clone a node and children.
   */
  void CloneNode (iDocumentNode* from, iDocumentNode* to);

  //-----------------------------------------------------------------------

  /**
   * List the contents of a part.
   */
  void ListMeshPart (iDocumentNode* meshpart, int level);

  /**
   * List the contents of a meshobj.
   */
  void ListMeshObject (iDocumentNode* mesh, int level);

  /**
   * List the contents of a factory.
   */
  void ListFactory (iDocumentNode* factory, int level);

  /**
   * List the contents of a sector.
   */
  void ListSector (iDocumentNode* sector, int level);

  /**
   * List the contents of the world.
   */
  void ListContents (iDocumentNode* world);

  //-----------------------------------------------------------------------

  /**
   * Validate the contents of one thing.
   */
  void ValidateContents (ltThing* thing);

  /**
   * Validate the contents of all things.
   */
  void ValidateContents ();

  //-----------------------------------------------------------------------

public:
  LevTool ();
  ~LevTool ();

  void Main ();
};

#endif // __LEVTOOL_H__

