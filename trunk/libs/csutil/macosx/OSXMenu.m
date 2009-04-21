//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXMenu.m
//
//	Generate a menu from a flat configuration file definition.
//
//-----------------------------------------------------------------------------
#include "OSXMenu.h"
#include "OSXConfigFile.h"
#include <string.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

static NSMenu* build_menu(char const* section, OSXConfigHandle, id);

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {
#define STR_CASE(X) } else if (strcmp(switched_str__,(#X)) == 0) {
#define STR_CASE_PREFIX(X) \
  } else if (strncmp(switched_str__,(#X),sizeof(#X)-1) == 0) {
#define STR_SWITCH_END }}

#define STR_APPEND(S1,S2) [[NSString stringWithFormat:@"%s%s", S1, S2] cString]
#define STR_APPENDD(S1,S2,S3) \
  [[NSString stringWithFormat:@"%s%s%s", S1, S2, S3] cString]


//-----------------------------------------------------------------------------
// item_scan
//	Looks up the named keys in the configuration file.  Pays particular
//	attention to keys "type", "title", "shortcut", "action", and "target"
//	which are used by menu_add_item().  Also pays attention to the
//	"inherit" key which allows an item to inherit attributes from another
//	item.  Attributes specified in the local item override those specified
//	in the inherited item.  Items may be inherited recursively to any
//	depth.
//-----------------------------------------------------------------------------
static void item_scan(OSXConfigHandle config, char const* section,
  char const** type, char const** title, char const** shortcut,
  char const** action, char const** target)
{
  char const* k_inherit  = STR_APPEND(section, "inherit" );
  char const* k_type     = STR_APPEND(section, "type"    );
  char const* k_title    = STR_APPEND(section, "title"   );
  char const* k_shortcut = STR_APPEND(section, "shortcut");
  char const* k_action   = STR_APPEND(section, "action"  );
  char const* k_target   = STR_APPEND(section, "target"  );

  char const* inherit = OSXConfigFile_lookup(config, k_inherit, 0);
  if (inherit != 0)
  {
    char const* parent = STR_APPENDD("OSX.Item.", inherit, ".");
    item_scan(config, parent, type, title, shortcut, action, target);
  }

  *type     = OSXConfigFile_lookup(config, k_type,     *type    );
  *title    = OSXConfigFile_lookup(config, k_title,    *title   );
  *shortcut = OSXConfigFile_lookup(config, k_shortcut, *shortcut);
  *action   = OSXConfigFile_lookup(config, k_action,   *action  );
  *target   = OSXConfigFile_lookup(config, k_target,   *target  );
}


//-----------------------------------------------------------------------------
// menu_add_item
//	Looks up the named keys in the configuration file using item_scan().
//	Pays particular attention to keys "type", "title", "shortcut",
//	"action", and "target".  Adds an item to the menu based upon the
//	attributes of these keys.  "type", if present, may specify "separator"
//	in which case the menu item is a separator line.  Otherwise, "title",
//	"shortcut", and "action" are used to generate the new menu item.
//	"target", if present, may be "application" which stands for NSApp;
//	"delegate" which stands for [NSApp delegate]; or "assistant" which
//	stands for the internal Crystal Space OSXDelegate instance owned by
//	iOSXAssistant.  If "target" is not specified then the item's action is
//	sent to the first-responder.
//-----------------------------------------------------------------------------
static void menu_add_item(NSMenu* menu, char const* key,
  OSXConfigHandle config, id assistant)
{
  char const* type     = 0;
  char const* title    = "";
  char const* shortcut = "";
  char const* action   = 0;
  char const* target   = 0;

  char const* section = STR_APPENDD("OSX.Item.", key, ".");
  item_scan(config, section, &type, &title, &shortcut, &action, &target);

  if (type != 0 && strcmp(type, "separator") == 0)
    [menu addItem:[NSMenuItem separatorItem]];
  else
  {
    SEL cmd = 0;
    id item;

    if (action != 0)
      cmd = NSSelectorFromString([NSString stringWithCString:action]);

    item = [menu addItemWithTitle:[NSString stringWithCString:title] action:cmd
      keyEquivalent:[NSString stringWithCString:shortcut]];
    [item setKeyEquivalentModifierMask:NSCommandKeyMask];

    if (target != 0)
    {
      STR_SWITCH (target)
	STR_CASE (application)
	  [item setTarget:NSApp];
	STR_CASE (assistant)
	  [item setTarget:assistant];
	STR_CASE (delegate)
	  [item setTarget:[NSApp delegate]];
      STR_SWITCH_END
    }
  }
}


//-----------------------------------------------------------------------------
// menu_add_submenu
//	Looks up the named keys in the configuration file.  Recursively calls
//	build_menu() to generate the submenu.  Pays particular attention to key
//	"type" which, if present, may be "window" or "services".  A "type"
//	qualification means that the menu should be configured as the
//	NSApplication's Window, or Services menu, respectively.
//-----------------------------------------------------------------------------
static void menu_add_submenu(NSMenu* menu, char const* name,
  OSXConfigHandle config, id assistant)
{
  NSMenu* const sub = build_menu(name, config, assistant);
  if (sub != 0)
  {
    char const* key = STR_APPENDD("OSX.Menu.", name, ".type");
    char const* type = OSXConfigFile_lookup(config, key, 0);

    NSMenuItem* const item =
      [menu addItemWithTitle:[sub title] action:0 keyEquivalent:@""];
    [menu setSubmenu:sub forItem:item];

    if (type != 0)
    {
      STR_SWITCH (type)
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
//	keys "menu" and "item".  For each "menu", adds a new submenu to the
//	parent menu.  For each "item", adds a new menu item.
// *NOTE*
//	The configuration file class requires unique key names.  However, since
//	"menu" and "item" directives can appear multiple times in a single menu
//	block, an artificial mechanism of appending a throw-away suffix to the
//	key is used to enforce uniqueness.  Since this suffix has no particular
//	meaning, we ignore it by using STR_CASE_PREFIX instead of STR_CASE.
//-----------------------------------------------------------------------------
static void menu_add(NSMenu* menu, char const* key, char const* value,
  OSXConfigHandle config, id assistant)
{
  STR_SWITCH (key)
    STR_CASE_PREFIX (menu)		// *NOTE*
      menu_add_submenu(menu, value, config, assistant);
    STR_CASE_PREFIX (item)
      menu_add_item(menu, value, config, assistant);
  STR_SWITCH_END
}


//-----------------------------------------------------------------------------
// build_menu
//	Looks up the named keys in the configuration file.  Enumerates over
//	each entry in the section to build the menu.  Pays particular attention
//	to key "title" which is used to set the menu's title.
//-----------------------------------------------------------------------------
static NSMenu* build_menu(
  char const* key, OSXConfigHandle config, id assistant)
{
  NSMenu* m = 0;
  char const* section = STR_APPENDD("OSX.Menu.", key, ".");
  if (OSXConfigFile_exists(config, section))
  {
    OSXConfigIterator iterator = OSXConfigFile_new_iterator(config, section);
    char const* k_title = STR_APPEND(section, "title");
    char const* title = OSXConfigFile_lookup(config, k_title, "");
    m = [[NSMenu alloc] initWithTitle:[NSString stringWithCString:title]];
    while (OSXConfigFile_iterator_next(iterator))
      menu_add(m, OSXConfigFile_iterator_key(iterator),
	OSXConfigFile_iterator_data(iterator), config, assistant);
    OSXConfigFile_dispose_iterator(iterator);
  }
  return m;
}


//-----------------------------------------------------------------------------
// OSXMenuGenerate
//	Generate a menu from the configuration information in macosx.cfg.
//-----------------------------------------------------------------------------
NSMenu* OSXMenuGenerate(
  id assistant, char const* menu_ident, OSXConfigHandle config)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSMenu* menu = build_menu(menu_ident, config, assistant);
  [pool release];
  return menu;
}

#undef STR_APPENDD
#undef STR_APPEND
#undef STR_SWITCH_END
#undef STR_CASE_PREFIX
#undef STR_CASE
#undef STR_SWITCH
