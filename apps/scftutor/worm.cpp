/*
    This program demonstrates a simple SCF object with the ability to
    create and return another object with same interface.
*/

#include "cssysdef.h"
#include "iworm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class csWorm : public iWorm
{
  int length;
  int number;
public:
  SCF_DECLARE_IBASE;

  csWorm (iBase *iParent);
  virtual ~csWorm ();
  virtual void Crawl ();
  virtual int Length ();
  virtual iWorm *Split (int iLen1, int iLen2);
  void SetLength (int iLength)
  { length = iLength; }
};

//--------------- implementation ----------------

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_IBASE (csWorm)
  SCF_IMPLEMENTS_INTERFACE (iWorm)
SCF_IMPLEMENT_IBASE_END

static int worm_count = 1;

csWorm::csWorm (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  length = 50 + (rand () % 50);
  number = worm_count++;
}

csWorm::~csWorm ()
{
  printf ("Worm %d dies ...\n", number);
}

void csWorm::Crawl ()
{
  printf ("Worm %d crawls ...\n", number);
}

int csWorm::Length ()
{
  return length;
}

iWorm *csWorm::Split (int iLen1, int iLen2)
{
  int newlength = length * iLen1 / (iLen1 + iLen2);

  if (newlength >= length)
    return NULL;

  csWorm *newworm = new csWorm (scfParent);
  newworm->SetLength (length - newlength);
  length = newlength;
  return newworm;
}

// ... and now export all classes

SCF_IMPLEMENT_FACTORY (csWorm)

SCF_EXPORT_CLASS_TABLE (worm)
  SCF_EXPORT_CLASS (csWorm, "test.worm", "A Worm that crawls")
SCF_EXPORT_CLASS_TABLE_END
