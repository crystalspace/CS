/*
  Crystal Space Quake Milk Shape ASCII convertor
  Copyright (C) 2002 by Steven Geens <steven.geens@student.kuleuven.ac.be>
  Based upon:
  Crystal Space Quake MDL/MD2 convertor

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cssysdef.h"
#include "msmodel.h"


CS_IMPLEMENT_APPLICATION

static void usage(FILE* s, int rc)
{
  fprintf(s, "Usage: ms2spr <option> [model-file] [sprite-name]\n[sprite-name] without the trailing .spr\n");
  fprintf(s, "Options:\n");
  fprintf(s, "  -h : help (this page)\n");
  fprintf(s, "  -c : example code for loading\n");
  exit(rc);
}

static void fatal_usage() { usage(stderr, -1); }
static void okay_usage()  { usage(stdout,  0); }

static void printCode(FILE* s, int rc)
{
  fprintf(s, "An example for loading the file(using variables from simple2):\n\n");
  fprintf(s, "loader->LoadLibraryFile (\"/lib/std/sprfile.spr\")//assume you called the file, sprfile\n");
  fprintf(s, "...\n");
  fprintf(s, "engine->Prepare ();//All library loading should be before prepare\n");
  fprintf(s, "...\n");
  fprintf(s, "iMeshFactoryWrapper* imeshfact = engine->GetMeshFactories()->FindByName(\"sprfile\");//same name as sprite file\n");
  fprintf(s, "iMeshWrapper* sprite = engine->CreateMeshWrapper ( imeshfact, \"MySprite\", room, pos);\n");
  fprintf(s, "iSprite3DState* spstate = SCF_QUERY_INTERFACE (sprite->GetMeshObject (), iSprite3DState);\n");
  fprintf(s, "iSkeletonState *skel_state = spstate->GetSkeletonState();\n");
  fprintf(s, "limb->DecRef();\n");
  fprintf(s, "iSkeletonConnectionState *con = SCF_QUERY_INTERFACE (limb, iSkeletonConnectionState );\n");
  fprintf(s, "iSkeletonBone *bone = SCF_QUERY_INTERFACE (con, iSkeletonBone);\n");
  fprintf(s, "iMotionTemplate* motion=motman->FindMotionByName(\"default\");//The Motion is always called default\n");
  fprintf(s, "iMotionController* mc=motman->AddController(bone);\n");
  fprintf(s, "mc->SetMotion(motion);\n");
  fprintf(s, "spstate->DecRef ();\n");
  fprintf(s, "sprite->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);\n");
  fprintf(s, "sprite->DecRef ();\n");
  exit(rc);
}

int main(int argc,char *argv[])
{
  printf("ms2spr version 0.9\n"
    "A Milk Shape ASCII model convertor for Crystal Space.\n"
    "By Steven Geens <steven.geens@student.kuleuven.ac.be>\n\n");

  if (argc < 3)
  {
    switch (argv[1][1])
    {
      case 'c':
        printCode(stdout,  0);
      default:
       fatal_usage();
    }
  }
  else
  {
    int i;
    for (i = 1; i < argc - 2; i++)
    {
      if (argv[i][0] != '-' && argv[i][0] != '/')
      {
        fprintf(stderr, "'%s' unreconized option\n", argv[i]);
        fatal_usage();
      }
      switch (argv[i][1])
      {
      case 'h':
      case '?':
        okay_usage();
      case 'c':
        printCode(stdout,  0);
      default:
        fprintf(stderr, "'%s' unreconized option.\n", argv[i]);
        fatal_usage();
      }
    }
  }

  const char* msfile = argv[argc - 2];
  MsModel* ms = NULL;
  if (MsModel::IsFileMsModel(msfile))
    ms = new MsModel(msfile);
  else
  {
    fprintf(stderr, "Not a recognized model file: %s\n", msfile);
    exit(-1);
  }

  if (ms->getError())
  {
    fprintf(stderr, "\nError: %s\n", ms->getErrorString());
    delete ms;
    exit(-1);
  }

  ms->dumpstats(stdout);
  putchar('\n');
  ms->WriteSPR(argv[argc - 1]);
  
  delete ms;
  return 0;
}

