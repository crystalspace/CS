/*
    Crystal Space Windowing System: main interface file
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

#ifndef __CS_CSWS_H__
#define __CS_CSWS_H__

/**
 * \addtogroup csws
 * @{ */
 
/**
 * \page CSWSHierachy Current CrystalSpace Windowing System class hierarchy
 * <pre>
 *   |--[ csRect ]			// Rectangle
 *   |--[ csGraphicsPipeline ]  	// Deferred drawing pipeline
 *   +--[ csComponent ]         	// Windowing System component
 *        |--[ csMouse ]		// Mouse cursor
 *        |--[ csApp ]			// Windowing System application
 *        |--[ csWindow ]		// A window with titlebar, menu, client window etc
 *        |--[ csTitleBar ]		// title bar
 *        |--[ csMenu ]			// popup menu / menu bar
 *        |--[ csDialog ]		// dialog client window
 *        |--[ csStatic ]		// static control
 *        |    +--[ csColorWheel ]	// color wheel control
 *        |--[ csButton ]		// button control
 *        |    |--[ csCheckBox ]	// check box control
 *        |    +--[ csRadioButton ]	// radio button control
 *        |--[ csScrollBar ]		// scroll bar control
 *        |--[ csInputLine ]		// input line control
 *        |    +--[ csSpinBox ]		// spin box control
 *        |--[ csListBox ]		// list box control
 *        |--[ csNotebook ]		// notebook control
 *        |--[ csGrid ]			// grid control
 *        |--[ csSplitter ]		// splitter control
 *        |--[ csTreeCtrl ]		// tree control
 *        +--[ csLayout ]		// layout control
 *        |    |--[ csLayout2 ]	        // layout control 2
 *        |    |--[ csAbsolueLayout ]	// absolute layout
 *        |    |--[ csBoxLayout ]	// box layout
 *        |    |--[ csFlowLayout ]	// flow layout
 *        |    |--[ csBorderLayout ]	// border layout
 *        |    |--[ csGridLayout ]	// grid layout
 *        |    |--[ csGridBagLayout ]	// gridbag layout

 * </pre>
 */

// Forward declarations
class csRect;
class csEventQueue;
class csGraphicsPipeline;
class csComponent;
class csLayout;
class csLayout2;
class csAbsoluteLayout;
class csBorderLayout;
class csBoxLayout;
class csFlowLayout;
class csGridLayout;
class csGridBagLayout;
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

struct iVFS;
struct iGraphics2D;
struct iGraphics3D;
struct iEvent;

#ifndef CSWS_INTERNAL

// Include all Windowing System components
#include "csgeom/csrect.h"		// Rectangle class
#include "csutil/csevent.h"		// Event class
#include "csutil/cseventq.h"		// Event Queue class
#include "csutil/csinput.h"		// Keyboard codes
#include "cstool/cspixmap.h"		// 2D sprites
#include "csgfxppl.h"			// Graphics pipeline
#include "cskeyacc.h"			// Keyboard accelerator class
#include "cscomp.h"			// Windowing System Component
#include "csmouse.h"			// Mouse manager class
#include "csstatic.h"			// Static components
#include "cscwheel.h"			// Color wheel components
#include "csbutton.h"			// Buttons
#include "cschkbox.h"			// CheckBox buttons
#include "csradbut.h"			// Radio buttons
#include "csttlbar.h"			// Window title bar
#include "csscrbar.h"			// Scroll bars
#include "csiline.h"			// Input line
#include "csspinbx.h"			// Spin boxes
#include "csmenu.h"			// Menu class
#include "cslistbx.h"			// List box class
#include "cstree.h"			// Tree control class
#include "csnotebk.h"			// Notebook class
#include "csgrid.h"			// Grid class
#include "cssplit.h"			// Splitter class
#include "csdialog.h"			// User dialogs
#include "cswindow.h"			// Window class
#include "cswstex.h"			// Windowing System textures
#include "csapp.h"			// Windowing System application
#include "cswsutil.h"			// Windowing System shortcuts and utilites
#include "csskin.h"			// Windowing System skin management
#include "cslayout.h"                   // layout
#include "csabslay.h"                   // absolute layout
#include "csboxlay.h"                   // box layout
#include "csbdrlay.h"                   // border layout
#include "csflwlay.h"                   // flow layout
#include "csgrdlay.h"                   // grid layout
#include "csbaglay.h"                   // gridbag layout
#include "csstddlg.h"			// Default dialogs (file, color, ...)
// Include all known skins here
#include "sdefault.h"

#endif // CSWS_INTERNAL

/** @} */

#endif // __CS_CSWS_H__
