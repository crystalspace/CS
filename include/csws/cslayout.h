/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __CSLAYOUT_H__
#define __CSLAYOUT_H__

#include "csws/cscomp.h"

/**
 * A 2D point object
 */
struct csPoint
{
  /// The coordinates of the point
  int x, y;

  /// Constructor: initialize the object with zeros
  csPoint ()
  { x = y = 0; }
  /// Constructor: initialize the object with given values
  csPoint (int iX, int iY)
  { x = iX; y = iY; }
  /// Set the coordinates of this point
  void Set (int iX, int iY)
  { x = iX; y = iY; }
};

/**
 *
 *
 */
class csLayoutConstraint
{
public:
  csComponent *comp;
public:
  csLayoutConstraint ()
  { comp = NULL; }
  csLayoutConstraint (csComponent *comp)
  { this->comp = comp; }
  virtual ~csLayoutConstraint () { }
  virtual csLayoutConstraint *Clone ();
};

/**
 *
 *
 */
class csConstraintVector : public csVector
{
public:
  virtual int Compare (csSome Item1, csSome Item2, int Mode = 0) const
  {
    (void)Mode;
    csLayoutConstraint *c1 = (csLayoutConstraint *)Item1, *c2 = (csLayoutConstraint *)Item2;
    return (c1->comp < c2->comp ? -1 : c1->comp > c2->comp ? 1 : 0);
  }
  virtual int CompareKey (csSome Item1, csConstSome Item2, int Mode = 0) const
  {
    (void)Mode;
    csLayoutConstraint *c1 = (csLayoutConstraint *)Item1;
    csComponent *c2 = (csComponent *)Item2;
    return (c1->comp < c2 ? -1 : c1->comp > c2 ? 1 : 0);
  }
  virtual bool FreeItem (csSome Item)
  { if (Item) delete (csLayoutConstraint *)Item; return true; }
  csLayoutConstraint *Get (int idx)
  { return (csLayoutConstraint *)csVector::Get (idx); }
};

/**
 *
 *
 */
class csLayout : public csComponent
{
protected:
  static bool mUseTwoPhaseLayoutingGlobally;
  static int mCurrentLayoutingPhase;
  bool bRecalcLayout;
  csConstraintVector vConstraints;
  csLayoutConstraint *lc;

public:
  csRect insets;
  enum LAYOUTING_PHASES {PHASE_0 = 0, PHASE_1 = 1};
  csLayoutConstraint c;

public:
  csLayout (csComponent *pParent);

  virtual csLayoutConstraint *AddLayoutComponent (csComponent *comp);
  virtual void RemoveLayoutComponent (csComponent *comp);
  virtual void SuggestSize (int &sugw, int& sugh) = 0;
  virtual void LayoutContainer () = 0;
  virtual void InvalidateLayout ();

  virtual int GetLayoutingPhase ();
  virtual void SetLayoutingPhase (int phase);
  virtual csPoint GetPhase0Size ();
  virtual bool TwoPhaseLayoutingEnabled ();
  static void SetTwoPhaseLayoutingGlobally (bool on);

  /// new impl. for csComponent
  virtual void Insert (csComponent *child);
  virtual bool HandleEvent (iEvent &Event);
  virtual void Draw ();
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  virtual void FixSize (int &newWidth, int &newHeight);
};

/**
 *
 *
 */
class csLayout2 : public csLayout
{
 public:
  csLayout2 (csComponent *pParent);

  virtual void MaximumLayoutSize (int &w, int &h) = 0;
  virtual float GetLayoutAlignmentX () = 0;
  virtual float GetLayoutAlignmentY () = 0;
};

#endif // __CSLAYOUT_H__
