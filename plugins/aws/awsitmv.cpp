#include "cssysdef.h"
#include "awsitmv.h"
#include "awslstbx.h"

awsListRowVector::awsListRowVector () :
  sortcol(0)
{
}

awsListRowVector::~awsListRowVector ()
{
}

bool awsListRowVector::FreeItem (csSome Item)
{
  delete (awsListRow *)Item;

  return true;
}

int awsListRowVector::Compare (csSome Item1, csSome Item2, int

/*Mode*/ ) const
{
  awsListRow *r1 = (awsListRow *)Item1;
  awsListRow *r2 = (awsListRow *)Item2;

  if (r1->cols[sortcol].text && r2->cols[sortcol].text)
    return r1->cols[sortcol].text->Compare (r2->cols[sortcol].text);
  else if (r1->cols[sortcol].text)
    return 1;
  else if (r2->cols[sortcol].text)
    return -1;
  else
    return 0;
}

int awsListRowVector::CompareKey (
  csSome Item,
  csConstSome Key,
  int

  /*Mode*/ ) const
{
  awsListRow *r1 = (awsListRow *)Item;

  if (r1->cols[sortcol].text)
    return r1->cols[sortcol].text->Compare ((iString *)Key);
  else
    return -1;
}
