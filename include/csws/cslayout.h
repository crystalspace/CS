/*
    Copyright (C) Aleksandras Gluchovas
    CS port by Norman Kraemer <norman@users.sourceforge.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSLAYOUT_H__
#define __CS_CSLAYOUT_H__

/**
 * \addtogroup csws_layout
 * @{ */
 
#include "csextern.h"
 
#include "csws/csdialog.h"
#include "csgeom/cspoint.h"
#include "csutil/parray.h"

/**
 * csLayoutConstraint is a basic constraint used for positioning a control
 * in a csLayout derived component.
 */
class CS_CRYSTALSPACE_EXPORT csLayoutConstraint
{
public:
  /// the attached component
  csComponent *comp;
public:
  /// Constructor: initialize the object with zero
  csLayoutConstraint ()
  { comp = 0; }
  /// Constructor: initialize the object with given value
  csLayoutConstraint (csComponent *comp)
  { this->comp = comp; }
  /// the destructor ... nothing much to say about.
  virtual ~csLayoutConstraint () {}
  /// make a copy of this constraint
  virtual csLayoutConstraint *Clone ();
};

/**
 * The layout classes collect the constraints in here.
 * The constraints (and thus the components to place) are
 * taken into account in the sequence they are added to this vector.
 */
class CS_CRYSTALSPACE_EXPORT csConstraintVector :
  public csPDelArray<csLayoutConstraint>
{
public:
  /// Look up an constraint given a components.
  static int CompareKey (csLayoutConstraint* const& Item1,
			 csComponent* const& Item2)
  {
    return (Item1->comp < Item2 ? -1 : Item1->comp > Item2 ? 1 : 0);
  }
  /// Return a functor wrapping CompareKey() for a given component.
  static csArrayCmp<csLayoutConstraint*,csComponent*> KeyCmp(csComponent* c)
  {
    return csArrayCmp<csLayoutConstraint*,csComponent*>(c, CompareKey);
  }
};

/**
 * \page WhatAreLayoutsFor What are layouts for?
 *
 * Usually one designs dialogs, forms etc. by placing components like
 * listcontrols, inputlines, treecontrols and so on at absolute locations
 * inside the dialog. While this is fine for a fixed size of your dialog
 * it turns out to be pretty ugly if you want allow resizing of dialogs,
 * forms etc. A comparable ugly effect you achive by running an application
 * in different window sizes.
 *
 * Layouts will help you to overcome this drawback.
 * They will allow you to relatively place a control and to resize
 * components when necessary.
 *
 * Layouts are themselves csComponents and have a transparent canvas.
 * Thus you will not note them. One important issue about layouts is
 * that they will transfer all Events of type csevCommand to its parent
 * control. This will allow you overwrite just one HandleEvent to receive
 * all commands from the components embedded in the layouts no matter how
 * deeply nested they are.
 */

/**
 * csLayout is our baseclass for various derived classes like
 * csFlowLayout, csBoxLayout, csGridBagLayout and others.
 */
class CS_CRYSTALSPACE_EXPORT csLayout : public csDialog
{
protected:
  /**
   * A shortcoming of the original java layouts is that components are
   * asked for its preferred size without knowing at what size its parent
   * component will be layout in the end. So the two phase layout is an
   * attempt to overcome this. Currently only FlowLayout uses this.
   */
  static bool mUseTwoPhaseLayoutingGlobally;
  static int mCurrentLayoutingPhase;
  /// do we need to recalc the positions and sizes of placed components ?
  bool bRecalcLayout;
  /// collect all constraints here
  csConstraintVector vConstraints;
  /// a pointer to the current constraint
  csLayoutConstraint *lc;

public:
  /// preserve space at the 4 borders of a layout
  csRect insets;
  enum LAYOUTING_PHASES {PHASE_0 = 0, PHASE_1 = 1};
  /**
   * Here we have the constraint a components will be layout with.
   * When a component is added a copy of this will be made and
   * attached to the component.
   */
  csLayoutConstraint c;

public:
  csLayout (csComponent *iParent, csDialogFrameStyle iFrameStyle = csdfsNone);

  /**
   * A components is added to a layout by creating it and passing
   * the layout component as its parent component.
   * If you insist of doing some voodoo you should use AddLayoutComponent
   * to add it to the layout.
   * <pre>
   * IN:  the component to add
   * OUT: the constraint that is used to layout the component
   *      (a copy of variable c  see above)
   * </pre>
   */
  virtual csLayoutConstraint *AddLayoutComponent (csComponent *comp);
  /// remove a component from the layout
  virtual void RemoveLayoutComponent (csComponent *comp);
  /// return best size for this layout
  virtual void SuggestSize (int &sugw, int& sugh) = 0;
  /// recalc positions and sizes of components
  virtual void LayoutContainer () = 0;
  /// make sure next time the layout is drawn the components are
  /// layed out first
  virtual void InvalidateLayout ();

  /// return current phase of layouting
  virtual int GetLayoutingPhase ();
  /// set current phase of layouting
  virtual void SetLayoutingPhase (int phase);
  /// save size of first phase for later reference
  virtual csPoint GetPhase0Size ();
  /// is the two phase layouting enabled ?
  virtual bool TwoPhaseLayoutingEnabled ();
  /// enable or disable 2 phase layouting
  static void SetTwoPhaseLayoutingGlobally (bool on);

  /// new impl. for csComponent
  virtual void Insert (csComponent *child);
  virtual bool HandleEvent (iEvent &Event);
  virtual void Draw ();
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  virtual void FixSize (int &newWidth, int &newHeight);
};

/**
 * csLayout2 extends csLayout to take the maximum layout size and
 * aligning along the x and y axis into account.
 */
class CS_CRYSTALSPACE_EXPORT csLayout2 : public csLayout
{
 public:
  csLayout2 (csComponent *pParent);

  virtual void MaximumLayoutSize (int &w, int &h) = 0;
  virtual float GetLayoutAlignmentX () = 0;
  virtual float GetLayoutAlignmentY () = 0;
};

/** @} */

#endif // __CS_CSLAYOUT_H__
