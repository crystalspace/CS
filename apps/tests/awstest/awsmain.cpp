#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "ivaria/reporter.h"
#include "ivaria/aws.h"
#include "awstest.h"

#include <stdio.h>


CS_IMPLEMENT_APPLICATION

// The global system driver
awsTest *System;


int
main(int argc, char *argv[])
{
  printf("Beginning test of AWS as a plugin...\n"); 

  // Create our main class.
  System = new awsTest();

  //System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  //System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  //System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  //System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  //System->RequestPlugin ("crystalspace.engine.3d:Engine");
  //System->RequestPlugin ("crystalspace.level.loader:LevelLoader");


  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
  if (!System->Initialize(argc, argv, "/config/awstest.cfg"))
  {
    System->Report(CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    exit (1);
  }
 
  // Main loop.
  System->Loop();

  delete System;
 return 0;
}
