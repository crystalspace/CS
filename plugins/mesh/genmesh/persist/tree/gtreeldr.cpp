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

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "gtreeldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/genmesh.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (HEIGHT)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csGeneralTreeFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGeneralTreeFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGeneralTreeFactoryLoader)

SCF_EXPORT_CLASS_TABLE (gtreeldr)
  SCF_EXPORT_CLASS (csGeneralTreeFactoryLoader,
    "crystalspace.mesh.loader.factory.genmesh.tree",
    "Crystal Space General/Tree Mesh Factory Loader")
SCF_EXPORT_CLASS_TABLE_END

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

//----------------------------------------------------------------------------

// This is a set of concrete rule implementations.

class csStraightRule : public csConstructionRule
{
private:
  csConstructionObject* object;

public:
  csStraightRule (csConstructionObject* object)
  {
    csStraightRule::object = object;
  }

  virtual csConstructionObject* GetConstructionObject (int)
  {
    return object;
  }

  virtual int GetRotation (int)
  {
    return 1;
    //return (rand () >> 3) % 6;
  }
};

class csRandomRule : public csConstructionRule
{
private:
  csConstructionObject* object1;
  csConstructionObject* object2;
  csConstructionObject* object3;

public:
  csRandomRule (csConstructionObject* object1,
  	csConstructionObject* object2,
	csConstructionObject* object3)
  {
    csRandomRule::object1 = object1;
    csRandomRule::object2 = object2;
    csRandomRule::object3 = object3;
  }

  virtual csConstructionObject* GetConstructionObject (int depth)
  {
    if (depth < 6)
      return object1;
    else if (depth < 9)
    {
      float f = float ((rand () >> 3) & 0xff) / 255.;
      if (f <= .05) return object2;
      return object1;
    }
    else if (depth == 9)
    {
      float f = float ((rand () >> 3) & 0xff) / 255.;
      if (f <= .1) return object3;
      return object1;
    }
    else return object3;
  }

  virtual int GetRotation (int)
  {
    return 1;
    //return (rand () >> 3) % 6;
  }
};

class csDepthRule : public csConstructionRule
{
private:
  csConstructionObject* object1;
  csConstructionObject* object2;
  int depth;

public:
  csDepthRule (csConstructionObject* object1,
  	csConstructionObject* object2, int depth)
  {
    csDepthRule::object1 = object1;
    csDepthRule::object2 = object2;
    csDepthRule::depth = depth;
  }

  virtual csConstructionObject* GetConstructionObject (int d)
  {
    if (d >= depth) return object2;
    else return object1;
  }

  virtual int GetRotation (int)
  {
    return (rand () >> 3) % 6;
  }
};


//----------------------------------------------------------------------------

csGeneralTreeFactoryLoader::csGeneralTreeFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;

  co_tree = NULL;
  co_branch1 = NULL;
  co_branch2 = NULL;
  co_top = NULL;
  co_sidebranch = NULL;
  co_twig = NULL;
  co_twigside1 = NULL;
  co_twigside2 = NULL;

  cg_straighttrunk = NULL;
  cg_shrinktrunk = NULL;
  cg_tip = NULL;
  cg_debug4 = NULL;
  cg_branch = NULL;
  cg_smallbranch = NULL;
}

csGeneralTreeFactoryLoader::~csGeneralTreeFactoryLoader ()
{
  if (reporter) reporter->DecRef ();
  if (plugin_mgr) plugin_mgr->DecRef ();

  delete co_tree;
  delete co_branch1;
  delete co_branch2;
  delete co_top;
  delete co_sidebranch;
  delete co_twig;
  delete co_twigside1;
  delete co_twigside2;

  delete cg_straighttrunk;
  delete cg_shrinktrunk;
  delete cg_tip;
  delete cg_debug4;
  delete cg_branch;
  delete cg_smallbranch;
}

