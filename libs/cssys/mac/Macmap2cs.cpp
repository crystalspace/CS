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
#include "mapstd.h"
#include "map.h"
#include "cworld.h"
#include "wad3file.h"
#include <Console.h>

//This will be redefined somewhere inside the CS libs. We don't want this
//to happen here.
#undef main

void PrintSyntax()
{
  printf("\nmap2cs version 0.4, Copyright (C) 1999 by Thomas Hieber\n\n");
  printf("map2cs comes with ABSOLUTELY NO WARRANTY; for details see the file 'COPYING'\n");
  printf("This is free software, and you are welcome to redistribute it under certain\n");
  printf("conditions; see `COPYING' for details.\n\n");

  printf("map2cs is a utility program to convert a MAP file to a Crystal Space level\n\n");

  printf("syntax: map2cs <mapfile> <worldfile> [configfile]\n");

  printf("for example: map2cs sample.map world quake.cfg\n");
  printf("             to create the CS level called world from sample.map\n");
  printf("             using config data in quake.cfg\n");
}

int main( int argc, char *argv[ ])
{
  //CWad3File wf;
  //wf.Extract("texpack.wad", "a_spiral");
  //return 0;

  argc = ccommand( &argv );

  if (argc < 3 || argc > 4)
  {
    PrintSyntax();
    return 1;
  }

  const char* configfile = "data/config/map2cs.cfg";
  if (argc==4)
  {
    configfile = argv[3];
  }
  const char* mapfile   = argv[1];
  const char* worldfile = argv[2];

  CMapFile Map;
  printf("Reading map '%s'\n", mapfile);
  Map.Read(mapfile, configfile);

  printf("Updating '%s' with texture info from '%s':\n", configfile, mapfile);
  Map.WriteTextureinfo();

  Map.CreatePolygons();
  printf("Writing world '%s'\n", worldfile);

  printf("Export using manual sectoring\n");
  CCSWorld World;
  World.Write(worldfile, &Map, mapfile);

  printf("done.");
  return 0;
}
