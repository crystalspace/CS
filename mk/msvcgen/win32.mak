#==============================================================================
#
#    Automatic MSVC-compliant DSW and DSP generation component
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
#------------------------------------------------------------------------------
# win32.mak
#
#	This makfile component extends the value of MSVC.DSP with extra
#	targets which are specific to Windows or which are not otherwise
#	represented by stand-alone makefiles within the project hierarchy.
#
# *NOTE* AUGMENTED
#	The macro MSVC.DSP.AUGMENTED generates a type-augmented project list.
#	For example, (CSGEOM CSFONT VFS) becomes (library.CSGEOM plugin.CSFONT
#	plugin.VFS).  Later, the DSP.PROJECT.DEPEND variables filter the list
#	by type and strip off the type-prefix in order to arrive at a
#	bare-word dependency list.  For instance, for the pseudo-project
#	"grpplugins", the list is filtered to (plugin.CSFONT plugin.VFS), and
#	then transformed into (CSFONT VFS), which is exactly the list of
#	bare-word project names upon which "grpplugins" should depend.
#
# *NOTE* CSSYS
#	We use a special name, WIN32SYS, for the "cssys" library, rather than
#	the expected name, CSSYS.  This is necessary for DSP generation since
#	the cssys library must be composed of Windows-specific resources.
#	Therefore, we can not use the standard SRC.CSSYS and INC.CSSYS
#	variables (see CS/libs/cssys/cssys.mak), since they may reference
#	inappropriate resources.  (Specifically, they may refer to resources
#	for the platform, such as Unix, which is performing the DSW and DSP
#	generation task.)  During the project file generation step, the name
#	WIN32SYS is automatically substituted for CSSYS in the list of
#	dependencies mentioned by a module's DEP.PROJECT variable.  Thus, any
#	module which actually depends upon CSSYS is made to appear as though
#	it really depends upon WIN32SYS.
#------------------------------------------------------------------------------

# Macro to generate a type-augmented project list. (*NOTE* AUGMENTED)
MSVC.DSP.AUGMENTED = $(foreach d,$(MSVC.DSP),$(DSP.$d.TYPE).$d)

# libcssys -- system-dependent library for Windows. (*NOTE* CSSYS)
MSVC.DSP += WIN32SYS
DSP.WIN32SYS.NAME = cssys
DSP.WIN32SYS.TYPE = library
DSP.WIN32SYS.RESOURCES = \
  $(wildcard libs/cssys/*.cpp libs/cssys/win32/*.cpp) \
  include/cssys/win32/csosdefs.h \
  include/cssys/win32/volatile.h \
  include/cssys/win32/win32.h \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp

# grpall -- represents all other projects indirectly through grpapps,
# grpplugins, and grplibs.
MSVC.DSP += ALL
DSP.ALL.NAME = all
DSP.ALL.TYPE = group
DSP.ALL.DEPEND = APPS PLUGINS LIBS

# grpapps -- represents all GUI and console application projects.
MSVC.DSP += APPS
DSP.APPS.NAME = apps
DSP.APPS.TYPE = group
DSP.APPS.DEPEND = $(patsubst appgui.%,%,$(patsubst appcon.%,%,\
  $(filter appgui.% appcon.%,$(MSVC.DSP.AUGMENTED))))

# grpplugins -- represents all plug-in projects.
MSVC.DSP += PLUGINS
DSP.PLUGINS.NAME = plugins
DSP.PLUGINS.TYPE = group
DSP.PLUGINS.DEPEND = \
  $(patsubst plugin.%,%,$(filter plugin.%,$(MSVC.DSP.AUGMENTED)))

# grplibs -- represents all static library projects.
MSVC.DSP += LIBS
DSP.LIBS.NAME = libs
DSP.LIBS.TYPE = group
DSP.LIBS.DEPEND = \
  $(patsubst library.%,%,$(filter library.%,$(MSVC.DSP.AUGMENTED))))
