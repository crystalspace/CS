/*
    Copyright (C) 2000 by Norman Kraemer

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

#include "cssysdef.h"
#include "csws/cslayout.h"
#include "csws/cswindow.h"
#include "iutil/event.h"

csLayoutConstraint *csLayoutConstraint::Clone ()
{
  return new csLayoutConstraint (comp);
}

bool csLayout::mUseTwoPhaseLayoutingGlobally = true;
int csLayout::mCurrentLayoutingPhase = csLayout::PHASE_1;

csLayout::csLayout (csComponent *iParent, csDialogFrameStyle iFrameStyle)
  : csDialog (iParent, iFrameStyle)
{
  bRecalcLayout = true;
  lc = &c;
  SetState (CSS_TRANSPARENT, true);
}

int csLayout::GetLayoutingPhase ()
{
  return mCurrentLayoutingPhase;
}

void csLayout::SetLayoutingPhase (int phase)
{
  mCurrentLayoutingPhase = phase;
}

csPoint csLayout::GetPhase0Size ()
{
  return csPoint (bound.Width (), bound.Height ());
}

bool csLayout::TwoPhaseLayoutingEnabled ()
{
  return mUseTwoPhaseLayoutingGlobally;
}

void csLayout::SetTwoPhaseLayoutingGlobally (bool on)
{
  mUseTwoPhaseLayoutingGlobally = on;
}

csLayoutConstraint *csLayout::AddLayoutComponent (csComponent* comp)
{
  csLayoutConstraint *constr = 0;
  if (lc)
  {
    constr = lc->Clone ();
    constr->comp = comp;
    vConstraints.Push (constr);
  }
  InvalidateLayout ();
  return constr;
}

void csLayout::RemoveLayoutComponent (csComponent* comp)
{
  int idx = vConstraints.FindKey (vConstraints.KeyCmp(comp));
  if (idx != -1)
    vConstraints.DeleteIndex (idx);
  InvalidateLayout ();
}

void csLayout::InvalidateLayout ()
{
  bRecalcLayout = true;
}

bool csLayout::HandleEvent (iEvent &Event)
{
  bool bHandled = csDialog::HandleEvent (Event);
  if (!bHandled && Event.Type == csevCommand)
    bHandled = (parent ? parent->HandleEvent (Event) : false);
  return bHandled;
}

void csLayout::Draw ()
{
  if (bRecalcLayout)
  {
    LayoutContainer ();
    bRecalcLayout = false;
  }
}

bool csLayout::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csDialog::SetRect (xmin, ymin, xmax, ymax))
  {
    InvalidateLayout ();
    return true;
  }
  return false;
}

void csLayout::FixSize (int &newWidth, int &newHeight)
{
  // we just note the new size
  SetSize (newWidth, newHeight);
  InvalidateLayout ();
}

void csLayout::Insert (csComponent *child)
{
  csDialog::Insert (child);
  AddLayoutComponent (child);
}

csLayout2::csLayout2 (csComponent *pParent) : csLayout (pParent)
{
}