void csGeneralTreeFactoryLoader::GenerateTrunk (csConstructionGeometry* co,
	float brad, float trad, float height)
{
  int j;
  csVector3 vt[200];
  csTriangle tri[200];
  csReversibleTransform transform;
  csMatrix3 m;
  int vtidx[100];

  j = 0;
  vt[j++].Set (-.1*brad, 0, 0*brad);
  vt[j++].Set (-.03*brad, 0, .07*brad);
  vt[j++].Set (.03*brad, 0, .07*brad);
  vt[j++].Set (.1*brad, 0, 0*brad);
  vt[j++].Set (.03*brad, 0, -.07*brad);
  vt[j++].Set (-.03*brad, 0, -.07*brad);

  vt[j++].Set (-.1*trad, height, 0*trad);
  vt[j++].Set (-.03*trad, height, .07*trad);
  vt[j++].Set (.03*trad, height, .07*trad);
  vt[j++].Set (.1*trad, height, 0*trad);
  vt[j++].Set (.03*trad, height, -.07*trad);
  vt[j++].Set (-.03*trad, height, -.07*trad);
  co->SetVertices (j, 6, vt);

  j = 0;
  tri[j].a = 0;  tri[j].b = 1;  tri[j].c = 7;  j++;
  tri[j].a = 7;  tri[j].b = 6;  tri[j].c = 0;  j++;
  tri[j].a = 1;  tri[j].b = 2;  tri[j].c = 8;  j++;
  tri[j].a = 8;  tri[j].b = 7;  tri[j].c = 1;  j++;
  tri[j].a = 2;  tri[j].b = 3;  tri[j].c = 9;  j++;
  tri[j].a = 9;  tri[j].b = 8;  tri[j].c = 2;  j++;
  tri[j].a = 3;  tri[j].b = 4;  tri[j].c = 10; j++;
  tri[j].a = 10; tri[j].b = 9;  tri[j].c = 3;  j++;
  tri[j].a = 4;  tri[j].b = 5;  tri[j].c = 11; j++;
  tri[j].a = 11; tri[j].b = 10; tri[j].c = 4;  j++;
  tri[j].a = 5;  tri[j].b = 0;  tri[j].c = 6;  j++;
  tri[j].a = 6;  tri[j].b = 11; tri[j].c = 5;  j++;
  co->SetTriangles (j, tri);
  transform.SetO2TTranslation (csVector3 (0, -height, 0));
  m.Identity ();
  transform.SetO2T (m);
  j = 0;
  vtidx[j++] = 6;
  vtidx[j++] = 7;
  vtidx[j++] = 8;
  vtidx[j++] = 9;
  vtidx[j++] = 10;
  vtidx[j++] = 11;
  csOutputConnector* ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);
}

