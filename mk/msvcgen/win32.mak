#==============================================================================
#
#    Automatic MSVC-compliant workspace and project generation component
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
#	We override the INC.CSSYS and SRC.CSSYS variables from the
#	platform-specific makefile since, for project file generation, the
#	Windows-specific resources must be used.  We can not rely upon the
#	default SRC.CSSYS and INC.CSSYS definitions (see
#	CS/libs/cssys/cssys.mak) since they may reference inappropriate
#	resources if the project is configured for a non-Windows platform.  For
#	example, it is possible to generate the project files from Unix.
#------------------------------------------------------------------------------

# Macro to generate a type-augmented project list. (*NOTE* AUGMENTED)
MSVC.DSP.AUGMENTED = $(foreach d,$(MSVC.DSP),$(DSP.$d.TYPE).$d)

# Platform-specific implementation for Windows. (*NOTE* CSSYS)
ifeq ($(DO_MSVCGEN),yes)
INC.CSSYS = \
  $(wildcard $(SRCDIR)/include/cssys/win32/*.h) \
  $(wildcard $(SRCDIR)/libs/cssys/win32/*.h)
SRC.CSSYS = \
  $(wildcard $(SRCDIR)/libs/cssys/*.cpp $(SRCDIR)/libs/cssys/win32/*.cpp) \
  $(SRCDIR)/libs/cssys/general/csprocessorcap.cpp \
  $(SRCDIR)/libs/cssys/general/findlib.cpp \
  $(SRCDIR)/libs/cssys/general/getopt.cpp \
  $(SRCDIR)/libs/cssys/general/pluginpaths.cpp \
  $(SRCDIR)/libs/cssys/general/resdir.cpp \
  $(SRCDIR)/libs/cssys/general/runloop.cpp
endif

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
