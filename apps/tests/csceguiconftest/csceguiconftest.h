/*
    Copyright (C) 2010 Jelle Hellemans

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __CSCEGUICONFTEST_H__
#define __CSCEGUICONFTEST_H__

#include "cstool/demoapplication.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

#include "ivaria/icegui.h"

#include "csutil/cfgnotifier.h"

struct iSector;

class CSCEGUIConfTest : public CS::Utility::DemoApplication
{
private:
  int myInt;
  float myFloat;
  bool myBool;
  csString myString;

private:
  csRef<iCEGUI> cegui;
  csRef<iConfigListener> configEventNotifier;

  csRef<iEventHandler> myIntL;
  csRef<iEventHandler> myFloatL;
  csRef<iEventHandler> myBoolL;
  csRef<iEventHandler> myStringL;

  void Frame();

public:
  CSCEGUIConfTest();
  ~CSCEGUIConfTest();

  void PrintHelp ();

  // Handle exit button clicked event
  bool OnListSelection (const CEGUI::EventArgs& e);

  bool OnInitialize(int argc, char* argv[]);

  bool Application();
  
  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.csceguiconftest")
      
  /* Declare that we want to receive events *after* the CEGUI plugin. */
  virtual const csHandlerID * GenericPrec (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    static csHandlerID precConstraint[2];
    
    precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
    precConstraint[1] = CS_HANDLERLIST_END;
    return precConstraint;
  }

  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
};

#endif