void csGeneralTreeFactoryLoader::GenerateBranch (csConstructionGeometry* co,
	float brad, float trad, float height)
{
  int j;
  csVector3 vt[200];
  csTriangle tri[200];
  csReversibleTransform transform;
  csMatrix3 m;
  csVector3 v;
  int vtidx[100];

  j = 0;
  vt[j++].Set (-.1*brad, 0, 0*brad);
  vt[j++].Set (-.03*brad, 0, .07*brad);
  vt[j++].Set (.03*brad, 0, .07*brad);
  vt[j++].Set (.1*brad, 0, 0*brad);
  vt[j++].Set (.03*brad, 0, -.07*brad);
  vt[j++].Set (-.03*brad, 0, -.07*brad);
  vt[j++].Set (-.1*trad, height, 0*trad);
  vt[j++].Set (-.03*trad, height, .07*trad);
  vt[j++].Set (.03*trad, height, .07*trad);
  vt[j++].Set (.1*trad, height, 0*trad);
  vt[j++].Set (.03*trad, height, -.07*trad);
  vt[j++].Set (-.03*trad, height, -.07*trad);

  // Vertices for the connection to the side branch.
  vt[j++].Set (-.1, height*.28, 0);
  vt[j++].Set (-.065, height*.42, .035);
  vt[j++].Set (-.065, height*.58, .035);
  vt[j++].Set (-.1, height*.72, 0);
  vt[j++].Set (-.065, height*.58, -.035);
  vt[j++].Set (-.065, height*.42, -.035);

  // Vertices for the top of the side branch.
  vt[j++].Set (-.21, height*.8, 0);
  vt[j++].Set (-.18, height*.83, .035);
  vt[j++].Set (-.15, height*.86, .035);
  vt[j++].Set (-.12, height*.89, 0);
  vt[j++].Set (-.15, height*.86, -.035);
  vt[j++].Set (-.18, height*.83, -.035);

  co->SetVertices (j, 6, vt);
  j = 0;
  tri[j].a = 1;  tri[j].b = 2;  tri[j].c = 8;  j++;
  tri[j].a = 8;  tri[j].b = 7;  tri[j].c = 1;  j++;
  tri[j].a = 2;  tri[j].b = 3;  tri[j].c = 9;  j++;
  tri[j].a = 9;  tri[j].b = 8;  tri[j].c = 2;  j++;
  tri[j].a = 3;  tri[j].b = 4;  tri[j].c = 10; j++;
  tri[j].a = 10; tri[j].b = 9;  tri[j].c = 3;  j++;
  tri[j].a = 4;  tri[j].b = 5;  tri[j].c = 11; j++;
  tri[j].a = 11; tri[j].b = 10; tri[j].c = 4;  j++;

  // Triangles for the connection of the main branch to side branch.
  tri[j].a = 14; tri[j].b = 7;  tri[j].c = 6;  j++;
  tri[j].a = 15; tri[j].b = 14; tri[j].c = 6;  j++;
  tri[j].a = 1;  tri[j].b = 7;  tri[j].c = 14; j++;
  tri[j].a = 13; tri[j].b = 1;  tri[j].c = 14; j++;
  tri[j].a = 12; tri[j].b = 1;  tri[j].c = 13; j++;
  tri[j].a = 0;  tri[j].b = 1;  tri[j].c = 12; j++;
  tri[j].a = 5;  tri[j].b = 0;  tri[j].c = 12; j++;
  tri[j].a = 5;  tri[j].b = 12; tri[j].c = 17; j++;
  tri[j].a = 5;  tri[j].b = 17; tri[j].c = 16; j++;
  tri[j].a = 5;  tri[j].b = 16; tri[j].c = 11; j++;
  tri[j].a = 16; tri[j].b = 15; tri[j].c = 6;  j++;
  tri[j].a = 16; tri[j].b = 6;  tri[j].c = 11; j++;

  // Side branch triangles.
  tri[j].a = 12; tri[j].b = 18; tri[j].c = 23; j++;
  tri[j].a = 12; tri[j].b = 23; tri[j].c = 17; j++;
  tri[j].a = 17; tri[j].b = 23; tri[j].c = 22; j++;
  tri[j].a = 17; tri[j].b = 22; tri[j].c = 16; j++;
  tri[j].a = 16; tri[j].b = 22; tri[j].c = 21; j++;
  tri[j].a = 16; tri[j].b = 21; tri[j].c = 15; j++;
  tri[j].a = 15; tri[j].b = 21; tri[j].c = 20; j++;
  tri[j].a = 15; tri[j].b = 20; tri[j].c = 14; j++;
  tri[j].a = 14; tri[j].b = 20; tri[j].c = 19; j++;
  tri[j].a = 14; tri[j].b = 19; tri[j].c = 13; j++;
  tri[j].a = 13; tri[j].b = 19; tri[j].c = 18; j++;
  tri[j].a = 13; tri[j].b = 18; tri[j].c = 12; j++;
  co->SetTriangles (j, tri);

  transform.SetO2TTranslation (csVector3 (0, -height, 0));
  m.Identity ();
  transform.SetO2T (m);
  j = 0;
  vtidx[j++] = 6;
  vtidx[j++] = 7;
  vtidx[j++] = 8;
  vtidx[j++] = 9;
  vtidx[j++] = 10;
  vtidx[j++] = 11;
  csOutputConnector* ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);

  m.Set (.4, 0, 0, 0, .4, 0, 0, 0, .4);
  m *= csZRotMatrix3 (.7);
  v = m.GetInverse () * csVector3 (.165, -.845*height, 0);
  transform.SetO2TTranslation (v);
  transform.SetO2T (m);

  j = 0;
  vtidx[j++] = 18;
  vtidx[j++] = 19;
  vtidx[j++] = 20;
  vtidx[j++] = 21;
  vtidx[j++] = 22;
  vtidx[j++] = 23;
  ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);
}

