#==============================================================================
#
#    Automatic MSVC-compliant workspace and project generation component
#    Copyright (C) 2004 by Eric Sunshine <sunshine@sunshineco.com>
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
#	This makefile component defines MSVC workspaces (.sln/.dsw files) which
#	should be synthesized.  It is possible to create customized workspaces
#	by specifying which projects are placed into which workspaces.  For
#	instance, an "all" workspace might contain all modules in the entire
#	project, whereas a "typical" workspace might contain only modules which
#	the typical user will be interested in building, and an "experimental"
#	module might contain experimental modules in addition to typically used
#	modules.
#
#	In the discussion which follows, assume that "WKS" is the core name of
#	the workspace being defined.  The following variables are used to
#	define the workspaces.
#
#	o MSVC.WORKSPACES -- Master list of workspaces to be generated.
#	  Entries must be *appended* to this list with the "+=" operator.  For
#	  example, "MSVC.WORKSPACES += WKS".
#
#	o MSVC.WORKSPACE.WKS.NAME -- Specifies the human-readable base filename
#	  for the workspace (such as "typical" or "experimental").
#
#	o MSVC.WORKSPACE.WKS.ACCEPT -- Optional opt-in filter.  By default a
#	  workspace includes all modules in the project.  The list can be
#	  filtered down by defining this variable (discussed below), which
#	  indicates modules that should be accepted into this workspace by
#	  explicit inclusion.
#
#	o MSVC.WORKSPACE.WKS.REJECT -- Optional opt-out filter.  By default a
#	  workspace includes all modules in the project.  The list can be
#	  filtered down by defining this variable (discussed below), which
#	  indicates modules that should be explicitly rejected from the
#	  workspace.
#
#	The MSVC.WORKSPACE.WKS.ACCEPT and MSVC.WORKSPACE.WKS.REJECT variables
#	should contain a whitespace-separated list of Perl regular expressions
#	that match the filenames of the generated project files (.vcproj/.dsp)
#	which should be accepted into or rejected from the workspace.  The
#	ACCEPT and REJECT lists can be used alone or in combination.  If used
#	together, REJECT expressions take precedence over ACCEPT expressions.
#
#------------------------------------------------------------------------------

# Everything.
MSVC.WORKSPACES += ALL
MSVC.WORKSPACE.ALL.NAME = all

# What the typical user will need. Presently, everything but the new renderer.
MSVC.WORKSPACES += TYPICAL
MSVC.WORKSPACE.TYPICAL.NAME = typical
MSVC.WORKSPACE.TYPICAL.REJECT = \
  chunklod \
  glrender3d \
  glshader_arb \
  glshader_cg \
  glshader_fixed \
  shadermgr \
  softrender3d \
  softshader \
  terrainldr \
  waterdemo \
  xmlshader

# Similar to "typical" but using the new renderer instead of the old.
MSVC.WORKSPACES += NEWRENDERER
MSVC.WORKSPACE.NEWRENDERER.NAME = newrenderer
MSVC.WORKSPACE.NEWRENDERER.REJECT = soft3d gl3d

# Walktest plus all plugins commonly required by Walktest.
MSVC.WORKSPACES += WALKALL
MSVC.WORKSPACE.WALKALL.NAME = walkall
MSVC.WORKSPACE.WALKALL.ACCEPT = \
    appwalktest \
    libcsgeom \
    libcsgfx \
    libcstool \
    libcsutil \
    plgball \
    plgballldr \
    plgbcterr \
    plgbezier \
    plgbezierldr \
    plgbugplug \
    plgcrossbld \
    plgcsbmpimg \
    plgcsconin \
    plgcsfont \
    plgcsgifimg \
    plgcsjpgimg \
    plgcsparser \
    plgcspngimg \
    plgcssynldr \
    plgddraw2d \
    plgdynavis \
    plgemit \
    plgemitldr \
    plgengine \
    plgengseq \
    plgexplo \
    plgexploldr \
    plgfire \
    plgfireldr \
    plgfountain \
    plgfountldr \
    plgfrustvis \
    plggenmesh \
    plggl3d \
    plgglwin32 \
    plggmeshldr \
    plggtreeldr \
    plggtreeldr \
    plghaze \
    plghazeldr \
    plgie3ds \
    plgieplex \
    plgimgplex \
    plglghtng \
    plgmd2ie \
    plgmdlie \
    plgnullmesh \
    plgnullmeshldr \
    plgparticles \
    plgparticlesldr \
    plgpartphys_ode \
    plgpartphys_simple \
    plgptanimimg \
    plgrain \
    plgrainldr \
    plgrapid \
    plgreporter \
    plgsequence \
    plgsimpcon \
    plgsnow \
    plgsnowldr \
    plgsoft3d \
    plgspiral \
    plgspirldr \
    plgspr2d \
    plgspr2dldr \
    plgspr3d \
    plgspr3dbin \
    plgspr3dldr \
    plgstarldr \
    plgstars \
    plgstdpt \
    plgstdrep \
    plgterrbig \
    plgterrfunc \
    plgthing \
    plgthingldr \
    plgvfs \
    plgxmlread
