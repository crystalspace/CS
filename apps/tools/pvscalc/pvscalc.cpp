/*
    Copyright (C) 2004-2005 by Jorrit Tyberghein

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

#include "pvscalc.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

PVSCalcSector::PVSCalcSector (PVSCalc* parent, iSector* sector, iPVSCuller* pvs)
{
  PVSCalcSector::parent = parent;
  PVSCalcSector::sector = sector;
  PVSCalcSector::pvs = pvs;
  pvstree = pvs->GetPVSTree ();
}

void PVSCalcSector::CountDistribution (
	const csArray<csBox3>& boxlist,
	float wherex, float wherey, float wherez,
	int& distx, int& disty, int& distz)
{
  size_t i;
  distx = disty = distz = 0;
  for (i = 0 ; i < boxlist.Length () ; i++)
  {
    int splitx = boxlist[i].TestSplit (0, wherex);
    int splity = boxlist[i].TestSplit (1, wherey);
    int splitz = boxlist[i].TestSplit (2, wherez);
    if (splitx < 0) distx--;
    else if (splitx > 0) distx++;
    if (splity < 0) disty--;
    else if (splity > 0) disty++;
    if (splitz < 0) distz--;
    else if (splitz > 0) distz++;
  }
}

void PVSCalcSector::DistributeBoxes (int axis, float where,
	const csArray<csBox3>& boxlist,
	csArray<csBox3>& boxlist_left,
	csArray<csBox3>& boxlist_right)
{
  size_t i;
  for (i = 0 ; i < boxlist.Length () ; i++)
  {
    int split = boxlist[i].TestSplit (axis, where);
    if (split <= 0)
      boxlist_left.Push (boxlist[i]);
    if (split >= 0)
      boxlist_right.Push (boxlist[i]);
  }
}

void PVSCalcSector::BuildKDTree (void* node, const csArray<csBox3>& boxlist,
	const csBox3& bbox, const csVector3& minsize)
{
  int axis;
  float where;
  csVector3 bbox_size = bbox.Max () - bbox.Min ();
  if (boxlist.Length () <= 1)
  {
    // If we have 1 or less objects left then we continue splitting the
    // node so that leafs are smaller then 'minsize'.
    if (bbox_size.x < minsize.x)
    {
      axis = 0;
      where = bbox.MinX () + bbox_size.x / 2;
    }
    else if (bbox_size.y < minsize.y)
    {
      axis = 1;
      where = bbox.MinY () + bbox_size.y / 2;
    }
    else if (bbox_size.z < minsize.z)
    {
      axis = 2;
      where = bbox.MinZ () + bbox_size.z / 2;
    }
    else
    {
      return;
    }
  }
  else
  {
    int distx, disty, distz;
    CountDistribution (boxlist,
    	bbox.MinX () + bbox_size.x / 2,
    	bbox.MinY () + bbox_size.y / 2,
    	bbox.MinZ () + bbox_size.z / 2,
	distx, disty, distz);
    if (distx < disty && distx < distz)
    {
      axis = 0;
      where = bbox.MinX () + bbox_size.x / 2;
    }
    else if (disty < distx && disty < distz)
    {
      axis = 1;
      where = bbox.MinY () + bbox_size.y / 2;
    }
    else
    {
      axis = 2;
      where = bbox.MinZ () + bbox_size.z / 2;
    }
  }

  void* child1;
  void* child2;
  csBox3 box1, box2;
  bbox.Split (axis, where, box1, box2);
  csArray<csBox3> boxlist_left;
  csArray<csBox3> boxlist_right;
  DistributeBoxes (axis, where, boxlist, boxlist_left, boxlist_right);
  pvstree->SplitNode (node, axis, where, child1, child2);
  BuildKDTree (child1, boxlist_left, box1, minsize);
  BuildKDTree (child2, boxlist_right, box2, minsize);
}

void PVSCalcSector::BuildKDTree ()
{
  iStaticPVSTree* pvstree = pvs->GetPVSTree ();
  pvstree->Clear ();
  void* root = pvstree->CreateRootNode ();
  const csBox3& bbox = pvstree->GetBoundingBox ();
  const csVector3& minsize = pvstree->GetMinimalNodeBox ();
  BuildKDTree (root, boxes, bbox, minsize);
}

void PVSCalcSector::CollectGeometry (iMeshWrapper* mesh,
  	csBox3& allbox, csBox3& staticbox,
	int& allcount, int& staticcount,
	int& allpcount, int& staticpcount)
{
  csBox3 mbox;
  mesh->GetWorldBoundingBox (mbox);
  iMeshObject* meshobj = mesh->GetMeshObject ();
  iObjectModel* objmodel = meshobj->GetObjectModel ();
  iPolygonMesh* polybase = objmodel->GetPolygonMeshBase ();

  // Increase stats for all objects.
  allcount++;
  allbox += mbox;
  if (polybase) allpcount += polybase->GetPolygonCount ();

  // Register the box.
  boxes.Push (mbox);

  iMeshFactoryWrapper* fact = mesh->GetFactory ();
  bool staticshape_fact = fact
    	? fact->GetMeshObjectFactory ()
		->GetFlags ().Check (CS_FACTORY_STATICSHAPE)
	: false;
  const csFlags& mesh_flags = meshobj->GetFlags ();
  bool staticshape_mesh = mesh_flags.Check (CS_MESH_STATICSHAPE);
  if (polybase && polybase->GetPolygonCount () &&
	mesh_flags.Check (CS_MESH_STATICPOS) &&
    	(staticshape_mesh || staticshape_fact))
  {
    // Increase stats for static objects.
    staticcount++;
    staticbox += mbox;
    staticpcount += polybase->GetPolygonCount ();
    csReversibleTransform trans = mesh->GetMovable ()->GetFullTransform ();
    csVector3* vertices = polybase->GetVertices ();
    csMeshedPolygon* polygons = polybase->GetPolygons ();
    int p, vt;
    for (p = 0 ; p < polybase->GetPolygonCount () ; p++)
    {
      const csMeshedPolygon& poly = polygons[p];
      csPoly3D poly3d;
      for (vt = 0 ; vt < poly.num_vertices ; vt++)
      {
        csVector3 vwor = trans.This2Other (vertices[poly.vertices[vt]]);
        poly3d.AddVertex (vwor);
      }
    }
  }

  int i;
  iMeshList* ml = mesh->GetChildren ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* child = ml->Get (i);
    CollectGeometry (child, allbox, staticbox, allcount, staticcount,
    	allpcount, staticpcount);
  }
}

void PVSCalcSector::Calculate ()
{
  parent->ReportInfo ("Calculating PVS for '%s'!",
  	sector->QueryObject ()->GetName ());

  const csVector3& minsize = pvstree->GetMinimalNodeBox ();
  parent->ReportInfo ("Minimal node size %g,%g,%g",
  	minsize.x, minsize.y, minsize.z);

  const csBox3& bbox = pvstree->GetBoundingBox ();
  parent->ReportInfo ("Total box (from culler) %g,%g,%g  %g,%g,%g",
  	bbox.MinX (), bbox.MinY (), bbox.MinZ (),
  	bbox.MaxX (), bbox.MaxY (), bbox.MaxZ ());

  int i;
  csBox3 allbox, staticbox;
  allbox.StartBoundingBox ();
  staticbox.StartBoundingBox ();
  int allcount = 0, staticcount = 0;
  int allpcount = 0, staticpcount = 0;
  iMeshList* ml = sector->GetMeshes ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* m = ml->Get (i);
    CollectGeometry (m, allbox, staticbox,
	allcount, staticcount,
	allpcount, staticpcount);
  }

  parent->ReportInfo ("Total box (all geometry) %g,%g,%g  %g,%g,%g",
  	allbox.MinX (), allbox.MinY (), allbox.MinZ (),
  	allbox.MaxX (), allbox.MaxY (), allbox.MaxZ ());
  parent->ReportInfo ("Total box (static geometry) %g,%g,%g  %g,%g,%g",
  	staticbox.MinX (), staticbox.MinY (), staticbox.MinZ (),
  	staticbox.MaxX (), staticbox.MaxY (), staticbox.MaxZ ());
  parent->ReportInfo( "%d static and %d total objects",
  	staticcount, allcount);
  parent->ReportInfo( "%d static and %d total polygons",
  	staticpcount, allpcount);
}

//-----------------------------------------------------------------------------

PVSCalc::PVSCalc ()
{
  SetApplicationName ("CrystalSpace.PVSCalcMap");
}

PVSCalc::~PVSCalc ()
{
}

bool PVSCalc::OnInitialize(int argc, char* argv[])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
      CS_REQUEST_VFS,
      CS_REQUEST_NULL3D,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_ENGINE,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue(GetObjectRegistry()))
    return ReportError("Failed to set up event handler!");

  return true;
}

void PVSCalc::OnExit()
{
}

bool PVSCalc::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  // The virtual clock.
  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY(GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = CS_QUERY_REGISTRY(GetObjectRegistry(), iLoader);
  if (!loader) return ReportError("Failed to locate Loader!");

  // Here we load our world from a map file.
  if (!LoadMap ()) return false;

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Calculate the PVS.
  CalculatePVS ();

  return true;
}

bool PVSCalc::SetMapDir (const char* map_dir)
{
  csStringArray paths;
  paths.Push ("/lev/");
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS->ChDirAuto (map_dir, &paths, 0, "world"))
  {
    ReportError ("Error setting VFS directory '%s'!", map_dir);
    return false;
  }
  return true;
}

bool PVSCalc::LoadMap ()
{
  // Set VFS current directory to the level we want to load.
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  const char* mapfile = cmdline->GetName (0);
  if (!mapfile)
  {
    ReportError ("Required parameters: <mapdir/zip> [ <sectorname> ]!");
    return false;
  }

  if (!SetMapDir (mapfile))
    return false;

  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    ReportError ("Error couldn't load world file at '%s'!", mapfile);
    return false;
  }

  // Get optional sector name. If no sector is given then all sectors
  // that have PVS will be calculated.
  sectorname = cmdline->GetName (1);

  return true;
}

void PVSCalc::CalculatePVS (iSector* sector, iPVSCuller* pvs)
{
  PVSCalcSector pvscalcsector (this, sector, pvs);
  pvscalcsector.Calculate ();
}

void PVSCalc::CalculatePVS ()
{
  int i;
  iSectorList* sl = engine->GetSectors ();
  bool found = false;
  for (i = 0 ; i < sl->GetCount () ; i++)
  {
    iSector* sector = sl->Get (i);
    const char* sname = sector->QueryObject ()->GetName ();
    if (sectorname.IsEmpty () || (sname != 0 &&
    	strcmp ((const char*)sectorname, sname) == 0))
    {
      iVisibilityCuller* viscul = sector->GetVisibilityCuller ();
      if (viscul)
      {
	csRef<iPVSCuller> pvs = SCF_QUERY_INTERFACE (viscul, iPVSCuller);
        if (pvs)
	{
	  found = true;
	  CalculatePVS (sector, pvs);
	}
      }
    }
  }
  if (!found)
  {
    if (sectorname.IsEmpty ())
      ReportError ("Found no sectors that have a PVS visibility culler!");
    else
      ReportError ("Sector '%s' has no PVS visibility culler!",
      	(const char*)sectorname);
  }
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return PVSCalc().Main(argc, argv);
}
