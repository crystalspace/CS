#ifndef __CS_AWS_ITEM_VECTOR_H__
#define __CS_AWS_ITEM_VECTOR_H__

# include "csutil/parray.h"

struct awsListRow;

/// Holds a vector of awsListRows
class awsListRowVector : public csPDelArray<awsListRow>
{
public:
  int local_sortcol;
  static int sortcol;
public:
  awsListRowVector () : local_sortcol (0) { }

  /// Compare two array elements in given Mode
  static int Compare (awsListRow const* Item1, awsListRow const* Item2);

  /// Compare entry with a key
  static int CompareKey (awsListRow const* Item, void* Key);

  /// Set the sort column
  void SetSortCol (int sc)  { local_sortcol = sc; }
};
#endif // __CS_AWS_ITEM_VECTOR_H__