void csGeneralTreeFactoryLoader::GenerateDebug4 (csConstructionGeometry* co)
{
  int j;
  csVector3 vt[200];
  csTriangle tri[200];
  csReversibleTransform transform;
  csMatrix3 m;
  int vtidx[100];

  j = 0;
  vt[j++].Set (-.1, 0, .1);
  vt[j++].Set (.1, 0, .1);
  vt[j++].Set (.1, 0, -.1);
  vt[j++].Set (-.1, 0, -.1);
  vt[j++].Set (-.1, 1, .1);
  vt[j++].Set (.1, 1, .1);
  vt[j++].Set (.1, 1, -.1);
  vt[j++].Set (-.1, 1, -.1);
  co->SetVertices (j, 4, vt);
  j = 0;
  tri[j].a = 0; tri[j].b = 4; tri[j].c = 7; j++;
  tri[j].a = 0; tri[j].b = 7; tri[j].c = 3; j++;
  tri[j].a = 3; tri[j].b = 7; tri[j].c = 6; j++;
  tri[j].a = 3; tri[j].b = 6; tri[j].c = 2; j++;
  tri[j].a = 2; tri[j].b = 6; tri[j].c = 5; j++;
  tri[j].a = 2; tri[j].b = 5; tri[j].c = 1; j++;
  tri[j].a = 1; tri[j].b = 5; tri[j].c = 4; j++;
  tri[j].a = 1; tri[j].b = 4; tri[j].c = 0; j++;
  co->SetTriangles (j, tri);

  transform.SetO2TTranslation (csVector3 (0, -1, 0));
  m.Identity ();
  transform.SetO2T (m);
  j = 0;
  vtidx[j++] = 4;
  vtidx[j++] = 5;
  vtidx[j++] = 6;
  vtidx[j++] = 7;
  csOutputConnector* ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);
}

