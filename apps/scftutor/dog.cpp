/*
    This program demonstrates an SCF object with an embedded interface.
*/

#include "cssysdef.h"
#include "idog.h"
#include "iname.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class csDog : public iDog
{
  char *Name;

  // Embedded interface
  class csName : public iName
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csDog);

    virtual char *GetName ();
    virtual void SetName (char *iName);
  } scfiName;
  friend class csName;

public:
  SCF_DECLARE_IBASE;

  csDog (iBase *iParent);
  virtual ~csDog ();
  virtual void Walk ();
  virtual void Barf (char *iWhat);
};

//--------------- implementation ----------------

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csDog)
  SCF_IMPLEMENTS_INTERFACE (iDog)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iName)
SCF_IMPLEMENT_IBASE_END

csDog::csDog (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiName);
  Name = NULL;
}

csDog::~csDog ()
{
  printf ("Dog %s dies ...\n", Name);
  if (Name)
    free (Name);
}

void csDog::Walk ()
{
  printf ("%s: I'm walking\n", Name);
}

void csDog::Barf (char *iWhat)
{
  printf ("I'm %s: barf, barf, %s\n", Name, iWhat);
}

// IName interface for dog

SCF_IMPLEMENT_EMBEDDED_IBASE (csDog::csName)
  SCF_IMPLEMENTS_INTERFACE (iName)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

char *csDog::csName::GetName ()
{
  return scfParent->Name;
}

void csDog::csName::SetName (char *iName)
{
  if (scfParent->Name)
    free (scfParent->Name);
  scfParent->Name = strdup (iName);
}

// ... and now export all classes

SCF_IMPLEMENT_FACTORY (csDog)

SCF_EXPORT_CLASS_TABLE (dog)
  SCF_EXPORT_CLASS (csDog, "test.dog", "A Dog that barfs")
SCF_EXPORT_CLASS_TABLE_END
