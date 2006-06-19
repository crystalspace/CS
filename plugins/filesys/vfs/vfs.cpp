/*
    Crystal Space Virtual File System class
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>
	Copyright (C) 2006 by Brandon Hamilton <brandon.hamilton@gmail.com>

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
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vfs.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstringarray.h"
#include "iutil/objreg.h"

#define NEW_CONFIG_SCANNING

CS_IMPLEMENT_PLUGIN

namespace cspluginVFS
{

// --------------------------------------------------------------- csVFS --- //
SCF_IMPLEMENT_FACTORY (csVFS)

// csVFS contructor
csVFS::csVFS (iBase *iParent) :
  scfImplementationType(this, iParent),
 /* basedir(0),
  resdir(0),
  appdir(0),
  dirstack(8,8),*/
  object_reg(0)/*,
  auto_name_counter(0),
  verbosity(VERBOSITY_NONE)*/
{
  /*cwd = new char [2];
  cwd [0] = VFS_PATH_SEPARATOR;
  cwd [1] = 0;*/
  mutex = csMutex::Create (true); // We need a recursive mutex.
}

  //
csVFS::~csVFS ()
{
  /*delete [] cwd;
  delete [] basedir;
  delete [] resdir;
  delete [] appdir;*/
}

bool csVFS::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  /*
#ifdef NEW_CONFIG_SCANNING
  static const char* vfsSubdirs[] = {
    "etc/" CS_PACKAGE_NAME,
    "etc", 
    "",
    0};

  csPathsList configPaths;
  const char* crystalconfig = getenv("CRYSTAL_CONFIG");
  if (crystalconfig)
    configPaths.AddUniqueExpanded (crystalconfig);
  
  csPathsList* basedirs = 
    csInstallationPathsHelper::GetPlatformInstallationPaths();
  configPaths.AddUniqueExpanded (*basedirs * csPathsList  (vfsSubdirs));
  delete basedirs;

  configPaths.AddUniqueExpanded (".");
#ifdef CS_CONFIGDIR
  configPaths.AddUniqueExpanded (CS_CONFIGDIR);
#endif

  configPaths = csPathsUtilities::LocateFile (configPaths, "vfs.cfg", true);
  if (configPaths.Length() > 0)
  {
    basedir = alloc_normalized_path (configPaths[0].path);
  }
#else
  basedir = alloc_normalized_path(csGetConfigPath());
#endif

  csRef<iVerbosityManager> vm (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (vm.IsValid()) 
  {
    verbosity = VERBOSITY_NONE;
    if (vm->Enabled("vfs.debug", false)) verbosity |= VERBOSITY_DEBUG;
    if (vm->Enabled("vfs.scan",  true )) verbosity |= VERBOSITY_SCAN;
    if (vm->Enabled("vfs.mount", true )) verbosity |= VERBOSITY_MOUNT;
  }

  csRef<iCommandLineParser> cmdline =
    CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  if (cmdline)
  {
    resdir = alloc_normalized_path(cmdline->GetResourceDir());
    appdir = alloc_normalized_path(cmdline->GetAppDir());
  }
  
  // Order-sensitive: Mounts in first-loaded configuration file take precedence
  // over conflicting mounts in files loaded later.
  csStringSet seen;
  bool const verbose_scan = IsVerbose(VERBOSITY_SCAN);
  load_vfs_config(config, resdir,  seen, verbose_scan);
  load_vfs_config(config, appdir,  seen, verbose_scan);
#ifdef NEW_CONFIG_SCANNING
  bool result =	load_vfs_config(config, resdir,  seen, verbose_scan);
  if (result && (basedir == 0))
    basedir = alloc_normalized_path (resdir);
  result = load_vfs_config(config, appdir,  seen, verbose_scan);
  if (result && (basedir == 0))
    basedir = alloc_normalized_path (appdir);
  for (size_t i = 0; i < configPaths.Length(); i++)
  {
    load_vfs_config(config, configPaths[i].path,  seen, verbose_scan);
  }
#else
  load_vfs_config(config, basedir, seen, verbose_scan);
#endif

  return ReadConfig ();*/
  return true;
}

bool csVFS::ChDir (const char *Path)
{
	return true;
}

void csVFS::PushDir (char const* Path)
{
	
}

bool csVFS::PopDir ()
{
	return true;
}

csPtr<iDataBuffer> csVFS::ExpandPath (const char *Path, bool IsDir) const
{
	return NULL;
}

bool csVFS::Exists (const char *Path) const
{
		return true;
}

csPtr<iStringArray> csVFS::FindFiles (const char *Path) const
{
	return NULL;	
}

csPtr<iFile> csVFS::Open (const char *FileName, int Mode)
{
	return NULL;	
}
  
csPtr<iDataBuffer> csVFS::ReadFile (const char *FileName, bool nullterm)
{
	return NULL;	
}

bool csVFS::WriteFile (const char *FileName, const char *Data, size_t Size)
{
		return true;	
}

bool csVFS::DeleteFile (const char *FileName)
{
		return true;	
}

bool csVFS::Sync ()
{
		return true;	
}

bool csVFS::Mount (const char *VirtualPath, const char *RealPath)
{
		return true;	
}

bool csVFS::Unmount (const char *VirtualPath, const char *RealPath)
{
		return true;	
}
  
csRef<iStringArray> csVFS::MountRoot (const char *VirtualPath)
{
		return NULL;
}

bool csVFS::SaveMounts (const char *FileName)
{
			return true;
}

bool csVFS::LoadMountsFromFile (iConfigFile* file)
{
		return true;	
}

bool csVFS::ChDirAuto (const char* path, const csStringArray* paths,	const char* vfspath, const char* filename)
{
			return true;
}

bool csVFS::GetFileTime (const char *FileName, csFileTime &oTime) const
{
			return true;
}		

bool csVFS::SetFileTime (const char *FileName, const csFileTime &iTime)
{
			return true;
}

bool csVFS::GetFileSize (const char *FileName, size_t &oSize)
{
			return true;
}

csPtr<iDataBuffer> csVFS::GetRealPath (const char *FileName)
{
		return NULL;
}

csRef<iStringArray> csVFS::GetMounts ()
{
		return NULL;
}

csRef<iStringArray> csVFS::GetRealMountPaths (const char *VirtualPath)
{
		return NULL;
}

// Register a filesystem plugin
bool csVFS::RegisterPlugin(csRef<iFileSystem> FileSystem)
{
	// Add the plugin
	fsPlugins.PushSmart(FileSystem);
	return true;
}

} // namespace cspluginVFS
