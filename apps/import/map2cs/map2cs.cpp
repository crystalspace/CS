/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "cssysdef.h"

#include <stdlib.h>

#include "mapstd.h"
#include "map.h"
#include "cworld.h"
#include "wad3file.h"
#include "iutil/plugin.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "csutil/xmltiny.h"
#include "cstool/initapp.h"
#include "igraphic/imageio.h"
#include "iutil/vfs.h"
#include "csver.h"

CS_IMPLEMENT_APPLICATION

//@@@ yup, global vars are ugly.
iImageIO* ImageLoader = 0;

void PrintSyntax()
{
  csPrintf("map2cs is a utility program to convert a MAP file to a Crystal Space level\n\n");

  csPrintf("syntax: map2cs <mapfile> <worldfile> [configfile]\n");

  csPrintf("for example: map2cs sample.map data/sample.zip sample.cfg\n");
  csPrintf("             to create the CS level called sample.zip from sample.map\n");
  csPrintf("             using config data in sample.cfg\n");
}

int appMain (iObjectRegistry* object_reg, int argc, char *argv[])
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  csRef<iImageIO> il = CS_LOAD_PLUGIN (plugin_mgr,
    "crystalspace.graphic.image.io.multiplex", iImageIO);
  if (!il)
  {
    csPrintf ("Couldn't load image multiplexer!\n");
    return 1;
  }
  else
  {
   ImageLoader = il;
  }
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    csPrintf ("Couldn't load VFS!\n");
    return 1;
  }

  csPrintf("\nmap2cs version " CS_VERSION "\n");
  csPrintf("Copyright (C) 1999-2003 by Thomas Hieber and others\n");
  csPrintf("CrystalSpace version " CS_VERSION "\n");
  csPrintf("Copyright (C) 1999-2003 by Jorrit Tyberghein and others\n\n");

  csPrintf("map2cs comes with ABSOLUTELY NO WARRANTY; for details see the file 'COPYING'\n");
  csPrintf("This is free software, and you are welcome to redistribute it under certain\n");
  csPrintf("conditions; see `COPYING' for details.\n\n");

  if (argc < 3 || argc > 4)
  {
    PrintSyntax();
    return 1;
  }
  
  const char* configfile;
  if (argc==4)
  {
    configfile = argv[3];
  }
  else
  {
    char* filename = new char[255];
    const char* crystal = getenv("CRYSTAL");
    if (!crystal)
    {
      csPrintf ("Couldn't find Crystal Space directory (CRYSTAL var)! Using current dir!\n");
      strcpy (filename, "data/config/map2cs.cfg");
    }
    else
    {
      strcpy (filename, crystal);
      strcat (filename, "/data/config/map2cs.cfg");
    }
    configfile=filename;
  }
  const char* mapfile   = argv[1];
  const char* worldfile = argv[2];

  CMapFile Map;
  csPrintf("Reading map '%s'...\n", mapfile);
  if (!Map.Read(mapfile, configfile))
    return 2;

  Map.CreatePolygons();
  csPrintf("Generating data for world '%s'...\n", worldfile);

  csRef<iDocumentSystem> xml(csPtr <iDocumentSystem>
    (new csTinyDocumentSystem()));
  csRef <iDocument> doc = xml->CreateDocument();
  csRef <iDocumentNode> root = doc->CreateRoot();

  CCSWorld World;
  World.Write(root, &Map, mapfile);

  remove (worldfile);
  VFS->Mount ("/map2cs_temp", worldfile);
  csPrintf ("Writing world...\n");
  doc->Write(VFS, "/map2cs_temp/world");

  csPrintf ("Writing textures...\n");
  if (!Map.GetTextureManager()->AddAllTexturesToVFS (VFS,
    "/map2cs_temp/"))
  {
    csPrintf ("Not all textures where written correctly.\n");
  }

  VFS->Unmount ("/map2cs_temp", worldfile);

  csPrintf("done.");

  return 0;
}

int main (int argc, char *argv[])
{
  // this is required for the image loader
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;
  if (!csInitializer::RequestPlugins (object_reg,
	CS_REQUEST_VFS,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END))
  {
    return 1;
  }

  int ret = appMain (object_reg, argc, argv);
  
  csInitializer::DestroyApplication (object_reg);

  return ret;
}
