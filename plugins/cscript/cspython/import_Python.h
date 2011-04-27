/*
    Copyright (C) 2009 by Frank Richter

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

/* Header file to wrap inclusion of Python.h
   On MSVC, #undefined _DEBUG around inclusion of Python.h.
   Reason: When _DEBUG is set, Python.h wants to link against pythonXX_d.lib
   instead of the 'release' pythonXX.lib. However, only the latter is shipped
   with Python releases. Thus, to make it possible to build a debug build
   with a stock Python install, fake absence of _DEBUG around Python.h.
 */

#ifdef _MSC_VER
#include <io.h>
#include <stdarg.h>
#if defined(_DEBUG) && !defined(DEBUG_PYTHON)
#undef _DEBUG
#define RESTORE__DEBUG
#endif
#endif
#include <Python.h>
#ifdef RESTORE__DEBUG
#define _DEBUG
#undef RESTORE__DEBUG
#endif
