/*
    Copyright (C) 1999 by Brandon Ehle <azverkan@yahoo.com>

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

extern "C" {
#include <lua.h>
#include <lualib.h>
//#include <luadebug.h>
}
#include "cssysdef.h"
#include "cslua.h"
#include "csutil/csstring.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csLua)
  SCF_IMPLEMENTS_INTERFACE(iScript)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLua::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csLua)

SCF_EXPORT_CLASS_TABLE(cslua)
  SCF_EXPORT_CLASS(csLua, "crystalspace.script.lua",
    "Crystal Space Script Lua")
SCF_EXPORT_CLASS_TABLE_END

csLua* csLua::shared_instance = NULL;

csLua::csLua(iBase *iParent) :Sys(NULL), Mode(CS_REPORTER_SEVERITY_NOTIFY),
	lua_state(NULL)
{
  SCF_CONSTRUCT_IBASE(iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  shared_instance = this;
}

#define LUA_STATE() ((lua_State*)lua_state)

csLua::~csLua()
{
  Mode=CS_REPORTER_SEVERITY_BUG;
  lua_close(LUA_STATE());
  lua_state=NULL;
  object_reg=NULL;
}

extern "C" {
  int cspace_initialize(lua_State *L);
}
extern int iObjectRegistry_tag;
bool csLua::Initialize(iObjectRegistry* object_reg)
{
  csLua::object_reg=object_reg;

  lua_state = lua_open(0); //Stacksize is 0, is there a better value?

//Userinit start
  lua_baselibopen(LUA_STATE());
  lua_iolibopen(LUA_STATE());
  lua_strlibopen(LUA_STATE());
  lua_mathlibopen(LUA_STATE());
  lua_dblibopen(LUA_STATE());
//Userinit end

  cspace_initialize(LUA_STATE());

  Mode=CS_REPORTER_SEVERITY_NOTIFY;

  // Store the system pointer in 'cspace.system'.
  lua_pushusertag(LUA_STATE(), (void*)object_reg, iObjectRegistry_tag);
  lua_setglobal(LUA_STATE(), "object_reg");

  return true;
}

void csLua::ShowError()
{
//Write me
}

bool csLua::RunText(const char* Text)
{
  int top = lua_gettop(LUA_STATE());
  int res = lua_dostring(LUA_STATE(), Text);  /* dostring | dofile */
  lua_settop(LUA_STATE(), top);  /* remove eventual results */

  if (res == LUA_ERRMEM) {
    Print(1, "lua: memory allocation error");
    return 0;
  } else if (res == LUA_ERRERR) {
    Print(1, "lua: error in error message");
    return 0;
  }

  return 1;
}

bool csLua::Store(const char* type, const char* name, void* data)
{
//Write me
  return 0;
}

bool csLua::LoadModule(const char* name)
{
  csString s;
  s << "dofile('" << name << "')";
  return RunText(s);
}

void csLua::Print(bool Error, const char *msg)
{
  iObjectRegistry* object_reg = Sys->GetObjectRegistry ();
  iReporter* reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (!reporter)
    csPrintf ("%s\n", msg);
  else
  {
    if(Error)
      reporter->Report (CS_REPORTER_SEVERITY_ERROR, "crystalspace.script.lua",
      	"CrystalScript Error: %s", msg);
    else
      reporter->Report (Mode, "crystalspace.script.lua",
      	"%s", msg);
  }
}

extern "C" {
  extern void swig_lua_init(lua_State *L) {
    //BNE We are already initialized by this time so this is a no-op
  }
}
