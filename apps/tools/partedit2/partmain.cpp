#include "cssysdef.h"

#include "cstool/initapp.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"
#include "ivaria/reporter.h"

#include "partedit2.h"

#include <stdio.h>


CS_IMPLEMENT_APPLICATION

// The global system driver
partEdit *System;


int
main(int argc, char *argv[])
{
  csPrintf("Starting particle system editor...\n");

  // Create our main class.
  System = new partEdit();

  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
  if (!System->Initialize(argc, argv, "/config/partedit2.cfg"))
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

