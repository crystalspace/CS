#ifndef __AWS_ITEM_VECTOR__
#define __AWS_ITEM_VECTOR__

#include "csutil/csvector.h"

/// Holds a vector of awsListRows
class awsListRowVector : public csVector
{
  int sortcol;
  
public:
  awsListRowVector();
  virtual ~awsListRowVector();

   /// Virtual function which frees a vector element; returns success status
  virtual bool FreeItem (csSome Item);

  /// Compare two array elements in given Mode
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;

  /// Compare entry with a key
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

  /// Set the sort column
  void SetSortCol(int sc)
  { sortcol = sc; }
};

#endif

