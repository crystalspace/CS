/*
    Copyright (C) 2010 by Jorrit Tyberghein, Eduardo Poyart

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

#ifndef __LOD_H__
#define __LOD_H__

struct Params
{
  csString input_file;
  csString output_file;
  ErrorMetricType error_metric_type;
  bool verbose;
  float min_dist;
  float max_dist;
  bool override_dist;
  bool dist_specified;
  
  Params():
    error_metric_type(ERROR_METRIC_FAST),
    verbose(false)
    {}
};

/**
 * This is the main class of this Tutorial. It contains the
 * basic initialization code and the main event handler.
 *
 * csApplicationFramework provides a handy object-oriented wrapper around the
 * Crystal Space initialization and start-up functions.
 *
 * csBaseEventHandler provides a base object which does absolutely nothing
 * with the events that are sent to it.
 */
class Lod : public csApplicationFramework, public csBaseEventHandler
{
private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;
  csRef<iThreadedLoader> tloader;
  csRef<iCollection> collection;
  csRef<iMeshFactoryWrapper> imeshfactw;

  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iRenderManager> rm;
  iSector* room;
  csRef<iThreadReturn> loading;
  csRef<iVFS> vfs;

  Params params;

  void Usage();
  bool ParseParams(int argc, char* argv[]);
  void CreateLODs(const char* filename_in, const char* filename_out);
  void CreateLODsRecursive(csRef<iDocumentNode> node);
  void CreateLODWithMeshFact(csRef<iDocumentNode> node);
  void Save(csRef<iDocument> doc, const char* filename);
  void SaveToNode(csRef<iDocumentNode> factNode);
  
  bool SetupModules ();

public:

  /// Construct our game. This will just set the application ID for now.
  Lod ();

  /// Destructor.
  ~Lod ();

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
   * by Simple1 to use Crystal Space), then this routine fires up the main
   * event loop. This is where everything starts. This loop will  basically
   * start firing events which actually causes Crystal Space to function.
   * Only when the program exits this function will return.
   */
  bool Application ();

  CS_EVENTHANDLER_NAMES("crystalspace.Lod")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __LOD_H__
