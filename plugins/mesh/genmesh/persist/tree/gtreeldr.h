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

#ifndef _GTREELDR_H_
#define _GTREELDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/transfrm.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct csTriangle;
class csVector3;

class csConstructionGeometry;
class csConstructionObject;
class csConstructionRule;

/**
 * General/Tree Mesh factory loader.
 */
class csGeneralTreeFactoryLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iReporter* reporter;

  csConstructionGeometry* cg_straighttrunk;
  csConstructionGeometry* cg_shrinktrunk;
  csConstructionGeometry* cg_tip;
  csConstructionGeometry* cg_debug4;
  csConstructionGeometry* cg_branch;
  csConstructionGeometry* cg_smallbranch;

  csConstructionObject* co_tree;
  csConstructionObject* co_branch1;
  csConstructionObject* co_branch2;
  csConstructionObject* co_top;
  csConstructionObject* co_sidebranch;
  csConstructionObject* co_twig;
  csConstructionObject* co_twigside1;
  csConstructionObject* co_twigside2;

  void GenerateTrunk (csConstructionGeometry* co,
	float bot_radius, float top_radius, float height);
  void GenerateBranch (csConstructionGeometry* co,
	float bot_radius, float top_radius, float height);
  void GenerateDebug4 (csConstructionGeometry* co);
  void GenerateSmallBranch (csConstructionGeometry* co,
	float bot_radius, float top_radius, float height);

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csGeneralTreeFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csGeneralTreeFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, 
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGeneralTreeFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * A construction rule. A rule will return a new construction object
 * based on some private rule information. The current building depth
 * is a parameter for the rule.
 */
class csConstructionRule
{
public:
  /// Return a new construction object.
  virtual csConstructionObject* GetConstructionObject (int depth) = 0;
  /// Return the rotation for this new construction object on its parent.
  virtual int GetRotation (int depth) = 0;
};

/**
 * An output connector.
 */
class csOutputConnector
{
private:
  // Array of vertex indices for this output connector.
  int* vtidx;
  // Number of points for this connector.
  int num_points;

  // Transform the input ground space to new ground space.
  // Input ground space is defined such so that the center of the
  // object is at 0,0,0 and y->1 points upwards. This transform
  // will transform that space so that this condition is met again
  // at the output connector end.
  // Transforms are represented by T=M*(O-V). In this case T (this space)
  // represents the real object space of the final object. O (other space)
  // represents the local space in which the object is represented (i.e.
  // the input ground space).
  csReversibleTransform transform;

public:
  csOutputConnector (int num_points, int* vtidx,
  	const csReversibleTransform& transform)
  {
    csOutputConnector::num_points = num_points;
    csOutputConnector::vtidx = new int [num_points];
    memcpy (csOutputConnector::vtidx, vtidx, num_points*sizeof (int));
    csOutputConnector::transform = transform;
  }
  ~csOutputConnector ()
  {
    delete[] vtidx;
  }

  int GetPointCount () const { return num_points; }
  int* GetPoints () const { return vtidx; }
  const csReversibleTransform& GetTransform () const { return transform; }
};

/**
 * The geometry for a construction object.
 */
class csConstructionGeometry
{
private:
  int num_input_points;
  int num_vertices;
  csVector3* vertices;
  int num_triangles;
  csTriangle* triangles;
  int num_output_connectors;
  csOutputConnector** output_connectors;

public:
  csConstructionGeometry ();
  ~csConstructionGeometry ();

  /**
   * Set vertex array. These vertices are given relative to the input
   * ground plane.
   */
  void SetVertices (int num_vertices, int num_input_points,
  	csVector3* vertices);

  /**
   * Set the array of triangles.
   */
  void SetTriangles (int num_triangles, csTriangle* triangles);

  /**
   * Get the number of input points. The first vertices of this
   * object correspond with these input points.
   */
  int GetInputPointCount () const
  {
    return num_input_points;
  }

  /**
   * Get the number of vertices in this construction geometry (including
   * the input points at the beginning and output points somewhere in
   * the object).
   */
  int GetVertexCount () const
  {
    return num_vertices;
  }

  /**
   * Get the vertex array.
   */
  csVector3* GetVertices () const
  {
    return vertices;
  }

  /**
   * Get the number of triangles.
   */
  int GetTriangleCount () const
  {
    return num_triangles;
  }

  /**
   * Get the triangle array.
   */
  csTriangle* GetTriangles () const
  {
    return triangles;
  }

  /**
   * Add an output connector. This class will take ownership of the
   * connector.
   */
  void AddConnector (csOutputConnector* con);

  /**
   * Get the number of output connectors.
   */
  int GetOutputConnectorCount () const
  {
    return num_output_connectors;
  }

  /**
   * Get the description for one output connector.
   */
  csOutputConnector* GetOutputConnector (int i) const
  {
    return output_connectors[i];
  }
};

/**
 * A construction object. This object has one input connector with which
 * it can be connected to the output of a previous construction object, and
 * zero or more output connectors on which other objects can be placed.
 * The input connector will be fitted on a previous output connector
 * by placing it on the current ground-plane. An output connector will
 * result in a new ground-plane (by transformation).
 */
class csConstructionObject
{
private:
  csConstructionGeometry* geometry;
  int num_rules;
  csConstructionRule** rules;

public:
  csConstructionObject (csConstructionGeometry* geometry);
  ~csConstructionObject ();

  /**
   * Get the construction geometry.
   */
  csConstructionGeometry* GetGeometry () { return geometry; }

  /**
   * Add a rule. This class will take ownership of the rule and
   * delete it at destruction time.
   */
  void AddRule (csConstructionRule* rule);

  /**
   * Get the number of rules.
   */
  int GetRuleCount () const { return num_rules; }

  /**
   * Get a rule.
   */
  csConstructionRule* GetRule (int idx) const { return rules[idx]; }
};

/**
 * A construction.
 */
class csConstruction
{
private:
  int num_vertices, max_vertices;
  csVector3* vertices;
  int num_triangles, max_triangles;
  csTriangle* triangles;

  int AddVertex (const csVector3& v);
  csTriangle& AddTriangle ();

public:
  csConstruction ();
  ~csConstruction ();

  /**
   * Setup initial vertices for the construction. These
   * will serve as the first output connector on which to start
   * building.
   */
  void SetupInitialVertices (int num_vertices, csVector3* vertices);

  /**
   * Add a construction object given vertices that can serve as
   * an output connector. Returns false if the output connector is
   * not compatible with the construction object (wrong number of
   * connection points). This function will recurse so that all outputs
   * of this construction object will also receive new constructions objects.
   * The transforms transforms input ground space (other) to object space
   * (this).
   * 'rotate_connection_points' is between 0 and num_connection_points-1.
   * When different from 0 this will rotate this new construction
   * object on the connection point.
   */
  bool AddConstructionObject (int depth, const csReversibleTransform& g2o,
  	int num_connection_points, int* connection_points,
	int rotate_connection_points,
  	csConstructionObject* con);

  /**
   * Get the number of vertices.
   */
  int GetVertexCount () const { return num_vertices; }
  /**
   * Get the array of vertices.
   */
  csVector3* GetVertices () const { return vertices; }
  /**
   * Get the number of triangles.
   */
  int GetTriangleCount () const { return num_triangles; }
  /**
   * Get the array of triangles.
   */
  csTriangle* GetTriangles () const { return triangles; }
};

#endif // _GTREELDR_H_