void csGeneralTreeFactoryLoader::GenerateSmallBranch (
	csConstructionGeometry* co,
	float brad, float trad, float height)
{
  int j;
  csVector3 vt[200];
  csTriangle tri[200];
  csReversibleTransform transform;
  csMatrix3 m;
  csVector3 v;
  int vtidx[100];

  j = 0;
  vt[j++].Set (-.1*brad, 0, 0*brad);
  vt[j++].Set (-.03*brad, 0, .07*brad);
  vt[j++].Set (.03*brad, 0, .07*brad);
  vt[j++].Set (.1*brad, 0, 0*brad);
  vt[j++].Set (.03*brad, 0, -.07*brad);
  vt[j++].Set (-.03*brad, 0, -.07*brad);
  vt[j++].Set (-.1*trad, height, 0*trad);
  vt[j++].Set (-.03*trad, height, .07*trad);
  vt[j++].Set (.03*trad, height, .07*trad);
  vt[j++].Set (.1*trad, height, 0*trad);
  vt[j++].Set (.03*trad, height, -.07*trad);
  vt[j++].Set (-.03*trad, height, -.07*trad);

  // Vertices for the connection to the side branch.
  vt[j++].Set (-.1, height*.4, 0);
  vt[j++].Set (-.085, height*.48, .015);
  vt[j++].Set (-.085, height*.52, .015);
  vt[j++].Set (-.1, height*.6, 0);
  vt[j++].Set (-.085, height*.52, -.015);
  vt[j++].Set (-.085, height*.48, -.015);

  // Vertices for the top of the side branch.
  vt[j++].Set (-.19, height*.52, 0);
  vt[j++].Set (-.175, height*.535, .015);
  vt[j++].Set (-.155, height*.555, .015);
  vt[j++].Set (-.14, height*.57, 0);
  vt[j++].Set (-.155, height*.555, -.015);
  vt[j++].Set (-.175, height*.535, -.015);

  co->SetVertices (j, 6, vt);
  j = 0;
  tri[j].a = 1;  tri[j].b = 2;  tri[j].c = 8;  j++;
  tri[j].a = 8;  tri[j].b = 7;  tri[j].c = 1;  j++;
  tri[j].a = 2;  tri[j].b = 3;  tri[j].c = 9;  j++;
  tri[j].a = 9;  tri[j].b = 8;  tri[j].c = 2;  j++;
  tri[j].a = 3;  tri[j].b = 4;  tri[j].c = 10; j++;
  tri[j].a = 10; tri[j].b = 9;  tri[j].c = 3;  j++;
  tri[j].a = 4;  tri[j].b = 5;  tri[j].c = 11; j++;
  tri[j].a = 11; tri[j].b = 10; tri[j].c = 4;  j++;

  // Triangles for the connection of the main branch to side branch.
  tri[j].a = 14; tri[j].b = 7;  tri[j].c = 6;  j++;
  tri[j].a = 15; tri[j].b = 14; tri[j].c = 6;  j++;
  tri[j].a = 1;  tri[j].b = 7;  tri[j].c = 14; j++;
  tri[j].a = 13; tri[j].b = 1;  tri[j].c = 14; j++;
  tri[j].a = 12; tri[j].b = 1;  tri[j].c = 13; j++;
  tri[j].a = 0;  tri[j].b = 1;  tri[j].c = 12; j++;
  tri[j].a = 5;  tri[j].b = 0;  tri[j].c = 12; j++;
  tri[j].a = 5;  tri[j].b = 12; tri[j].c = 17; j++;
  tri[j].a = 5;  tri[j].b = 17; tri[j].c = 16; j++;
  tri[j].a = 5;  tri[j].b = 16; tri[j].c = 11; j++;
  tri[j].a = 16; tri[j].b = 15; tri[j].c = 6;  j++;
  tri[j].a = 16; tri[j].b = 6;  tri[j].c = 11; j++;

  // Side branch triangles.
  tri[j].a = 12; tri[j].b = 18; tri[j].c = 23; j++;
  tri[j].a = 12; tri[j].b = 23; tri[j].c = 17; j++;
  tri[j].a = 17; tri[j].b = 23; tri[j].c = 22; j++;
  tri[j].a = 17; tri[j].b = 22; tri[j].c = 16; j++;
  tri[j].a = 16; tri[j].b = 22; tri[j].c = 21; j++;
  tri[j].a = 16; tri[j].b = 21; tri[j].c = 15; j++;
  tri[j].a = 15; tri[j].b = 21; tri[j].c = 20; j++;
  tri[j].a = 15; tri[j].b = 20; tri[j].c = 14; j++;
  tri[j].a = 14; tri[j].b = 20; tri[j].c = 19; j++;
  tri[j].a = 14; tri[j].b = 19; tri[j].c = 13; j++;
  tri[j].a = 13; tri[j].b = 19; tri[j].c = 18; j++;
  tri[j].a = 13; tri[j].b = 18; tri[j].c = 12; j++;
  co->SetTriangles (j, tri);

  transform.SetO2TTranslation (csVector3 (0, -height, 0));
  m.Identity ();
  transform.SetO2T (m);
  j = 0;
  vtidx[j++] = 6;
  vtidx[j++] = 7;
  vtidx[j++] = 8;
  vtidx[j++] = 9;
  vtidx[j++] = 10;
  vtidx[j++] = 11;
  csOutputConnector* ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);

  m.Set (.3, 0, 0, 0, .3, 0, 0, 0, .3);
  m *= csZRotMatrix3 (.7);
  v = m.GetInverse () * csVector3 (.165, -.545*height, 0);
  transform.SetO2TTranslation (v);
  transform.SetO2T (m);

  j = 0;
  vtidx[j++] = 18;
  vtidx[j++] = 19;
  vtidx[j++] = 20;
  vtidx[j++] = 21;
  vtidx[j++] = 22;
  vtidx[j++] = 23;
  ocon = new csOutputConnector (j, vtidx, transform);
  co->AddConnector (ocon);
}

bool csGeneralTreeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralTreeFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  csVector3 vt[100];
  csTriangle tri[100];
  int j;

  cg_straighttrunk = new csConstructionGeometry ();
  cg_shrinktrunk = new csConstructionGeometry ();
  cg_tip = new csConstructionGeometry ();
  cg_debug4 = new csConstructionGeometry ();
  cg_branch = new csConstructionGeometry ();
  cg_smallbranch = new csConstructionGeometry ();

  //---------
  // A straight trunk.
  //---------
  GenerateTrunk (cg_straighttrunk, 1, 1, .5);

  //---------
  // A trunk that shrinks at the top.
  //---------
  GenerateTrunk (cg_shrinktrunk, 1, .8, .5);

  //---------
  // A tip with no output connectors.
  //---------
  j = 0;
  vt[j++].Set (-.1, 0, 0);
  vt[j++].Set (-.03, 0, .07);
  vt[j++].Set (.03, 0, .07);
  vt[j++].Set (.1, 0, 0);
  vt[j++].Set (.03, 0, -.07);
  vt[j++].Set (-.03, 0, -.07);
  vt[j++].Set (0, .5, 0);
  cg_tip->SetVertices (j, 6, vt);
  j = 0;
  tri[j].a = 0; tri[j].b = 1; tri[j].c = 6; j++;
  tri[j].a = 1; tri[j].b = 2; tri[j].c = 6; j++;
  tri[j].a = 2; tri[j].b = 3; tri[j].c = 6; j++;
  tri[j].a = 3; tri[j].b = 4; tri[j].c = 6; j++;
  tri[j].a = 4; tri[j].b = 5; tri[j].c = 6; j++;
  tri[j].a = 5; tri[j].b = 0; tri[j].c = 6; j++;
  cg_tip->SetTriangles (j, tri);

  //---------
  // An object for testing. Has four connection points instead of six.
  //---------
  GenerateDebug4 (cg_debug4);

  //---------
  // A branch.
  //---------
  GenerateBranch (cg_branch, 1, 1, .15);
  GenerateSmallBranch (cg_smallbranch, 1, 1, .2);

  //---------
  // Rules.
  //---------

