/*
  Crystal Space Maya .ma Convertor
  Copyright (C) 2002 by Keith Fulton <keith@paqrat.com>
    (loosely based on "mdl2spr" by Nathaniel Saint Martin <noote@bigfoot.com>
                     and Eric Sunshine <sunshine@sunshineco.com>)

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csutil/sysfunc.h"
#include "maya_mdl.h"
#include "igraphic/imageio.h"

CS_IMPLEMENT_APPLICATION


csArray<Animation*> anims;
csArray<Animation*> sockets;

static void usage(FILE* s, int rc)
{
  csFPrintf(s, "Usage: maya2spr <model-file> <sprite-file> [-a[ctions] action-name frame# duration [-dg vert# startframe# stopframe#... ] ...] [-s[ockets] socket-name poly# ...] \n");
  csFPrintf(s, "       <model-file>  is the name of the Maya file to import.\n");
  csFPrintf(s, "       <sprite-file> is the name of the CS Sprite file to export (XML format)\n");
  csFPrintf(s, "       -a[ctions]    means action/animation list follows.\n");
  csFPrintf(s, "       action-name   is the name of the animation CS will use.\n");
  csFPrintf(s, "       frame#        is the starting frame of the named animation.\n");
  csFPrintf(s, "       duration      is the overall length in msec of the named animation.\n");
  csFPrintf(s, "                     Anims are assumed to go from the starting frame\n");
  csFPrintf(s, "                     to the next animation start frame-1, or the last frame\n");
  csFPrintf(s, "                     in the file if no more actions are specified.\n");
  csFPrintf(s, "                     You can have as many action-names and #'s as you want.\n");
  csFPrintf(s, "       -dg           means displacement group list follows.\n");
  csFPrintf(s, "       vertex#       is the vertex to use for displacement measurements.\n");
  csFPrintf(s, "       startframe#   is the frame to start using this vertex.\n");
  csFPrintf(s, "       stopframe#    is the frame to stop using this vertex.\n");
  csFPrintf(s, "                     You can have as many displacement groups as you want.\n");
  csFPrintf(s, "                     Frames not within displacement groups will use msec delay time.\n\n");
  csFPrintf(s, "                     These displacements will only apply to the last action specified.\n");
  csFPrintf(s, "       -s[ockets]    means sockets list follows.\n");
  csFPrintf(s, "       socket-name   is the name of the socket.\n");
  csFPrintf(s, "       poly#         is the index of the polygon to use as the socket.\n");
  csFPrintf(s, "                     You can have as many name/poly pairs as you want.\n");
  csFPrintf(s, "Example: maya2spr kran.ma kran.spr -action stand 1 2000 -action walk 31 1000 -dg 147 1 10 671 20 25 -s lhand 532 rhand 145\n\n");
  csFPrintf(s, "This example processes the kran.ma file, translating it to kran.spr.  The stand action starts with frame 1\n");
  csFPrintf(s, "and takes 2 seconds before it should loop.  Then the walk action starts with frame 31 and takes 1 second to loop.\n");
  csFPrintf(s, "The walk action specifies that frames 1 to 10 should use the z displacements from vertex 147 and the z displacements\n");
  csFPrintf(s, "from vertex 671 for frames 20 to 25.  All other frames will use 1000 msec/30 frames as their time delays.  Then\n");
  csFPrintf(s, "it defines 2 sockets, specifying triangle 532 for the left hand and #145 for the right hand.\n\n");
  exit(rc);
}


static void fatal_usage() { usage(stderr, -1); }
//static void okay_usage()  { usage(stdout,  0); }

int proc_arg(int argc, char *argv[], int which)
{
    static Animation *last_anim;

    if (!strncmp(argv[which],"-a",2)) // build list of animations with frames
    {
    	int i;
        for (i = which+1; i < argc-1 && argv[i][0]!='-'; i+=3)
        {
            Animation *anim = new Animation;
            anim->name = argv[i];
            anim->startframe = atoi(argv[i+1]);
	    anim->duration   = atoi(argv[i+2]);

            anims.Push(anim);

	    csPrintf("Defined Animation: %s starting with frame %d.\n",
		   (const char *)anim->name, anim->startframe);

	    last_anim = anim;
        }
	return i;
    }
    else if (!strncmp(argv[which],"-s",2)) // build list of sockets and tri #'s.
    {
    	int i;
        for (i = which+1; i < argc-1 && argv[i][0]!='-'; i+=2)
        {
            Animation *socket = new Animation;
            socket->name = argv[i];
            socket->startframe = atoi(argv[i+1]);
            sockets.Push(socket);

	    csPrintf("Defined Socket: %s at polygon %d.\n",
		   (const char *)socket->name,socket->startframe);
        }
	return i;
    }
    else if (!strncmp(argv[which],"-dg",3)) // specify displacement frames
    {
    	int i;
        for (i = which+1; i < argc-1 && argv[i][0]!='-'; i+=3)
        {
            DisplacementGroup df;
            df.vertex = atoi(argv[i]);
            df.startframe = atoi(argv[i+1]);
            df.stopframe = atoi(argv[i+2]);

            last_anim->displacements.Push(df);

	    csPrintf("Defined displacement group: Frames %d thru %d, using vertex %d\n",
		   df.startframe, df.stopframe, df.vertex);
        }
	return i;
    }
    else
    {
       csPrintf("Illegal parameter %d (%s).  Please try again.\n",
	      which,argv[which]);
       return 0; // error
    }
}

int main(int argc,char *argv[])
{
  csPrintf("maya2spr version 0.1\n"
    "A Maya Ascii (.MA) convertor for Crystal Space.\n"
    "By Keith Fulton\n\n");

  if (argc < 6)
    fatal_usage();
  else
  {
     int parm = 3;
     while (parm < argc-1)
     {
	 parm = proc_arg(argc,argv,parm);
	 if (!parm)
	     fatal_usage();
     }
  }
  if (!anims.Length() ) // no anims means we need to specify a default one
  {
        csPrintf("Warning: Only default action being generated!\n");

        Animation *anim = new Animation;
        anim->name = "default";
        anim->startframe = 1;

	DisplacementGroup dg;
	dg.startframe=0;
	dg.stopframe =0;
	dg.vertex=0;
	anim->displacements.Push(dg); // dummy displacement group
        anims.Push(anim);
  }

  const char* mdlfile = argv[1];
  MayaModel* mdl = 0;
  if (Maya4Model::IsFileMayaModel(mdlfile))
    mdl = new Maya4Model(mdlfile);
  else
  {
    csFPrintf(stderr, "Not a recognized Maya 4.0 Ascii model file: %s\n", mdlfile);
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

  mdl->WriteSPR(argv[2],anims);

  delete mdl;

  csPrintf("maya2spr completed successfully.\n");

  return 0;
}
