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
#include "icfgfile.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
}

static NSMenu* build_menu( char const* section, iConfigFile const* );

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {}
#define STR_CASE(X) else if (strcmp(switched_str__,(#X)) == 0)
#define STR_SWITCH_END }

//-----------------------------------------------------------------------------
// OpenStep 4.2 --> MacOS/X Server compatibility
//-----------------------------------------------------------------------------
#if defined(OS_NEXT_MACOSXS)

static void menu_add_separator( NSMenu* m )
    { [m addItem:[NSMenuItem separatorItem]]; }

#else // !OS_NEXT_MACOSXS

static void menu_add_separator( NSMenu* m )
    { [[m addItemWithTitle:@"-" action:0 keyEquivalent:@""] setEnabled:NO]; }

@interface NSApplication (NeXTMenu)
- (void)setAppleMenu:(NSMenu*)m;
@end
@implementation NSApplication (NeXTMenu)
- (void)setAppleMenu:(NSMenu*)m {}
@end

#endif // OS_NEXT_MACOSXS


//-----------------------------------------------------------------------------
// menu_add_item
//	Looks up the named section in the INI file.  Pays particular attention
//	to keys "type", "title", "shortcut", "action", & "target".  Adds a menu
//	item to the menu based upon the attributes of these keys.  "type", if
//	present, may specify "separator" in which case the menu item is a
//	"separator".  Otherwise, "title", "shortcut", & "action" are used to
//	generate the new menu item.  "target", if present, may be either
//	"application" which stands for NSApp, or "delegate" which stands for
//	[NSApp delegate].  If "target" is not specified then the item's action
//	is sent to the first-responder.
//-----------------------------------------------------------------------------
static void menu_add_item( NSMenu* menu, char const* key,
    iConfigFile const* config )
    {
    char const* section = [[NSString stringWithFormat:@"Item.%s",key] cString];

    char const* type = config->GetStr( section, "type", 0 );
    if (type != 0 && strcmp( type, "separator" ) == 0)
	menu_add_separator( menu );
    else
	{
	char const* title    = config->GetStr( section, "title",    "" );
	char const* shortcut = config->GetStr( section, "shortcut", "" );
	char const* action   = config->GetStr( section, "action",   0  );

	SEL cmd = 0;
	if (action != 0)
	    cmd = NSSelectorFromString([NSString stringWithCString:action]);

	NSMenuItem* const item = [menu addItemWithTitle:
		[NSString stringWithCString:title] action:cmd
		keyEquivalent:[NSString stringWithCString:shortcut]];
	[item setKeyEquivalentModifierMask:NSCommandKeyMask];

	char const* target = config->GetStr( section, "target", 0 );
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
//	Looks up the named section in the INI file.  Recursively calls
//	build_menu() to generate the submenu.  Pays particular attention to
//	key "type" which, if present, may be one of "apple", "window", or
//	"services".  A TYPE qualification means that the menu should be
//	configured as the NSApplication's Apple, Window, or Services menu,
//	respectively.
//-----------------------------------------------------------------------------
static void menu_add_submenu( NSMenu* menu, char const* key,
    iConfigFile const* config )
    {
    NSMenu* const sub = build_menu( key, config );
    if (sub != 0)
	{
	NSMenuItem* const item =
		[menu addItemWithTitle:[sub title] action:0 keyEquivalent:@""];
	[menu setSubmenu:sub forItem:item];

	char const* section =
	    [[NSString stringWithFormat:@"Menu.%s",key] cString];
	char const* type = config->GetStr( section, "type", 0 );
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
// menu_add
//	Called for each entry in a menu section.  Pays particular attention to
//	keys "menu" & "item".  For each "menu", adds a new submenu to the
//	parent menu.  For each "item", adds a new menu item.
//-----------------------------------------------------------------------------
static void menu_add( NSMenu* menu, char const* key, char const* value,
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
static NSMenu* build_menu( char const* key, iConfigFile const* config )
    {
    NSMenu* m = 0;
    char const* section = [[NSString stringWithFormat:@"Menu.%s",key] cString];
    if (config->SectionExists( section ))
	{
	char const* title = config->GetStr( section, "title", "" );
	m = [[NSMenu alloc] initWithTitle:[NSString stringWithCString:title]];
	iConfigDataIterator* iterator = config->EnumData( section );
	while (iterator->Next())
	    menu_add( m, iterator->GetKey(),
		(char const*)iterator->GetData(), config);
	iterator->DecRef();
	}
    return m;
    }


//-----------------------------------------------------------------------------
// NeXTMenuGenerate
//	Generate a menu from the configuration information in NeXTMenu.cfg.
//-----------------------------------------------------------------------------
NSMenu* NeXTMenuGenerate( char const* menu_ident, iConfigFile const* config )
    {
    return build_menu( menu_ident, config );
    }

#undef STR_SWITCH_END
#undef STR_CASE
#undef STR_SWITCH
