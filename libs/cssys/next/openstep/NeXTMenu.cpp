//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
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
//	Generate a menu from the INI style configuration file NeXTMenu.cfg.
//
//-----------------------------------------------------------------------------
#import "NeXTMenu.h"
#import "util/inifile.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
}

static NSMenu* build_menu( char const* section, csIniFile const& );

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {}
#define STR_CASE(X) else if (strcmp(switched_str__,(#X)) == 0)
#define STR_SWITCH_END }

struct NeXTParamData
    {
    csIniFile const& config;
    void* data;
    NeXTParamData( csIniFile const& c, void* d ) : config(c), data(d) {}
    };

//-----------------------------------------------------------------------------
// Import the NeXTMenu.cfg file which defines the menu for this platform.
// Uses pre-processor search-path to locate correct configuration file.
//-----------------------------------------------------------------------------
static char const MENU_CONFIG[] =
#include "NeXTMenu.cfg"
;
size_t const MENU_SIZE = sizeof(MENU_CONFIG) / sizeof(MENU_CONFIG[0]);


//-----------------------------------------------------------------------------
// OpenStep 4.2 --> Rhapsody compatibility
//-----------------------------------------------------------------------------
#if defined(OS_NEXT_RHAPSODY)

static void menu_add_separator( NSMenu* m )
    { [m addItem:[NSMenuItem separatorItem]]; }

#else // !OS_NEXT_RHAPSODY

static void menu_add_separator( NSMenu* m )
    { [[m addItemWithTitle:@"-" action:0 keyEquivalent:@""] setEnabled:NO]; }

@interface NSApplication (NeXTMenu)
- (void)setAppleMenu:(NSMenu*)m;
@end
@implementation NSApplication (NeXTMenu)
- (void)setAppleMenu:(NSMenu*)m {}
@end

#endif // OS_NEXT_RHAPSODY


//-----------------------------------------------------------------------------
// menu_add_item
//	Looks up the named section in csIniFile.  Pays particular attention to
//	keys TYPE, TITLE, SHORTCUT, ACTION, & TARGET.  Adds a menu item to the
//	menu based upon the attributes of these keys.  TYPE, if present, may
//	specify "separator" in which case the menu item is a "separator".
//	Otherwise, TITLE, SHORTCUT, & ACTION are used to generate the new menu
//	item.  TARGET, if present, may be either "application" which stands for
//	NSApp, or "delegate" which stands for [NSApp delegate].  If TARGET is
//	not specified then the item's action is sent to the first-responder.
//-----------------------------------------------------------------------------
static void menu_add_item( NSMenu* menu, char const* key,
    csIniFile const& config )
    {
    char const* section = [[NSString stringWithFormat:@"Item.%s",key] cString];

    char const* type = config.GetStr( section, "TYPE", 0 );
    if (type != 0 && strcmp( type, "separator" ) == 0)
	menu_add_separator( menu );
    else
	{
	char const* title    = config.GetStr( section, "TITLE",    "" );
	char const* shortcut = config.GetStr( section, "SHORTCUT", "" );
	char const* action   = config.GetStr( section, "ACTION",   0  );

	SEL cmd = 0;
	if (action != 0)
	    cmd = NSSelectorFromString([NSString stringWithCString:action]);

	NSMenuItem* const item = [menu addItemWithTitle:
		[NSString stringWithCString:title] action:cmd
		keyEquivalent:[NSString stringWithCString:shortcut]];
	[item setKeyEquivalentModifierMask:NSCommandKeyMask];

	char const* target = config.GetStr( section, "TARGET", 0 );
	if (target != 0)
	    {
	    STR_SWITCH (target)
		STR_CASE (application)
		    [item setTarget:NSApp];
		STR_CASE (delegate)
		    [item setTarget:[NSApp delegate]];
	    STR_SWITCH_END
	    }
	}
    }


//-----------------------------------------------------------------------------
// menu_add_submenu
//	Looks up the named section in csIniFile.  Recursively calls
//	build_menu() to generate the submenu.  Pays particular attention to
//	key TYPE which, if present, may be one of "apple", "window", or
//	"services".  A TYPE qualification means that the menu should be
//	configured as the NSApplication's Apple, Window, or Services menu,
//	respectively.
//-----------------------------------------------------------------------------
static void menu_add_submenu( NSMenu* menu, char const* key,
    csIniFile const& config )
    {
    char const* section = [[NSString stringWithFormat:@"Menu.%s",key] cString];
    NSMenu* const sub = build_menu( section, config );
    if (sub != 0)
	{
	NSMenuItem* const item =
		[menu addItemWithTitle:[sub title] action:0 keyEquivalent:@""];
	[menu setSubmenu:sub forItem:item];

	char const* type = config.GetStr( section, "TYPE", 0 );
	if (type != 0)
	    {
	    STR_SWITCH (type)
		STR_CASE (apple)
		    [NSApp setAppleMenu:sub];
		STR_CASE (window)
		    [NSApp setWindowsMenu:sub];
		STR_CASE (services)
		    [NSApp setServicesMenu:sub];
	    STR_SWITCH_END
	    }
	}
    }


//-----------------------------------------------------------------------------
// menu_iterator
//	The "menu" section iterator.  Called by csIniFile for each entry in a
//	menu section.  Pays particular attention to keys MENU & ITEM.  For
//	each MENU, adds a new submenu to the parent menu.  For each ITEM, adds
//	a new menu item.
//-----------------------------------------------------------------------------
static bool menu_iterator( csSome param, char* key, size_t size, csSome data )
    {
    NeXTParamData const* param_data = (NeXTParamData const*)param;
    NSMenu* const menu = (NSMenu*)param_data->data;
    char const* value = (char const*)data;

    STR_SWITCH (key)
	STR_CASE (MENU)
	    menu_add_submenu( menu, value, param_data->config );
	STR_CASE (ITEM)
	    menu_add_item( menu, value, param_data->config );
    STR_SWITCH_END

    return false;	// false ==> continue enumeration
    }


//-----------------------------------------------------------------------------
// build_menu
//	Looks up the named section in csIniFile.  Enumerates over each entry
//	in the section to build the menu.  Pays particular attention to key
//	TITLE which is used to set the menu's title.
//-----------------------------------------------------------------------------
static NSMenu* build_menu( char const* section, csIniFile const& config )
    {
    NSMenu* m = 0;
    if (config.SectionExists( section ))
	{
	char const* title = config.GetStr( section, "TITLE", "" );
	m = [[NSMenu alloc] initWithTitle:[NSString stringWithCString:title]];
	NeXTParamData const param( config, m );
	config.EnumData(section, menu_iterator, REINTERPRET_CAST(void*)&param);
	}
    return m;
    }


//-----------------------------------------------------------------------------
// NeXTMenuGenerate
//	Generate a menu from the configuration information in NeXTMenu.cfg.
//-----------------------------------------------------------------------------
NSMenu* NeXTMenuGenerate()
    {
    NSMenu* menu = 0;
    csIniFile config;
    if (!config.Load( MENU_CONFIG, MENU_SIZE ))
	printf( "Error parsing menu configuration\n%s\n", MENU_CONFIG );
    else
	menu = build_menu( "Menu.main", config );
    return menu;
    }

#undef STR_SWITCH_END
#undef STR_CASE
#undef STR_SWITCH
