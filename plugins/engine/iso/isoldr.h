/*
    Copyright (C) 2002 by Richard Uren

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

#ifndef __ISOLDR_H__
#define __ISOLDR_H__

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

//#include "csgeom/vector3.h"
//#include "csgeom/matrix3.h"
//#include "csgfx/rgbpixel.h"

struct iIsoEngine;
struct iGraphics3D;
struct iObjectRegistry;
struct iReporter;
struct iLoaderPlugin;
struct iMaterialWrapper;
struct iPluginManager;
struct iMeshWrapper;
struct iSyntaxService;

/**
 * The loader for Crystal Space iso maps.
 */
class csIsoLoader : public iIsoLoader
{
private:

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

  /// Report any error.
  void ReportError (const char* id, const char* description, ...);
  /// Report a notification.
  void ReportNotify (const char* description, ...);
  /// Find a material 
  virtual iMaterialWrapper* FindMaterial (const char *iName);
  /// Find a mesh factory
  virtual iMeshFactoryWrapper* FindMeshFactory (const char *iName);

  // Helper functions
  bool CheckParams(char* params, const char* tag, char* data);
  bool CheckToken(int cmd, const char* tag, char* data);
  void TokenError (const char *Object);

  bool LoadMap (char* buf);
  bool LoadPlugins (char* buf);

  bool ParseStart(char* buf, const char* prefix);
  bool ParseGrid (char* buf, const char* prefix);
  bool ParseGridList (char* buf, const char* prefix);
  bool ParseMaterialList (char* buf, const char* prefix);
  bool ParseLight (char* buf, const char* prefix);
  bool ParseTile2D (char* buf, const char* prefix);
  bool ParsePluginList (char* buf, const char* prefix);
  bool ParseMeshFactory (char* buf, const char* prefix);
  bool ParseMeshObject (char* buf, const char* prefix);

  iIsoWorld *world;
  iIsoGrid *current_grid;
  iIsoView *view;

  // Starting position in the world
  csVector3 start_v;

public:
  /********** iLoader implementation **********/
  SCF_DECLARE_IBASE;

  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iIsoEngine* Engine;
  iGraphics3D* G3D;
  iReporter* Reporter;
  iVFS* VFS;

  iSyntaxService* Syntax;

  csIsoLoader(iBase *p);
  ~csIsoLoader();
  
  bool Initialize(iObjectRegistry* object_Reg);
  bool LoadMapFile (const char* filename);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csIsoLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

#endif
