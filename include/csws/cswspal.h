/*
    Crystal Space Windowing System: CSWS palette definition
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

#ifndef __CSWSPAL_H__
#define __CSWSPAL_H__

/**
 * Control palette export structure
 */
struct csPaletteExport
{
  int *Palette;
  int Length;
};

/// A global array containing CSWS palette for all component types
extern csPaletteExport cswsPalette[];

/**
 * These are indexes into cswsPalette for each component type
 */
/// Application palette index
#define CSPAL_APP			0
/// Button palette index
#define CSPAL_BUTTON			1
/// Dialog palette index
#define CSPAL_DIALOG			2
/// Input line palette index
#define CSPAL_INPUTLINE			3
/// List box palette index
#define CSPAL_LISTBOX			CSPAL_INPUTLINE
/// Menu item palette index
#define CSPAL_MENUITEM			4
/// Menu palette index
#define CSPAL_MENU			5
/// Static component palette index
#define CSPAL_STATIC			6
/// Title bar palette index
#define CSPAL_TITLEBAR			7
/// Window palette index
#define CSPAL_WINDOW			8
/// List box item palette index
#define CSPAL_LISTBOXITEM		9
/// Scroll bar palette index
#define CSPAL_SCROLLBAR			10

/**
 * csApp class color palette indexes.<p>
 */
/// Workspace color index
#define CSPAL_APP_WORKSPACE	0

/**
 * csButton class color palette indexes.<p>
 */
/// button background
#define CSPAL_BUTTON_BACKGROUND		0
/// default button frame
#define CSPAL_BUTTON_DEFFRAME		1
/// 3D border dark
#define CSPAL_BUTTON_DARK3D		2
/// 3D border light
#define CSPAL_BUTTON_LIGHT3D		3
/// button text
#define CSPAL_BUTTON_TEXT		4
/// button disabled text
#define CSPAL_BUTTON_DTEXT		5

/**
 * csDialog class color palette indexes.<p>
 */
/// Background color index
#define CSPAL_DIALOG_BACKGROUND		0
/// 3D border dark
#define CSPAL_DIALOG_DARK3D		1
/// 3D border light
#define CSPAL_DIALOG_LIGHT3D		2
/// 2nd level 3D border dark
#define CSPAL_DIALOG_2DARK3D		3
/// 2nd level 3D border light
#define CSPAL_DIALOG_2LIGHT3D		4

/**
 * csButton class color palette indexes.<p>
 */
/// input line background
#define CSPAL_INPUTLINE_BACKGROUND	0
/// Background color for csifsThickRect
#define CSPAL_INPUTLINE_BACKGROUND2	1
/// 3D border dark
#define CSPAL_INPUTLINE_DARK3D		2
/// 3D border light
#define CSPAL_INPUTLINE_LIGHT3D		3
/// 2nd level 3D border dark
#define CSPAL_INPUTLINE_2DARK3D		4
/// 2nd level 3D border light
#define CSPAL_INPUTLINE_2LIGHT3D	5
/// input line text
#define CSPAL_INPUTLINE_TEXT		6
/// input line selection background
#define CSPAL_INPUTLINE_SELBACKGROUND	7
/// input line selected text
#define CSPAL_INPUTLINE_SELTEXT		8

/**
 * csListBox class color palette indexes.<p>
 */
/// list box background
#define CSPAL_LISTBOX_BACKGROUND	CSPAL_INPUTLINE_BACKGROUND
/// background color for cslfsThickRect
#define CSPAL_LISTBOX_BACKGROUND2	CSPAL_INPUTLINE_BACKGROUND2
/// 3D border dark
#define CSPAL_LISTBOX_DARK3D		CSPAL_INPUTLINE_DARK3D
/// 3D border light
#define CSPAL_LISTBOX_LIGHT3D		CSPAL_INPUTLINE_LIGHT3D
/// 2nd level 3D border dark
#define CSPAL_LISTBOX_2DARK3D		CSPAL_INPUTLINE_2DARK3D
/// 2nd level 3D border light
#define CSPAL_LISTBOX_2LIGHT3D		CSPAL_INPUTLINE_2LIGHT3D
/// list box text
#define CSPAL_LISTBOX_TEXT		CSPAL_INPUTLINE_TEXT
/// list box selection background
#define CSPAL_LISTBOX_SELBACKGROUND	CSPAL_INPUTLINE_SELBACKGROUND
/// list box selected text
#define CSPAL_LISTBOX_SELTEXT		CSPAL_INPUTLINE_SELTEXT

