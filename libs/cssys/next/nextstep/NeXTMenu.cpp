//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTMenu.cpp
//
//	Generate a menu from an INI file definition.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTMenu.h"
#include "csutil/inifile.h"
extern "Objective-C" {
#import <appkit/Application.h>
#import <appkit/Menu.h>
#import <appkit/MenuCell.h>
}

static Menu* build_menu( char const* section, iConfigFile const* );

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {}
#define STR_CASE(X) else if (strcmp(switched_str__,(#X)) == 0)
#define STR_SWITCH_END }

//-----------------------------------------------------------------------------
// str_append
//-----------------------------------------------------------------------------
static char* str_append( char const* prefix, char const* suffix )
    {
    char* p = (char*)malloc( strlen(prefix) + strlen(suffix) + 1 );
    strcpy( p, prefix );
    strcat( p, suffix );
    return p;
    }


//-----------------------------------------------------------------------------
// menu_add_item
//	Looks up the named section in the INI file.  Pays particular attention
//	to keys "title", "shortcut", "action", & "target".  Adds a menu item to
//	the menu based upon the attributes of these keys.  "title", "shortcut",
//	& "action" are used to generate the new menu item.  "target", if
//	present, may be either "application" which stands for NXApp, or
//	"delegate" which stands for [NXApp delegate].  If "target" is not
//	specified then the item's action is sent to the first-responder.
//-----------------------------------------------------------------------------
static void menu_add_item( Menu* menu, char const* key,
    iConfigFile const* config )
    {
    char* section = str_append( "Item.", key );

    char const* title    = config->GetStr( section, "title",    "" );
    char const* shortcut = config->GetStr( section, "shortcut", "" );
    char const* action   = config->GetStr( section, "action",   0  );

    SEL cmd = 0;
    if (action != 0)
	cmd = sel_getUid( action );

    MenuCell* const item = [menu addItem:title action:cmd
	keyEquivalent:shortcut[0]];

    char const* target = config->GetStr( section, "target", 0 );
    if (target != 0)
	{
	STR_SWITCH (target)
	    STR_CASE (application)
		[item setTarget:NXApp];
	    STR_CASE (delegate)
		[item setTarget:[NXApp delegate]];
	STR_SWITCH_END
	}

    free( section );
    }


//-----------------------------------------------------------------------------
// menu_add_submenu
//	Looks up the named section in the INI file.  Recursively calls
//	build_menu() to generate the submenu.  Pays particular attention to
//	key "type" which, if present, may be either "window", or "services".
//	A "type" qualification means that the menu should be configured as the
//	Application's Windows, or Services menu, respectively.
//-----------------------------------------------------------------------------
static void menu_add_submenu( Menu* menu, char const* key,
    iConfigFile const* config )
    {
    char* section = str_append( "Menu.", key );
    Menu* const sub = build_menu( section, config );
    if (sub != 0)
	{
	MenuCell* item = [menu addItem:[sub title] action:0 keyEquivalent:0];
	[menu setSubmenu:sub forItem:item];

	char const* type = config->GetStr( section, "type", 0 );
	if (type != 0)
	    {
	    STR_SWITCH (type)
		STR_CASE (window)
		    [NXApp setWindowsMenu:sub];
		STR_CASE (services)
		    [NXApp setServicesMenu:sub];
	    STR_SWITCH_END
	    }
	}
    free( section );
    }


//-----------------------------------------------------------------------------
// menu_add
//	Called for each entry in a menu section.  Pays particular attention to
//	keys "menu" & "item".  For each "menu", adds a new submenu to the
//	parent menu.  For each "item", adds a new menu item.
//-----------------------------------------------------------------------------
static void menu_add( Menu* menu, char const* key, char const* value,
    iConfigFile const* config )
    {
    STR_SWITCH (key)
	STR_CASE (menu)
	    menu_add_submenu( menu, value, config );
	STR_CASE (item)
	    menu_add_item( menu, value, config );
    STR_SWITCH_END
    }


//-----------------------------------------------------------------------------
// build_menu
//	Looks up the named section in the INI file.  Enumerates over each entry
//	in the section to build the menu.  Pays particular attention to key
//	"title" which is used to set the menu's title.
//-----------------------------------------------------------------------------
static Menu* build_menu( char const* key, iConfigFile const* config )
    {
    Menu* m = 0;
    char* section = str_append( "Menu.", key );
    if (config->SectionExists( section ))
	{
	char const* title = config->GetStr( section, "title", "" );
	m = [[Menu alloc] initTitle:title];
	iConfigDataIterator* iterator = config.EnumData(section);
	while (iterator->Next())
	    menu_add( m, iterator->GetKey(),
		(const char*)iterator->GetData(), config);
	iterator->DecRef();
	}
    free( section );
    return m;
    }


//-----------------------------------------------------------------------------
// NeXTMenuGenerate
//	Generate a menu from the configuration information in NeXTMenu.cfg.
//-----------------------------------------------------------------------------
Menu* NeXTMenuGenerate( char const* menu_ident, iConfigFile const* config )
    {
    return build_menu( menu_ident, config );
    }

#undef STR_SWITCH_END
#undef STR_CASE
#undef STR_SWITCH
