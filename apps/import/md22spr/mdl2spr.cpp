/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 1998 by Nathaniel Saint Martin <noote@bigfoot.com>
  Significant overhaul by Eric Sunshine <sunshine@sunshineco.com> in Feb 2000

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
#include "csutil/sysfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m2s_mdl.h"
#include "m2s_md2.h"
#include "igraphic/imageio.h"

CS_IMPLEMENT_APPLICATION

static float scaleMdl = 0.025f;
static int delayMdl = 100;
static float positionMdlX = 0.0;
static float positionMdlY = 0.0;
static float positionMdlZ = 0.0;
static bool actionNamingMdl = true;
static bool resizeSkin = true;
static int maxFrames = -1;
csRef<iImageIO> mdl2spr_imageio;

static void usage(FILE* s, int rc)
{
  csFPrintf(s, "Usage: mdl2spr <option> [model-file] [sprite-name]\n\n");
  csFPrintf(s, "Options:\n");
  csFPrintf(s, "  -h : help (this page)\n");
  csFPrintf(s, "  -s <float> : global scale of model (default %g)\n", scaleMdl);
  csFPrintf(s, "  -d <int> : frame delay for frame which don't have delay "
    "(default %d)\n", delayMdl);
  csFPrintf(s, "  -n : %s auto naming of action frameset (%s by default)\n",
    actionNamingMdl ? "disable" : "enable",
    actionNamingMdl ? "enabled" : "disabled");
  csFPrintf(s, "  -x <float> : sprite moving on X axis (default %g)\n",
    positionMdlX);
  csFPrintf(s, "  -y <float> : sprite moving on Y axis (default %g)\n",
    positionMdlY);
  csFPrintf(s, "  -z <float> : sprite moving on Z axis (default %g)\n",
    positionMdlZ);
  csFPrintf(s, "  -r : %s automatic power-of-2 skin resizing (%s by default)\n",
    resizeSkin ? "disable" : "enabled", resizeSkin ? "enabled" : "disabled");
  csFPrintf(s, "       Only applies to MDL files (not MD2).\n");
  csFPrintf(s, "  -f : writes only n frames to SPR file (default all)\n");
  exit(rc);
}

static void fatal_usage() { usage(stderr, -1); }
static void okay_usage()  { usage(stdout,  0); }

static int get_int(int& n, int argc, const char* const* argv)
{
  int i = 0;
  n++;
  if (n >= argc)
  {
    csFPrintf(stderr, "Missing float value following %s\n", argv[n]);
    fatal_usage();
  }
  else
  {
    i = atoi(argv[n]);
    if (i == 0)
    {
      csFPrintf(stderr, "Unable to convert %s to int value\n", argv[n]);
      fatal_usage();
    }
  }
  return i;
}

static float get_float(int& n, int argc, const char* const* argv)
{
  float f = 0;
  n++;
  if (n >= argc)
  {
    csFPrintf(stderr, "Missing float value following %s\n", argv[n]);
    fatal_usage();
  }
  else
  {
    f = (float)atof(argv[n]);
    if (f == 0)
    {
      csFPrintf(stderr, "Unable to convert %s to float value\n", argv[n]);
      fatal_usage();
    }
  }
  return f;
}
int main(int argc,char *argv[])
{
  csPrintf("mdl2spr version 0.40\n"
    "A quake model (MDL/MD2) convertor for Crystal Space.\n"
    "By Nathaniel Saint Martin <noote@bigfoot.com>\n"
    "Project overhauled by Eric Sunshine <sunshine@sunshineco.com>\n\n");

  if (argc < 3)
    fatal_usage();
  else
  {
	int i;
    for (i = 1; i < argc - 2; i++)
    {
      if (argv[i][0] != '-' && argv[i][0] != '/')
      {
        csFPrintf(stderr, "'%s' unreconized option\n", argv[i]);
        fatal_usage();
      }
      switch (argv[i][1])
      {
      case 'h':
      case '?':
        okay_usage();

      case 'n':
        actionNamingMdl = !actionNamingMdl;
        csPrintf("%s auto naming of action frameset.\n",
	  actionNamingMdl ? "Enabled" : "Disabled");
        break;

      case 's':
        scaleMdl = get_float(i, argc, argv);
        csPrintf("General scale set to %g.\n", scaleMdl);
        break;

      case 'd':
        delayMdl = get_int(i, argc, argv);
        csPrintf("Frame delay set to %d.\n", delayMdl);
        break;

      case 'x':
        positionMdlX = get_float(i, argc, argv);
        csPrintf("X coordinate set to %g.\n", positionMdlX);
        break;

      case 'y':
        positionMdlY = get_float(i, argc, argv);
        csPrintf("Y coordinate set to %g.\n", positionMdlY);
        break;

      case 'z':
        positionMdlZ = get_float(i, argc, argv);
        csPrintf("Z coordinate set to %g.\n", positionMdlZ);
        break;

      case 'r':
        resizeSkin = !resizeSkin;
        csPrintf("%s automatic power-of-2 skin resizing.\n",
	  resizeSkin ? "Enabled" : "Disabled");
        break;

      case 'f':
        maxFrames = get_int(i, argc, argv);
        csPrintf("Max Frames set to %d.\n", maxFrames);
        break;

      default:
        csFPrintf(stderr, "'%s' unreconized option.\n", argv[i]);
        fatal_usage();
      }
    }
  }

  scfInitialize (argc, argv);

  const char* mdlfile = argv[argc - 2];
  QModel* mdl = 0;
  if (Mdl::IsFileMDLModel(mdlfile))
    mdl = new Mdl(mdlfile);
  else if (Md2::IsFileMD2Model(mdlfile))
    mdl = new Md2(mdlfile);
  else
  {
    csFPrintf(stderr, "Not a recognized model file: %s\n", mdlfile);
    exit(-1);
  }

  if (mdl->getError())
  {
    csFPrintf(stderr, "\nError: %s\n", mdl->getErrorString());
    delete mdl;
    exit(-1);
  }

  mdl->dumpstats(stdout);
  putchar('\n');
  mdl->WriteSPR(argv[argc - 1], scaleMdl, delayMdl, positionMdlX,
    positionMdlY, positionMdlZ, actionNamingMdl, resizeSkin, maxFrames);

  mdl2spr_imageio = 0;
  iSCF::SCF->Finish ();

  delete mdl;
  return 0;
}
