/*
  Crystal Space Windowing System: main interface file
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSWS_H__
#define __CSWS_H__

/*
 * Current CrystalSpace Windowing System class hierarchy:<p>
 * <pre>
 * [ csBase ]
 *   |--[ csVector ]            	// Vector of some objects (FreeItem = NOP)
 *   |    |--[ csStrVector ]    	// Vector of strings (FreeItem = delete[])
 *   |    +--[ csObjVector ]    	// ... csBase heirs (FreeItem = delete (csBase *))
 *   |--[ csSprite2D ]			// A 2D sprite
 *   |--[ csEvent ]             	// Windowing System event
 *   |--[ csEventQueue ]        	// Event queue class
 *   |--[ csGraphicsPipeline ]  	// Deferred drawing pipeline
 *   +--[ csComponent ]         	// Windowing System component
 *        |--[ csMouse ]		// Mouse cursor
 *        |--[ csApp ]			// Windowing System application
 *        |--[ csWindow ]		// A window with titlebar, menu, client window etc
 *        |--[ csTitleBar ]		// title bar
 *        |--[ csMenu ]			// popup menu / menu bar
 *        |--[ csDialog ]		// dialog client window
 *        |--[ csStatic ]		// static control
 *        |--[ csButton ]		// button control
 *        |    |--[ csCheckBox ]	// check box control
 *        |    +--[ csRadioButton ]	// radio button control
 *        |--[ csScrollBar ]		// scroll bar control
 *        |--[ csInputLine ]		// input line control
 *        |    +--[ csSpinBox ]		// spin box control
 *        +--[ csListBox ]		// list box control
 *
 * [ csRect ]				// Rectangle (not related to above hierarchy)
 * </pre>
 */

// Forward declarations
class csBase;
class csRect;
class csVector;
class csStrVector;
class csObjVector;
class csEvent;
class csEventQueue;
class csGraphicsPipeline;
class csComponent;
class csMouse;
class csStatic;
class csScrollBar;
class csButton;
class csCheckBox;
class csRadioButton;
class csTitleBar;
class csInputLine;
class csMenu;
class csListBox;
class csDialog;
class csWindow;
class csApp;

// CrystalSpace headers
#include "cswsaux.h"

#ifndef CSWS_INTERNAL

// Include all Windowing System components
#include "csutil/csbase.h"		// Base class
#include "csgeom/csrect.h"		// Rectangle class
#include "csutil/csvector.h"		// Vector of objects
#include "csutil/csstrvec.h"		// Vector of string objects
#include "csengine/basic/csobjvec.h"	// Vector of csBase heirs
#include "csengine/csview.h"		// CrystalSpace view
#include "csinput/csevent.h"		// Event class
#include "csinput/cseventq.h"		// Event Queue class
#include "csinput/csinput.h"		// Keyboard codes
#include "csengine/2d/csspr2d.h"	// 2D sprites
#include "csgfxppl.h"			// Graphics pipeline
#include "cskeyacc.h"			// Keyboard accelerator class
#include "cscomp.h"			// Windowing System Component
#include "csmouse.h"			// Mouse manager class
#include "csstatic.h"			// Static components
#include "csbutton.h"			// Buttons
#include "cschkbox.h"			// CheckBox buttons
#include "csradbut.h"			// Radio buttons
#include "csttlbar.h"			// Window title bar
#include "csscrbar.h"			// Scroll bars
#include "csiline.h"			// Input line
#include "csspinbx.h"			// Spin boxes
#include "csmenu.h"			// Menu class
#include "cslistbx.h"			// List box class
#include "csdialog.h"			// User dialogs
#include "cswindow.h"			// Window class
#include "csapp.h"			// Windowing System application
#include "cswsutil.h"			// Windowing System shortcuts and utilites

#endif // CSWS_INTERNAL

#endif // __CSWS_H__
