//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTMenu.m
//
//	Generate a menu from a flat configuration file definition.
//
//-----------------------------------------------------------------------------
#include "NeXTMenu.h"
#include "NeXTConfigFile.h"
#include <string.h>
#import <appkit/Application.h>
#import <appkit/Menu.h>
#import <appkit/MenuCell.h>

static Menu* build_menu(char const* section, NeXTConfigHandle);

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {
#define STR_CASE(X) } else if (strcmp(switched_str__,(#X)) == 0) {
#define STR_CASE_PREFIX(X) \
  } else if (strncmp(switched_str__,(#X),sizeof(#X)-1) == 0) {
#define STR_SWITCH_END }}

//-----------------------------------------------------------------------------
// str_append
//-----------------------------------------------------------------------------
static char* str_append(char const* s1, char const* s2, char const* s3)
{
  char* p = (char*)malloc(strlen(s1) + strlen(s2) + (s3 ? strlen(s3):0) + 1);
  strcpy(p, s1);
  strcat(p, s2);
  if (s3 != 0)
    strcat(p, s3);
  return p;
}


//-----------------------------------------------------------------------------
// item_scan
//	Looks up the named keys in the configuration file.  Pays particular
//	attention to keys "title", "shortcut", "action", and "target" which are
//	used by menu_add_item().  Also pays attention to the "inherit" key
//	which allows an item to inherit attributes from another item.
//	Attributes specified in the local item override those specified in the
//	inherited item.  Items may be inherited recursively to any depth.
//-----------------------------------------------------------------------------
static void item_scan(NeXTConfigHandle config, char const* section,
  char const** title, char const** shortcut, char const** action,
  char const** target)
{
  char* k_inherit  = str_append(section, "inherit",  0);
  char* k_title    = str_append(section, "title",    0);
  char* k_shortcut = str_append(section, "shortcut", 0);
  char* k_action   = str_append(section, "action",   0);
  char* k_target   = str_append(section, "target",   0);
  
  char const* inherit = NeXTConfigFile_lookup(config, k_inherit, 0);
  if (inherit != 0)
  {
    char* parent = str_append("Item.", inherit, ".");
    item_scan(config, parent, title, shortcut, action, target);
    free(parent);
  }
  
  *title    = NeXTConfigFile_lookup(config, k_title,    *title   );
  *shortcut = NeXTConfigFile_lookup(config, k_shortcut, *shortcut);
  *action   = NeXTConfigFile_lookup(config, k_action,   *action  );
  *target   = NeXTConfigFile_lookup(config, k_target,   *target  );
  
  free(k_target  );
  free(k_action  );
  free(k_shortcut);
  free(k_title   );
  free(k_inherit );
}


//-----------------------------------------------------------------------------
// menu_add_item
//	Looks up the named keys in the configuration file using item_scan().
//	Pays particular attention to keys "title", "shortcut", "action", and
//	"target".  Adds an item to the menu based upon the attributes of these
//	keys.  "title", "shortcut", and "action" are used to generate the new
//	menu item.  "target", if present, may be either "application" which
//	stands for NXApp, or "delegate" which stands for [NXApp delegate].  If
//	"target" is not specified then the item's action is sent to the
//	first-responder.
//-----------------------------------------------------------------------------
static void menu_add_item(Menu* menu, char const* key, NeXTConfigHandle config)
{
  SEL cmd = 0;
  MenuCell* item;
  
  char const* title    = "";
  char const* shortcut = "";
  char const* action   = 0;
  char const* target   = 0;
  
  char* section = str_append("Item.", key, ".");
  item_scan(config, section, &title, &shortcut, &action, &target);
  free(section);
  
  if (action != 0)
    cmd = sel_getUid(action);
  
  item = [menu addItem:title action:cmd keyEquivalent:shortcut[0]];
  
  if (target != 0)
  {
    STR_SWITCH (target)
      STR_CASE (application)
	[item setTarget:NXApp];
      STR_CASE (delegate)
	[item setTarget:[NXApp delegate]];
    STR_SWITCH_END
  }
}


//-----------------------------------------------------------------------------
// menu_add_submenu
//	Looks up the named keys in the configuration file.  Recursively calls
//	build_menu() to generate the submenu.  Pays particular attention to key
//	"type" which, if present, may be either "window", or "services".  A
//	"type" qualification means that the menu should be configured as the
//	Application's Windows, or Services menu, respectively.
//-----------------------------------------------------------------------------
static void menu_add_submenu(Menu* menu, char const* name,
  NeXTConfigHandle config)
{
  Menu* const sub = build_menu(name, config);
  if (sub != 0)
  {
    char* key = str_append("Menu.", name, ".type");
    char const* type = NeXTConfigFile_lookup(config, key, 0);
  
    MenuCell* item = [menu addItem:[sub title] action:0 keyEquivalent:0];
    [menu setSubmenu:sub forItem:item];
  
    if (type != 0)
    {
      STR_SWITCH (type)
	STR_CASE (window)
	  [NXApp setWindowsMenu:sub];
	STR_CASE (services)
	  [NXApp setServicesMenu:sub];
      STR_SWITCH_END
    }
    free(key);
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
static void menu_add(Menu* menu, char const* key, char const* value,
  NeXTConfigHandle config)
{
  STR_SWITCH (key)
    STR_CASE_PREFIX (menu)		// *NOTE*
      menu_add_submenu(menu, value, config);
    STR_CASE_PREFIX (item)
      menu_add_item(menu, value, config);
  STR_SWITCH_END
}


//-----------------------------------------------------------------------------
// build_menu
//	Looks up the named keys in the configuration file.  Enumerates over
//	each entry in the section to build the menu.  Pays particular attention
//	to key "title" which is used to set the menu's title.
//-----------------------------------------------------------------------------
static Menu* build_menu(char const* key, NeXTConfigHandle config)
{
  Menu* m = 0;
  char* section = str_append("Menu.", key, ".");
  if (NeXTConfigFile_exists(config, section))
  {
    NeXTConfigIterator iterator =
      NeXTConfigFile_new_iterator(config, section);
    char* k_title = str_append(section, "title", 0);
    char const* title = NeXTConfigFile_lookup(config, k_title, "");
    free(k_title);
    m = [[Menu alloc] initTitle:title];
    while (NeXTConfigFile_iterator_next(iterator))
      menu_add(m, NeXTConfigFile_iterator_key(iterator),
	NeXTConfigFile_iterator_data(iterator), config);
    NeXTConfigFile_dispose_iterator(iterator);
  }
  free(section);
  return m;
}


//-----------------------------------------------------------------------------
// NeXTMenuGenerate
//	Generate a menu from the configuration information in next.cfg.
//-----------------------------------------------------------------------------
Menu* NeXTMenuGenerate(char const* menu_ident, NeXTConfigHandle config)
{
  return build_menu(menu_ident, config);
}

#undef STR_SWITCH_END
#undef STR_CASE_PREFIX
#undef STR_CASE
#undef STR_SWITCH
