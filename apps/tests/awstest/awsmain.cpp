#include "cssysdef.h"
#include "cstool/initapp.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "ivaria/reporter.h"
#include "iaws/aws.h"
#include "awstest.h"

#include <stdio.h>


CS_IMPLEMENT_APPLICATION

// The global system driver
awsTest *System;


int
main(int argc, char *argv[])
{
  csPrintf("Beginning test of AWS as a plugin...\n");

  // Create our main class.
  System = new awsTest();

  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
  if (!System->Initialize(argc, argv, "/config/awstest.cfg"))
  {
    System->Report(CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    exit (1);
  }

  // Main loop.
  iObjectRegistry* object_reg = System->object_reg;
  csDefaultRunLoop(System->object_reg);

  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);

  return 0;
}

