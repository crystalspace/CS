/*
    Linker with additional features for OS/2
    Copyright (C) 1999 by Andrew Zabolotny

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

/*
    Usage is pretty simple: use os2dll instead of GCC when building
    the library giving absolutely same parameters which you would pass
    to GCC; you can pass additional parameters which will be stripped
    from GCC command line:

      LD=...		- Linker (default value: gcc)
      DESCRIPTION="..."	- Description for dynamic-linked library
      OUT=...		- Output directory for temporary files
      CONSOLE		- to build console executables
*/

  call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
  call SysLoadFuncs

  parse arg args;

  linker = "gcc";
  linker_flags = "";
  linker_def = "$temp$.def";
  dir_out = "";
  mod_name = "";
  mod_desc = "";
  do_dll = 0;
  do_console = 0;

  max_i = words(args);
  do i = 1 to max_i
    a = word(args,i);
    select
      when left(a, 3) = "LD=" then
        linker = substr(a, 4);
      when left(a, 4) = "OUT=" then
        dir_out = substr(a, 5);
      when left(a, 7) = "CONSOLE" then
        do_console = 1;
      when ((left(a, 13) = 'DESCRIPTION="') | (left(a, 13) = '"DESCRIPTION=')) then
      do
        mod_desc = substr(a, 14);
        do forever
          if (right(a,1) = '"') then
            leave;
          i = i + 1;
          if (i > max_i) then
          do
            say "OS2LINK.CMD: Invalid DESCRIPTION option";
            exit 1;
          end
          a = word(args, i);
          mod_desc = mod_desc" "a;
        end
        mod_desc='"'left(mod_desc, length(mod_desc)-1)'"';
      end
      when a = "-o" then
      do
        linker_flags = linker_flags" "a;
        i = i + 1;
        a = word(args, i);
        if (translate(right(a, 3)) = "DLL") then
          do_dll = 1;
        linker_flags = linker_flags" "a;
        linker_def = left(a, lastpos('.', a))"def";
        mod_name = filespec('N', a);
        mod_name = left(mod_name, lastpos('.', mod_name) - 1);
      end
      otherwise
        linker_flags = linker_flags" "a;
    end;
  end;

  if (dir_out \= "") then
    linker_def = dir_out||filespec('N', linker_def);

  call SysFileDelete linker_def
  if (do_dll) then
  do
    if (mod_name = "") then
    do
      say "OS2LINK.CMD: No -o option given: don't know library name"
      exit 1;
    end

    call lineout linker_def,"LIBRARY "mod_name" INITINSTANCE TERMINSTANCE";
    if (mod_desc \= "") then
      call lineout linker_def,"DESCRIPTION "mod_desc;
    call lineout linker_def,"EXPORTS";
    call lineout linker_def,"	DllInitialize";
    call lineout linker_def,"	DllCanUnloadNow";
    call lineout linker_def,"	DllGetClassObject";
    call lineout linker_def,"	DllRegisterServer";
    call lineout linker_def,"	DllUnregisterServer";
  end
  else
  do
    if (do_console) then
      call lineout linker_def,"NAME "mod_name" WINDOWCOMPAT"
    else
      call lineout linker_def,"NAME "mod_name" WINDOWAPI"
    if (mod_desc \= "") then
      call lineout linker_def,"DESCRIPTION "mod_desc
    call lineout linker_def,"STACKSIZE 1048576"
  end;
  call stream linker_def, "C", "Close";

  call doCommand(linker' 'linker_flags' 'linker_def);

  exit 0;

doCommand:
  parse arg _cmd_;
  say _cmd_;
  '@'_cmd_;
  if (rc \= 0) then
  do
    say 'command failed, exit code='rc;
    exit 2;
  end;
return;
