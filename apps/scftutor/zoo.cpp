/*
    This program demonstrates several aspects of SCF usage:

    - Dynamic class loading/unloading (Dog and Worm classes)
    - Always-static class linkage (Frog class)
    - Object-created-within-object (Worm::Split method)
    - Run-time class registration with SCF kernel (no need for scf.cfg)
    - Embedded interface (iName within iDog)
    - Multiple interfaces (iFrog also supports iName)
*/

#include <stdio.h>
#include "sysdef.h"
#include "csutil/scf.h"
#include "csutil/inifile.h"

#include "iname.h"
#include "idog.h"
#include "iworm.h"
#include "ifrog.h"

// for the case we're using static linking ...
#ifdef CS_STATIC_LINKED
  REGISTER_STATIC_LIBRARY (Dog)
  REGISTER_STATIC_LIBRARY (Worm)
#endif

// frog is always statically linked
REGISTER_STATIC_LIBRARY (Frog)

int main ()
{
#if 0
  // This method requires you register dlls with scfreg (or manually) in scf.cfg
  csIniFile config ("scf.cfg");
  scfInitialize (&config);
#else
  // Don't use a .cfg file, instead manually register classes
  scfInitialize (NULL);
  scfRegisterClass ("test.dog", "Dog");
  scfRegisterClass ("test.worm", "Worm");
#endif

  iDog *dog = CREATE_INSTANCE ("test.dog", iDog);
  if (!dog)
    fprintf (stderr, "No csDog shared class!\n");
  else
  {
    iName *name = QUERY_INTERFACE (dog, iName);
    if (!name)
      fprintf (stderr, "dog does not support iName interface!\n");
    else
    {
      name->SetName ("Droopy");
      dog->Walk ();
      dog->Barf ("hello!");
      printf ("Dog's name is %s\n", name->GetName ());
      name->DecRef ();
    }
    dog->DecRef ();
  }

  printf ("----------------\n");

  iWorm *worm = CREATE_INSTANCE ("test.worm", iWorm);
  if (!worm)
    fprintf (stderr, "No csWorm shared class!\n");
  else
  {
    worm->Crawl ();
    printf ("Worm 1 length is %d\n", worm->Length ());
    printf ("Splitting worm into two ...\n");
    iWorm *worm2 = worm->Split (60, 40);
    if (!worm2)
      fprintf (stderr, "Failed to split the worm!\n");
    else
    {
      printf ("Worm1 length: %d  Worm2 length: %d\n", worm->Length (), worm2->Length ());
      worm2->Crawl ();
      worm2->DecRef ();
    }

    worm->DecRef ();
  }

  printf ("----------------\n");

  iFrog *frog = CREATE_INSTANCE ("test.frog", iFrog);
  if (!frog)
    fprintf (stderr, "No csFrog shared class!\n");
  else
  {
    iName *name = QUERY_INTERFACE (frog, iName);
    if (!name)
      fprintf (stderr, "frog does not support iName interface!\n");
    else
    {
      name->SetName ("Froggy");
      frog->Jump ();
      frog->Croak ("Barf");
      printf ("Frog's name is %s\n", name->GetName ());
      name->DecRef ();
    }

    frog->DecRef ();
  }

  scfFinish ();
}
