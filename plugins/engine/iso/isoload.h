/*
    Copyright (C) 2002 by Richard Uren <richard@starport.net>

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

#ifndef __ISOLOAD_H__
#define __ISOLOAD_H__

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csutil/util.h"
#include "csutil/plugldr.h"

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"

#include "ivaria/iso.h"
#include "ivaria/isoldr.h"
#include "ivideo/graph3d.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"

#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csgfx/rgbpixel.h"

struct iIsoEngine;
struct iGraphics3D;
struct iObjectRegistry;
struct iReporter;
struct iLoaderPlugin;
struct iMaterialWrapper;
struct iPluginManager;
struct iMeshWrapper;
struct iSyntaxService;
struct iLoaderContext;

/**
 * The loader for Crystal Space iso maps.
 */
class csIsoLoader : public iIsoLoader
{
private:
  iLoaderContext* ldr_context;
  iLoaderContext* GetLoaderContext ();

  // csLoadedPluginVector Cut & Paste from - csloader.h
  // Perhaps theres a better way to do this ??
  class csLoadedPluginVector : public csVector
  {
  private:
    // Find a loader plugin record
    struct csLoaderPluginRec* FindPluginRec (const char* name);
    // Return the loader plugin from a record, possibly loading the plugin now
    iLoaderPlugin* GetPluginFromRec (csLoaderPluginRec*);
  public:
    iPluginManager* plugin_mgr;

    // constructor
    csLoadedPluginVector (int iLimit = 8, int iThresh = 16);
    // destructor
    ~csLoadedPluginVector ();
    // delete a plugin record
    virtual bool FreeItem (csSome Item);
    // find a plugin by its name or load it if it doesn't exist
    iLoaderPlugin* FindPlugin (const char* Name);
    // add a new plugin record
    void NewPlugin (const char* ShortName, const char* ClassID);
  };

  /// List of loaded plugins
  csLoadedPluginVector loaded_plugins;

  //----- Helpers ------

  /// Report any error.
  void ReportError (const char* id, const char* description, ...);
  /// Report a notification.
  void ReportNotify (const char* description, ...);
  /// Find a material 
  virtual iMaterialWrapper* FindMaterial (const char *iName);
  /// Find a mesh factory
  virtual iMeshFactoryWrapper* FindMeshFactory (const char *iName);
  /// Returns true if params is not null, displays error
  /// via iReporter otherwise & returns false
  bool CheckParams(char* params, const char* tag, char* data);
  /// Displays error & returns false if an invalid token was found
  bool CheckToken(int cmd, const char* tag, char* data);
  /// Traditional TokenError routine
  void TokenError (const char *Object);

  //---- End Helpers ----

  // 
  bool LoadMap (char* buf);
  /// Loads plugins
  bool LoadPlugins (char* buf);
  /// Parses START definition - (sets iIsoView to POSITION)
  bool ParseStart(char* buf, const char* prefix);
  /// Parses GRID definition
  bool ParseGrid (char* buf, const char* prefix);
  /// Parses GRIDS definition (a list of GRID)
  bool ParseGridList (char* buf, const char* prefix);
  /// Parses MATERIALS definition (a list of MATERIAL)
  /// Note that the MATERIAL definition is much simpler
  /// in the iso engine because it has no textures.
  bool ParseMaterialList (char* buf, const char* prefix);
  /// Parse a LIGHT definition - a light belongs to a GRID
  bool ParseLight (char* buf, const char* prefix);
  /// Parse a TILE2D defintiion (for tiling an area on a GRID)
  bool ParseTile2D (char* buf, const char* prefix);
  /// Parse a PLUGINS defintion - This code copied from
  /// csparser/plgldr.cpp and class defintions changed to csIsoLoader
  bool ParsePluginList (char* buf, const char* prefix);
  /// Parse a basic MESHFACTORY definition.
  bool ParseMeshFactory (char* buf, const char* prefix);
  /// Parse a basic MESHOBJ definition (uses iLoaderPlugin to
  /// load plugins)
  bool ParseMeshObject (char* buf, const char* prefix);

  iIsoWorld *world;
  iIsoGrid *current_grid;
  iIsoView *view;

  // Starting position in the world
  csVector3 start_v;

public:
  
  SCF_DECLARE_IBASE;

  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iIsoEngine* Engine;
  iGraphics3D* G3D;
  iReporter* Reporter;
  iVFS* VFS;

  // iSyntaxService - Used to load vectors, matrices & misc other things
  iSyntaxService* Syntax;

  csIsoLoader(iBase *p);
  ~csIsoLoader();
  
  bool Initialize(iObjectRegistry *object_Reg);

  /// Loads an iso map file 
  bool LoadMapFile (const char* filename);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csIsoLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

#endif
