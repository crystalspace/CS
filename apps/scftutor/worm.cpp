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
  DECLARE_IBASE;

  csWorm (iBase *iParent);
  virtual ~csWorm ();
  virtual void Crawl ();
  virtual int Length ();
  virtual iWorm *Split (int iLen1, int iLen2);
  void SetLength (int iLength)
  { length = iLength; }
};

//--------------- implementation ----------------

IMPLEMENT_IBASE (csWorm)
  IMPLEMENTS_INTERFACE (iWorm)
IMPLEMENT_IBASE_END

static int worm_count = 1;

csWorm::csWorm (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
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

IMPLEMENT_FACTORY (csWorm)

EXPORT_CLASS_TABLE (worm)
  EXPORT_CLASS (csWorm, "test.worm", "A Worm that crawls")
EXPORT_CLASS_TABLE_END
