// test program -- should go away as loader gets debugged.....

//
// WARNING
//
// To successfully run this program, find the lines:
//
//  if (!(G3D = QUERY_PLUGIN (sys, iGraphics3D)))
//    return false;
//
// inside engine.cpp (method csEngine::Initialize) and temporarily
// comment them out with an #if 0 ... #endif.
//

#include "cssysdef.h"
#include "cssys/system.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/csview.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "csparser/csloader.h"
#include "cssys/sysdriv.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"

#include "stdldr.h"

REGISTER_STATIC_LIBRARY (vfs)
REGISTER_STATIC_LIBRARY (engine)

class MyApp : public SysSystemDriver
{
public:
  MyApp () : SysSystemDriver () {}
  virtual bool CheckDrivers ()
  { return true; }
};

int main (int argc, char* argv[])
{
  System = new MyApp ();
  csEngine::System = System;

  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.engine.core:Engine");
  System->RequestPlugin ("crystalspace.graphics3d.infinite:VideoDriver");

  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    exit (1);
  }

  csStandardLoader *ldr = new csStandardLoader (NULL);
  if (!ldr->Initialize (System))
  {
    System->Printf (MSG_FATAL_ERROR, "Failed to initialize loader\n");
    exit (1);
  }

  printf ("Success: %d\n", ldr->Load ("/this/plugins/csstdldr/test/map.test"));

  delete ldr;
//delete System;

  return 0;
}
