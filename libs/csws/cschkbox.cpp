/*
    Crystal Space Windowing System: check box button class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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
#include <string.h>
#include "csws/cschkbox.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "csws/csskin.h"

#define CHECKBOX_TEXTURE_NAME	"csws::CheckBox"

csCheckBox::csCheckBox (csComponent *iParent, int iButtonID, int iButtonStyle)
  : csButton (iParent, cscmdNothing, iButtonStyle, csbfsNone)
{
  id = iButtonID;
  CheckBoxState = cscbsIndefinite;
  SetCheckBoxState (cscbsNonChecked);
  SetSuggestedSize (0, 0);
}

void csCheckBox::Press ()
{
  csButton::Press ();
  if (ButtonStyle & CSBS_CBAUTO)
  {
    switch (ButtonStyle & CSBS_CBTYPEMASK)
    {
      case CSBS_CB2STATE:
        if (CheckBoxState == cscbsNonChecked)
          SetCheckBoxState (cscbsChecked);
        else
          SetCheckBoxState (cscbsNonChecked);
        break;
      case CSBS_CB3STATE:
        if (CheckBoxState == cscbsNonChecked)
          SetCheckBoxState (cscbsChecked);
        else if (CheckBoxState == cscbsChecked)
          SetCheckBoxState (cscbsIndefinite);
        else
          SetCheckBoxState (cscbsNonChecked);
        break;
    }
  } /* endif */
}

void csCheckBox::SetButtBitmap (char *id_n, char *id_p)
{
  int tx,ty,tw,th;
  ParseConfigBitmap (app, app->skin->Prefix, "Dialog", id_n, tx, ty, tw, th);
  csPixmap *bmpn = new csSimplePixmap (app->GetTexture (
    CHECKBOX_TEXTURE_NAME), tx, ty, tw, th);
  ParseConfigBitmap (app, app->skin->Prefix, "Dialog", id_p, tx, ty, tw, th);
  csPixmap *bmpp = new csSimplePixmap (app->GetTexture (
    CHECKBOX_TEXTURE_NAME), tx, ty, tw, th);
  SetBitmap (bmpn, bmpp);
}

void csCheckBox::SetCheckBoxState (csCheckBoxState iNewState)
{
  if (iNewState == CheckBoxState)
    return;
  switch (iNewState)
  {
    case cscbsNonChecked:
      SetButtBitmap ("CHKOFFN", "CHKOFFP");
      CheckBoxState = iNewState;
      break;
    case cscbsChecked:
      SetButtBitmap ("CHKONN", "CHKONP");
      CheckBoxState = iNewState;
      break;
    case cscbsIndefinite:
      SetButtBitmap ("CHK3SN", "CHK3SP");
      CheckBoxState = iNewState;
      break;
  } /* endswitch */
  if (parent)
    parent->SendCommand (cscmdCheckBoxSwitched, (intptr_t)this);
}

bool csCheckBox::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdAreYouDefault:
          return true;
        case cscmdCheckBoxSet:
          SetCheckBoxState ((csCheckBoxState)(int)Event.Command.Info);
          return true;
        case cscmdCheckBoxQuery:
          Event.Command.Info = (intptr_t)CheckBoxState;
          return true;
      } /* endswitch */
      break;
  } /* endswitch */
  return csButton::HandleEvent (Event);
}