#if 0
  co_tree = new csConstructionObject (cg_debug4);
  co_tree->AddRule (new csDepthRule (co_tree, NULL, 3));
#elif 0
  co_tree = new csConstructionObject (cg_straighttrunk);
  co_branch1 = new csConstructionObject (cg_branch);
  co_sidebranch = new csConstructionObject (cg_straighttrunk);

  co_tree->AddRule (new csStraightRule (co_branch1));
  co_branch1->AddRule (new csStraightRule (co_sidebranch));
  co_branch1->AddRule (new csStraightRule (co_sidebranch));
  co_sidebranch->AddRule (new csDepthRule (co_sidebranch, NULL, 4));
#else 1
  co_tree = new csConstructionObject (cg_straighttrunk);
  co_branch1 = new csConstructionObject (cg_branch);
  co_branch2 = new csConstructionObject (cg_branch);
  co_top = new csConstructionObject (cg_tip);
  co_sidebranch = new csConstructionObject (cg_branch);
  co_twig = new csConstructionObject (cg_smallbranch);
  co_twigside1 = new csConstructionObject (cg_straighttrunk);
  co_twigside2 = new csConstructionObject (cg_straighttrunk);

  co_tree->AddRule (new csStraightRule (co_branch1));
  co_branch1->AddRule (new csStraightRule (co_branch2));
  co_branch1->AddRule (new csStraightRule (co_sidebranch));
  co_branch2->AddRule (new csStraightRule (co_top));
  co_branch2->AddRule (new csStraightRule (co_sidebranch));
  co_sidebranch->AddRule (new csDepthRule (co_sidebranch, co_top, 7));
  co_sidebranch->AddRule (new csStraightRule (co_twig));
  co_twig->AddRule (new csDepthRule (co_twig, co_top, 7));
  co_twig->AddRule (new csStraightRule (co_twigside1));
  co_twigside1->AddRule (new csStraightRule (co_twigside2));
  co_twigside2->AddRule (new csStraightRule (co_top));
#endif

  return true;
}

iBase* csGeneralTreeFactoryLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (HEIGHT)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.genmesh", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.genmesh",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.gentreefactoryloader.setup.objecttype",
		"Could not load the general mesh object plugin!");
    return NULL;
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();

  iGeneralFactoryState* state = SCF_QUERY_INTERFACE (fact,
  	iGeneralFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.gentreefactoryloader.parse.badformat",
		"Bad format while parsing general mesh/tree factory!");
      state->DecRef ();
      fact->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_HEIGHT:
	break;
    }
  }

  csConstruction* construction = new csConstruction ();
  csVector3 vertices[6];
#if 0
  int j = 0;
  vertices[j++].Set (-.1, 0, .1);
  vertices[j++].Set (.1, 0, .1);
  vertices[j++].Set (.1, 0, -.1);
  vertices[j++].Set (-.1, 0, -.1);
  construction->SetupInitialVertices (4, vertices);
  int vtidx[6];
  j = 0;
  vtidx[j++] = 0;
  vtidx[j++] = 1;
  vtidx[j++] = 2;
  vtidx[j++] = 3;
  construction->AddConstructionObject (0, csReversibleTransform (),
  	4, vtidx, 0, co_tree);
