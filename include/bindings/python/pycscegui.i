/*
    Copyright (C) 2006 Pablo Martin <caedesv@users.sourceforge.net>

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
  SWIG interface for Crystal Space Python Cegui Plugin
  Wraps iCEGUI crystalspace module.
*/

%module pycscegui

%define GETTER_METHOD(classname,varname)
%extend classname {
 %pythoncode %{
  varname = property(_pycscegui. ## classname ## _Get ## varname ## Ptr)  %}
}
%enddef

%import "bindings/cspace.i"

%{
#include "crystalspace.h"
#include "ivaria/icegui.h"
%}

INTERFACE_PRE(iCEGUI);
%include "ivaria/icegui.h"
INTERFACE_POST(iCEGUI);

GETTER_METHOD(iCEGUI,SchemeManager)
GETTER_METHOD(iCEGUI,System)
GETTER_METHOD(iCEGUI,FontManager)
GETTER_METHOD(iCEGUI,GlobalEventSet)
GETTER_METHOD(iCEGUI,ImagesetManager)
GETTER_METHOD(iCEGUI,Logger)
GETTER_METHOD(iCEGUI,MouseCursor)
GETTER_METHOD(iCEGUI,WindowFactoryManager)
GETTER_METHOD(iCEGUI,WindowManager)

/* We need this runtime test because most swig versions are incompatible
   and will either crash or return unwrapped cegui pointers. */
%pythoncode {
SWIG_BUILD_VERSION = SWIG_VERSION
}
%pythoncode %{
msg_swig_cant_check = """
Warning: Could not check pycegui version, note you might experience
         crashes if pycegui and pycscegui were not built with compatible
         swig versions.
"""
msg_swig_incompatible = """
Warning: pycegui and pycscegui were generated with different swig versions.
         You might experience crashes because of this, or swig might not
         return wrapped cegui pointers from iCEGUI.
"""
try:
	import cegui
	if not hasattr(cegui,"SWIG_BUILD_VERSION"):
		print msg_swig_cant_check
	elif not SWIG_BUILD_VERSION == cegui.SWIG_BUILD_VERSION:
		print msg_swig_incompatible
except:
	print "Warning: You dont seem to have pycegui installed."
	print "         Please install as pycscegui needs this."
%}


