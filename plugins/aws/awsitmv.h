#ifndef __CS_AWS_ITEM_VECTOR_H__
#define __CS_AWS_ITEM_VECTOR_H__

# include "csutil/csvector.h"

/// Holds a vector of awsListRows
class awsListRowVector :
  public csVector
{
  int sortcol;
public:
  awsListRowVector ();
  virtual ~awsListRowVector ();

  /// Virtual function which frees a vector element; returns success status
  virtual bool FreeItem (void* Item);

  /// Compare two array elements in given Mode
  virtual int Compare (void* Item1, void* Item2, int Mode) const;

  /// Compare entry with a key
  virtual int CompareKey (void* Item, const void* Key, int Mode) const;

  /// Set the sort column
  void SetSortCol (int sc)  { sortcol = sc; }
};
#endif // __CS_AWS_ITEM_VECTOR_H__
