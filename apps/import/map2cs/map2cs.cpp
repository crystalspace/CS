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
#include "xworld.h"
#include "wad3file.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"

CS_IMPLEMENT_PLATFORM_APPLICATION

void PrintSyntax()
{
  printf("map2cs is a utility program to convert a MAP file to a Crystal Space level\n\n");

  printf("syntax: map2cs <mapfile> <worldfile> [configfile]\n");

  printf("for example: map2cs sample.map data/sample.zip sample.cfg\n");
  printf("             to create the CS level called sample.zip from sample.map\n");
  printf("             using config data in sample.cfg\n");
}

int main( int argc, char *argv[ ])
{
  // this is required for the image loader
  scfInitialize(new csConfigFile("scf.cfg"));

  printf("\nmap2cs version 0.82A, Copyright (C) 2000 by Thomas Hieber\n\n");
  printf("map2cs comes with ABSOLUTELY NO WARRANTY; for details see the file 'COPYING'\n");
  printf("This is free software, and you are welcome to redistribute it under certain\n");
  printf("conditions; see `COPYING' for details.\n\n");

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
      printf ("Couldn't find Crystal Space directory! Set CRYSTAL var!\n");
      exit(1);
    }
    strcpy (filename, crystal);
    strcat (filename, "/data/config/map2cs.cfg");
    configfile=filename;
  }
  const char* mapfile   = argv[1];
  const char* worldfile = argv[2];

  CMapFile Map;
  printf("Reading map '%s'\n", mapfile);
  Map.Read(mapfile, configfile);

  Map.CreatePolygons();
  printf("Writing world '%s'\n", worldfile);

  // if an output file spec begins with xml=something, then output is done via xml
#ifndef NO_XML_SUPPORT
  if (!strncmp(worldfile, "xml=", 4)) {
    printf ("XML format wanted.\n");
    CXmlWorld World;
    World.Write(worldfile+4, &Map, mapfile);
  }
  else
#endif
  {
    printf ("Standard CS world format wanted.\n");
    CCSWorld World;
    World.Write(worldfile, &Map, mapfile);
  }

  printf("done.");

  iSCF::SCF->Finish();

  return 0;
}