#else
  int j = 0;
  vertices[j++].Set (-.1, 0, 0);
  vertices[j++].Set (-.03, 0, .07);
  vertices[j++].Set (.03, 0, .07);
  vertices[j++].Set (.1, 0, 0);
  vertices[j++].Set (.03, 0, -.07);
  vertices[j++].Set (-.03, 0, -.07);
  construction->SetupInitialVertices (6, vertices);
  int vtidx[6];
  j = 0;
  vtidx[j++] = 0;
  vtidx[j++] = 1;
  vtidx[j++] = 2;
  vtidx[j++] = 3;
  vtidx[j++] = 4;
  vtidx[j++] = 5;
  construction->AddConstructionObject (0, csReversibleTransform (),
  	6, vtidx, 0, co_tree);
#endif
printf ("tri:%d vt:%d\n", construction->GetTriangleCount (),
		construction->GetVertexCount ());

  state->SetVertexCount (construction->GetVertexCount ());
  csVector3* vt = state->GetVertices ();
  memcpy (vt, construction->GetVertices (),
  	construction->GetVertexCount ()*sizeof (csVector3));
  csVector2* tx = state->GetTexels ();
  int i;
  for (i = 0 ; i < construction->GetVertexCount () ; i++)
  {
    const csVector3& v = vt[i];
    csVector2& uv = tx[i];
    // This is a bit like 3D texture mapping.
    uv.x = fmod (10.*fabs (v.x+v.z), 1.)/2.;
    uv.y = fmod (fabs (v.y), 1.);
    if (int (v.y) & 1) uv.y = 1-uv.y;
  }
  state->SetTriangleCount (construction->GetTriangleCount ());
  memcpy (state->GetTriangles (), construction->GetTriangles (),
  	construction->GetTriangleCount ()*sizeof (csTriangle));
  state->CalculateNormals ();
  delete construction;

  state->DecRef ();
  return fact;
}

//----------------------------------------------------------------------------

csConstructionGeometry::csConstructionGeometry ()
{
  num_input_points = 0;
  num_vertices = 0;
  vertices = NULL;
  num_triangles = 0;
  triangles = NULL;
  num_output_connectors = 0;
  output_connectors = NULL;
  int i;
  for (i = 0 ; i < num_output_connectors ; i++)
  {
    delete output_connectors[i];
  }
  delete[] output_connectors;
}

csConstructionGeometry::~csConstructionGeometry ()
{
  delete[] vertices;
  delete[] triangles;
}

void csConstructionGeometry::SetVertices (
  int inum_vertices, int inum_input_points, csVector3* ivertices)
{
  delete[] vertices;
  num_input_points = inum_input_points;
  num_vertices = inum_vertices;
  vertices = new csVector3 [inum_vertices];
  memcpy (vertices, ivertices, num_vertices * sizeof (csVector3));
}

void csConstructionGeometry::SetTriangles (
  int inum_triangles, csTriangle* itriangles)
{
  delete[] triangles;
  num_triangles = inum_triangles;
  triangles = new csTriangle [inum_triangles];
  memcpy (triangles, itriangles, num_triangles * sizeof (csTriangle));
}

void csConstructionGeometry::AddConnector (csOutputConnector* con)
{
  if (num_output_connectors == 0)
  {
    num_output_connectors = 1;
    output_connectors = new csOutputConnector* [1];
    output_connectors[0] = con;
  }
  else
  {
    csOutputConnector** new_output_connectors = new csOutputConnector* [
    	num_output_connectors+1];
    memcpy (new_output_connectors, output_connectors,
    	num_output_connectors * sizeof (csOutputConnector*));
    delete[] output_connectors;
    output_connectors = new_output_connectors;
    output_connectors[num_output_connectors] = con;
    num_output_connectors++;
  }
}

//----------------------------------------------------------------------------

csConstructionObject::csConstructionObject (csConstructionGeometry* igeometry)
{
  geometry = igeometry;
  num_rules = 0;
  rules = NULL;
}

csConstructionObject::~csConstructionObject ()
{
  int i;
  for (i = 0 ; i < num_rules ; i++)
  {
    csConstructionRule* rule = rules[i];
    delete rule;
  }
  delete[] rules;
}

void csConstructionObject::AddRule (csConstructionRule* rule)
{
  csConstructionRule** new_rules = new csConstructionRule*[num_rules+1];
  if (rules)
  {
    memcpy (new_rules, rules, sizeof (csConstructionRule*)*num_rules);
    delete[] rules;
  }
  rules = new_rules;
  rules[num_rules++] = rule;
}

