#include "cssysdef.h"
#include "awsitmv.h"
#include "awslstbx.h"

int awsListRowVector::sortcol = 0;

int awsListRowVector::Compare (awsListRow const* r1, awsListRow const* r2)
{
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
  awsListRow const* r1,
  void* Key)
{
  if (r1->cols[sortcol].text)
    return strcmp (r1->cols[sortcol].text->GetData (), ((iString *)Key)->GetData ());
  else
    return -1;
}
