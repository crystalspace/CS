#==============================================================================
#
#    Automatic MSVC-compliant workspace and project generation component
#    Copyright (C) 2000,2004 by Eric Sunshine <sunshine@sunshineco.com>
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
# *NOTE* SYS_CSUTIL
#	We override the SRC.SYS_CSUTIL and INC.SYS_CSUTIL variables from the
#	platform-specific makefile since, for project file generation, the
#	Windows-specific resources must be used.  We can not rely upon the
#	default SRC.SYS_CSUTIL and INC.SYS_CSUTIL definitions (see
#	CS/libs/csutil/csutil.mak) since they may reference inappropriate
#	resources if the project is configured for a non-Windows platform.  For
#	example, it is possible to generate the project files from Unix.
#
#------------------------------------------------------------------------------

# Platform-specific implementation for Windows. (*NOTE* SYS_CSUTIL)
ifeq ($(DO_MSVCGEN),yes)
INC.SYS_CSUTIL = \
  $(wildcard $(SRCDIR)/include/csutil/win32/*.h) \
  $(wildcard $(SRCDIR)/libs/csutil/win32/*.h)
SRC.SYS_CSUTIL = \
  $(wildcard $(SRCDIR)/libs/csutil/win32/*.cpp) \
  $(SRCDIR)/libs/csutil/generic/appdir.cpp \
  $(SRCDIR)/libs/csutil/generic/csprocessorcap.cpp \
  $(SRCDIR)/libs/csutil/generic/findlib.cpp \
  $(SRCDIR)/libs/csutil/generic/getopt.cpp \
  $(SRCDIR)/libs/csutil/generic/pluginpaths.cpp \
  $(SRCDIR)/libs/csutil/generic/resdir.cpp \
  $(SRCDIR)/libs/csutil/generic/runloop.cpp
endif