//----------------------------------------------------------------------------

int csConstruction::AddVertex (const csVector3& v)
{
  if (num_vertices >= max_vertices)
  {
    csVector3* new_vertices = new csVector3 [max_vertices+40];
    if (num_vertices > 0)
    {
      memcpy (new_vertices, vertices, num_vertices * sizeof (csVector3));
      delete[] vertices;
    }
    vertices = new_vertices;
    max_vertices += 40;
  }
  vertices[num_vertices++].Set (v);
  return num_vertices-1;
}

csTriangle& csConstruction::AddTriangle ()
{
  if (num_triangles >= max_triangles)
  {
    csTriangle* new_triangles = new csTriangle [max_triangles+30];
    if (num_triangles > 0)
    {
      memcpy (new_triangles, triangles, num_triangles * sizeof (csTriangle));
      delete[] triangles;
    }
    triangles = new_triangles;
    max_triangles += 30;
  }
  num_triangles++;
  return triangles[num_triangles-1];
}

csConstruction::csConstruction ()
{
  num_vertices = max_vertices = 0;
  vertices = NULL;
  num_triangles = max_triangles = 0;
  triangles = NULL;
}

csConstruction::~csConstruction ()
{
  delete[] vertices;
  delete[] triangles;
}

void csConstruction::SetupInitialVertices (int num_vertices,
	csVector3* vertices)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    AddVertex (vertices[i]);
}

bool csConstruction::AddConstructionObject (int depth,
	const csReversibleTransform& g2o,
  	int num_connection_points, int* connection_points,
	int rotate_connection_points,
  	csConstructionObject* con)
{
  int i;
  csConstructionGeometry* geom = con->GetGeometry ();

  if (geom->GetInputPointCount () != num_connection_points)
    return false;

  int offset = num_vertices-num_connection_points;

  // Map point on construction object to output point here.
  int* mapping = new int[geom->GetVertexCount ()];
  for (i = 0 ; i < geom->GetVertexCount () ; i++)
  {
    if (i < num_connection_points)
      mapping[i] = connection_points[
      	(i+rotate_connection_points)%num_connection_points];
    else
      mapping[i] = i+offset;
  }

  // Set up a rotation matrix.
  csReversibleTransform rotation;
  const csReversibleTransform *new_g2o;
  if (rotate_connection_points == 0)
    new_g2o = &g2o;
  else
  {
    new_g2o = &rotation;
    csYRotMatrix3 rot_matrix (-PI*2*float (rotate_connection_points)
    		/ float (num_connection_points));
    rotation = g2o * csTransform (rot_matrix, csVector3 (0));
  }

  // Add all vertices except for the initial connection points.
  for (i = num_connection_points ; i < geom->GetVertexCount () ; i++)
  {
    AddVertex (new_g2o->Other2This (geom->GetVertices ()[i]));
  }

  // Add all triangles.
  for (i = 0 ; i < geom->GetTriangleCount () ; i++)
  {
    const csTriangle& ot = geom->GetTriangles ()[i];
    csTriangle& nt = AddTriangle ();
    nt.a = mapping[ot.a];
    nt.b = mapping[ot.b];
    nt.c = mapping[ot.c];
  }

  // Check all output connectors and apply the rules.
  for (i = 0 ; i < geom->GetOutputConnectorCount () ; i++)
  {
    csOutputConnector* ocon = geom->GetOutputConnector (i);
    csConstructionRule* rule = con->GetRule (i);
    csConstructionObject* new_con = rule->GetConstructionObject (depth);
    if (new_con)
    {
      int* new_points = new int[ocon->GetPointCount ()];
      int j;
      for (j = 0 ; j < ocon->GetPointCount () ; j++)
        new_points[j] = mapping[ocon->GetPoints ()[j]];

      if (!AddConstructionObject (depth+1, (*new_g2o)*ocon->GetTransform (),
  	  ocon->GetPointCount (), new_points,
	  rule->GetRotation (depth),
	  new_con))
      {
        delete[] new_points;
        delete[] mapping;
        return false;
      }
      delete[] new_points;
    }
  }

  delete[] mapping;
  return true;
}

//----------------------------------------------------------------------------

