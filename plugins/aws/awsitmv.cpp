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

bool awsListRowVector::FreeItem (void* Item)
{
  delete (awsListRow *)Item;

  return true;
}

int awsListRowVector::Compare (void* Item1, void* Item2, int

/*Mode*/ ) const
{
  awsListRow *r1 = (awsListRow *)Item1;
  awsListRow *r2 = (awsListRow *)Item2;

  if (r1->cols[sortcol].text && r2->cols[sortcol].text)
    return strcmp (r1->cols[sortcol].text->GetData (), r2->cols[sortcol].text->GetData ());
  else if (r1->cols[sortcol].text)
    return 1;
  else if (r2->cols[sortcol].text)
    return -1;
  else
    return 0;
}

int awsListRowVector::CompareKey (
  void* Item,
  const void* Key,
  int

  /*Mode*/ ) const
{
  awsListRow *r1 = (awsListRow *)Item;

  if (r1->cols[sortcol].text)
    return strcmp (r1->cols[sortcol].text->GetData (), ((iString *)Key)->GetData ());
  else
    return -1;
}
