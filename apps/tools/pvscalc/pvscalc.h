/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __PVSCALC_H__
#define __PVSCALC_H__

#include <crystalspace.h>

class PVSCalc;

/**
 * The PVS calculator for one sector.
 */
class PVSCalcSector
{
private:
  PVSCalc* parent;
  iSector* sector;
  iPVSCuller* pvs;
  iStaticPVSTree* pvstree;
  int maxdepth;
  int countnodes;

  // All static polygons. Will be sorted on size.
  csArray<csPoly3D> polygons;
  // All world boxes for all objects. Will be used for calculating kdtree.
  csArray<csBox3> boxes;

  /**
   * Count distribution of boxes for the three axii.
   * distx, disty, and distz will contain the difference between
   * left and right distribution. So the smallest value is best.
   */
  void CountDistribution (
	const csArray<csBox3>& boxlist,
	float wherex, float wherey, float wherez,
	int& distx, int& disty, int& distz,
	bool& badx, bool &bady, bool& badz);
  /// Distribute a set of boxes to left/right.
  void DistributeBoxes (int axis, float where,
	const csArray<csBox3>& boxlist,
	csArray<csBox3>& boxlist_left,
	csArray<csBox3>& boxlist_right);
  /// Build the kdtree.
  void BuildKDTree ();
  void BuildKDTree (void* node, const csArray<csBox3>& boxlist,
	const csBox3& bbox, const csVector3& minsize,
	bool minsize_only, int depth);

public:
  PVSCalcSector (PVSCalc* parent, iSector* sector, iPVSCuller* pvs);

  /**
   * Collect all geometry from this mesh if static.
   * If not-static the mesh is still used for kdtree generation.
   */
  void CollectGeometry (iMeshWrapper* mesh,
  	csBox3& allbox, csBox3& staticbox,
	int& allcount, int& staticcount,
	int& allpcount, int& staticpcount);

  /**
   * Calculate the PVS for this sector.
   */
  void Calculate ();
};

/**
 * PVS calculator application. This is for the PVS visibility culler.
 */
class PVSCalc : public csApplicationFramework, public csBaseEventHandler
{
private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;

  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;

  /// A pointer to the keyboard driver.
  csRef<iKeyboardDriver> kbd;

  /// A pointer to the virtual clock.
  csRef<iVirtualClock> vc;

  /// The sector we are scanning. Or empty if we scan all.
  csString sectorname;

  /// Here we will load our world from a map file.
  bool LoadMap ();

  /// Set the current dir to the requested mapfile.
  bool SetMapDir (const char* map_dir);

  /// Calculate PVS for the given sector and culler.
  void CalculatePVS (iSector* sector, iPVSCuller* pvs);

  /// Calculate PVS for all sectors as given in 'sectorname'.
  void CalculatePVS ();

public:

  /// Construct our game. This will just set the application ID for now.
  PVSCalc ();

  /// Destructor.
  ~PVSCalc ();

  /// Final cleanup.
  void OnExit ();

  /**
   * Main initialization routine. This routine will set up some basic stuff
   * (like load all needed plugins, setup the event handler, ...).
   * In case of failure this routine will return false. You can assume
   * that the error message has been reported to the user.
   */
  bool OnInitialize (int argc, char* argv[]);

  /**
   * Run the application.
   * First, there are some more initialization (everything that is needed 
   * by PVSCalc1 to use Crystal Space), then this routine fires up the main
   * event loop. This is where everything starts. This loop will  basically
   * start firing events which actually causes Crystal Space to function.
   * Only when the program exits this function will return.
   */
  bool Application ();

};

#endif // __PVSCALC_H__