/**
 * csMenuItem class color palette indexes.<p>
 */
/// Unselected menu item background
#define CSPAL_MENUITEM_BACKGROUND	0
/// Selected menu item background
#define CSPAL_MENUITEM_SELECTION	1
/// Unselected text
#define CSPAL_MENUITEM_UTEXT		2
/// Selected text
#define CSPAL_MENUITEM_STEXT		3
/// Disabled text
#define CSPAL_MENUITEM_DTEXT		4
/// Separator 3D dark
#define CSPAL_MENUITEM_DARK3D		5
/// Separator 3D light
#define CSPAL_MENUITEM_LIGHT3D		6

/**
 * csMenu class color palette indexes.<p>
 */
/// Menu background
#define CSPAL_MENU_BACKGROUND		0
/// 3D border dark
#define CSPAL_MENU_DARK3D		1
/// 3D border light
#define CSPAL_MENU_LIGHT3D		2
/// 3D border 2nd level dark
#define CSPAL_MENU_2DARK3D		3
/// 3D border 2nd level light
#define CSPAL_MENU_2LIGHT3D		4

/**
 * csStatic class color palette indexes.<p>
 */
/// static component background
#define CSPAL_STATIC_BACKGROUND		0
/// static component text / inactive
#define CSPAL_STATIC_ITEXT		1
/// static component text / active
#define CSPAL_STATIC_ATEXT		2
/// 3D border dark
#define CSPAL_STATIC_DARK3D		3
/// 3D border light
#define CSPAL_STATIC_LIGHT3D		4

/**
 * csTitleBar class color palette indexes.<p>
 */
/// 3D border dark
#define CSPAL_TITLEBAR_DARK3D		0
/// 3D border light
#define CSPAL_TITLEBAR_LIGHT3D		1
/// passive title background
#define CSPAL_TITLEBAR_PBACKGROUND	2
/// passive title text
#define CSPAL_TITLEBAR_PTEXT		3
/// passive title 3D bars dark
#define CSPAL_TITLEBAR_PDARK3D		4
/// passive title 3D bars light
#define CSPAL_TITLEBAR_PLIGHT3D		5
/// active title background
#define CSPAL_TITLEBAR_ABACKGROUND	6
/// active title text
#define CSPAL_TITLEBAR_ATEXT		7
/// active title 3D bars dark
#define CSPAL_TITLEBAR_ADARK3D		8
/// active title 3D bars light
#define CSPAL_TITLEBAR_ALIGHT3D		9

/**
 * csWindow class color palette indexes.<p>
 */
/// 3D border dark
#define CSPAL_WINDOW_DARK3D		0
/// 3D border light
#define CSPAL_WINDOW_LIGHT3D		1
/// 3D border 2nd level dark
#define CSPAL_WINDOW_2DARK3D		2
/// 3D border 2nd level light
#define CSPAL_WINDOW_2LIGHT3D		3
/// border normal
#define CSPAL_WINDOW_BORDER		4

/**
 * csListBoxItem class color palette indexes.<p>
 */
/// Unselected listbox item background
#define CSPAL_LISTBOXITEM_BACKGROUND	0
/// Selected listbox item background
#define CSPAL_LISTBOXITEM_SELECTION	1
/// Thin rectangle around selected item
#define CSPAL_LISTBOXITEM_SELRECT	2
/// Disabled text
#define CSPAL_LISTBOXITEM_DTEXT		3
/// Unselected normal text
#define CSPAL_LISTBOXITEM_UNTEXT	4
/// Selected normal text
#define CSPAL_LISTBOXITEM_SNTEXT	5
/// Unselected emphasized text
#define CSPAL_LISTBOXITEM_UETEXT	6
/// Selected emphasized text
#define CSPAL_LISTBOXITEM_SETEXT	7

/**
 * csScrollBar class color palette indexes.<p>
 */
/// Scroll bar background
#define CSPAL_SCROLLBAR_BACKGROUND	0
/// Scroll bar selected background
#define CSPAL_SCROLLBAR_SELBACKGROUND	1
/// 3D border dark
#define CSPAL_SCROLLBAR_DARK3D		2
/// 3D border light
#define CSPAL_SCROLLBAR_LIGHT3D		3

#endif // __CSWSPAL_H__
