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
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"

#include "iname.h"
#include "idog.h"
#include "iworm.h"
#include "ifrog.h"

// for the case we're using static linking ...
#ifdef CS_STATIC_LINKED
  REGISTER_STATIC_LIBRARY (dog)
  REGISTER_STATIC_LIBRARY (worm)
#endif

// frog is always statically linked
REGISTER_STATIC_LIBRARY (Frog)

// This function will clone given object, do something with him
// and finally destroy
void Clone (iBase *iObject)
{
  printf ("--- cloning the object\n");
  iFactory *factory = QUERY_INTERFACE (iObject, iFactory);
  if (!factory)
  {
    fprintf (stderr, "Object does not support the iFactory interface!\n");
    return;
  }
  printf ("Class description: %s\n", factory->QueryDescription ());

  // Create a new instance of the class
  iBase *newobj = (iBase *)factory->CreateInstance ();

  // Release the factory interface of the parent - we don't need it anymore
  factory->DecRef ();

  if (!newobj)
  {
    fprintf (stderr, "Failed to create a object of the same type!\n");
    return;
  }

  // Check if the object supports the iName interface
  iName *name = QUERY_INTERFACE (newobj, iName);
  if (name)
  {
    if (!name->GetName())
      printf ("Object is unnamed; renaming to \"Clone\"\n");
    else
      printf ("Object's name is \"%s\"; renaming to \"Clone\"\n",
        name->GetName ());
    name->SetName ("Clone");
    name->DecRef ();
  }

  // Delete the clone
  newobj->DecRef ();
}

//Changing this causes a link2001 error for win32.
//int main(int,char **)
int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
#if 0
  // This method requires you register dlls with scfreg (or manually) in scf.cfg
  csConfigFile config ("scf.cfg");
  scfInitialize (&config);
#else
  // Don't use a .cfg file, instead manually register classes
  scfInitialize (NULL);
  scfRegisterClass ("test.dog", "dog");
  scfRegisterClass ("test.worm", "worm");
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

    Clone (dog);

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

    Clone (worm);

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

    Clone (frog);

    frog->DecRef ();
  }
  scfFinish ();
  return 0;
}
