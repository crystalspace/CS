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

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE(csLua)
  IMPLEMENTS_INTERFACE(iScript)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(csLua)

EXPORT_CLASS_TABLE(cslua)
  EXPORT_CLASS(csLua, "crystalspace.script.lua",
    "Crystal Space Script Lua")
EXPORT_CLASS_TABLE_END

csLua *thisclass=NULL;

csLua::csLua(iBase *iParent) :Sys(NULL), Mode(MSG_INITIALIZATION)
{
  CONSTRUCT_IBASE(iParent);
}

lua_State *L = NULL;
csLua::~csLua()
{
  Mode=MSG_INTERNAL_ERROR;
  lua_close(L);
  Sys=NULL;
  thisclass=NULL;
}

void cspace_init (lua_State *lua_state);
bool csLua::Initialize(iSystem* iSys)
{
  Sys=iSys;
  thisclass=this;

  L = lua_open(0); //Stacksize is 0, is there a better value?

//Userinit start
  lua_baselibopen(L);
  lua_iolibopen(L);
  lua_strlibopen(L);
  lua_mathlibopen(L);
  lua_dblibopen(L);
//Userinit end

  cspace_init(L);

  Mode=MSG_STDOUT;
  return true;
}

void csLua::ShowError()
{
//Write me
}

bool csLua::RunText(const char* Text)
{
  int top = lua_gettop(L);
  int res = lua_dostring(L, Text);  /* dostring | dofile */
  lua_settop(L, top);  /* remove eventual results */

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
  if(Error)
    Sys->Printf(MSG_FATAL_ERROR, "CrystalScript Error: %s\n", msg);
  else
    Sys->Printf(Mode, "%s", msg);
}
