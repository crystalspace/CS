@echo off
rem ===========================================================================
rem  Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
rem
rem  A batch which creates a directory plus any missing parent directories.  It
rem  is safe to invoke this script even if the given directory already exists.
rem  Input is the directory pathname as a set of space-separated components,
rem  rather than a backslash-delimeted string.  For example, to create a
rem  directory using the relative pathname foo\bar\cow:
rem
rem	mkdirs foo bar cow
rem
rem  Absolute and relative pathnames are supported.  It is important to avoid
rem  specifying a component with a trailing slash.  To create a subdirectory
rem  tree under the root of the current drive (such as \foo\bar\cow), rather
rem  than using "\" as the first component, use "\.".  For example:
rem
rem	mkdirs \. foo bar cow
rem
rem  A drive letter may also be specified, sans the backslash.  For example:
rem
rem	mkdirs c: foo bar cow
rem
rem  Note, however, that in general it is not necessary to separate the root
rem  path componenet from the rest of the pathname since the root component can
rem  be assumed to already exist, thus the above examples can be restated as:
rem
rem	mkdirs \foo bar cow
rem	mkdirs c:\foo bar cow
rem
rem ===========================================================================

set dirpfx=
:loop
if "%1" == "" goto done
if "%dirpfx%" == "" set dirwork=%1
if not "%dirpfx%" == "" set dirwork=%dirpfx%\%1
if not exist %dirwork%\nul mkdir %dirwork%
set dirpfx=%dirwork%
shift
goto loop
:done
set dirpfx=
set dirwork=
