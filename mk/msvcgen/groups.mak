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
# groups.mak
#
#	This makfile component defines pseudo-project groups which should
#	appear in each workspace.
#
#	In the discussion which follows, assume that "GRP" is the core name of
#	the group being defined.  The following variables are used to define
#	the groups.
#
#	o MSVC.GROUPS -- Master list of pseudo-project groups to include in
#         each generated workspace.  Entries must be *appended* to this list
#         with the "+=" operator.  For example, "MSVC.GROUPS += PLUGINS".  The
#         MSVC.GROUPS list is analogous to MSVC.DSP list (see msvcgen.mak),
#         except that it lists group projects rather than literal projects.
#         Group projects define collections of literal projects.  For instance,
#         one might define a "plugins" group which depends upon all of the
#         plugin projects in the workspace.
#
#	o DSP.GRP.NAME -- Base name (such as "plugins") for the generated
#	  group.  Same meaning as explained in msvcgen.mak.
#
#	o DSP.GRP.TYPE -- Project file's type.  For groups, it should be
#	  "group".
#
#	o DSP.GRP.DEPEND -- List of dependencies for this module.  For groups,
#	  lists all of the projects upon which the group depends.  See
#	  msvcgen.mak for a complete description of this variable.
#
# *NOTE* AUGMENTED
#        The macro MSVC.DSP.AUGMENTED generates a type-augmented project list.
#        For example, (CSGEOM CSFONT VFS) becomes (library.CSGEOM plugin.CSFONT
#        plugin.VFS).  Later, the DSP.PROJECT.DEPEND variables filter the list
#        by type and strip off the type-prefix in order to arrive at a
#        bare-word dependency list.  For instance, for the pseudo-project
#        "grpplugins", the list is filtered to (plugin.CSFONT plugin.VFS), and
#        then transformed into (CSFONT VFS), which is exactly the list of
#        bare-word project names upon which "grpplugins" should depend.
#
#------------------------------------------------------------------------------

# Macro to generate a type-augmented project list. (*NOTE* AUGMENTED)
MSVC.DSP.AUGMENTED = $(foreach d,$(MSVC.DSP),$(DSP.$d.TYPE).$d)

# grpall -- represents all projects.
MSVC.GROUPS += ALL
DSP.ALL.NAME = all
DSP.ALL.TYPE = group
DSP.ALL.DEPEND = $(MSVC.DSP)

# grpapps -- represents all GUI and console application projects.
MSVC.GROUPS += APPS
DSP.APPS.NAME = apps
DSP.APPS.TYPE = group
DSP.APPS.DEPEND = $(patsubst appgui.%,%,$(patsubst appcon.%,%,\
  $(filter appgui.% appcon.%,$(MSVC.DSP.AUGMENTED))))

# grpplugins -- represents all plug-in projects.
MSVC.GROUPS += PLUGINS
DSP.PLUGINS.NAME = plugins
DSP.PLUGINS.TYPE = group
DSP.PLUGINS.DEPEND = \
  $(patsubst plugin.%,%,$(filter plugin.%,$(MSVC.DSP.AUGMENTED)))

# grplibs -- represents all static library projects.
MSVC.GROUPS += LIBS
DSP.LIBS.NAME = libs
DSP.LIBS.TYPE = group
DSP.LIBS.DEPEND = \
  $(patsubst library.%,%,$(filter library.%,$(MSVC.DSP.AUGMENTED)))
