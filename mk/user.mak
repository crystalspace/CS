#=============================================================================
# This is an configuration file which describes user options global to all
# Crystal Space programs and libraries.  Edit it to suit your taste.  Also
# have a look at the platform-dependent makefile for system settings.
#=============================================================================

#-----------------------------------------------------------------------------
# Dynamic Settings
# Changes to these settings will be reflected in the makefile system
# immediately.
#-----------------------------------------------------------------------------

# Default list of plugins to build.  Note that you'll link all plugins into
# executable in the case of static build.  If you want additional plugins to
# be built either define the environment variable PLUGINS or put a line similar
# to those below into config.mak. The plugins listed in PLUGINS variable
# are always built; PLUGINS.DYNAMIC are built only if plugins are compiled
# as shared libraries. Please think twice before adding anything to PLUGINS;
# in most cases you will want to add to PLUGINS.DYNAMIC.

PLUGINS += filesys/vfs
PLUGINS += video/renderer/software
PLUGINS += video/loader/mplex
PLUGINS += video/loader/gif
PLUGINS += video/loader/png
PLUGINS += video/loader/jpg
PLUGINS += sound/loader/mplex
PLUGINS += sound/loader/au
PLUGINS += sound/loader/aiff
PLUGINS += sound/loader/iff
PLUGINS += sound/loader/wav
PLUGINS += font/server/csfont 
PLUGINS += console/output/simple
PLUGINS += console/input/standard
PLUGINS += colldet/rapid
PLUGINS += perfstat
PLUGINS += bugplug
PLUGINS += mesh/object/cube        mesh/loader/cube
PLUGINS += mesh/object/spr2d       mesh/loader/spr2d
PLUGINS += mesh/object/spr3d       mesh/loader/spr3d
PLUGINS += mesh/object/fountain    mesh/loader/fountain
PLUGINS += mesh/object/explo       mesh/loader/explo
PLUGINS += mesh/object/fire        mesh/loader/fire
PLUGINS += mesh/object/snow        mesh/loader/snow
PLUGINS += mesh/object/rain        mesh/loader/rain
PLUGINS += mesh/object/spiral      mesh/loader/spiral
PLUGINS += mesh/object/ball        mesh/loader/ball
PLUGINS += mesh/object/surf        mesh/loader/surf
PLUGINS += mesh/object/emit        mesh/loader/emit
PLUGINS += mesh/object/stars
PLUGINS +=                         mesh/loader/thing
PLUGINS += mesh/object/metaball    mesh/loader/metaball
PLUGINS += terrain/object/terrfunc terrain/loader/terrfunc
PLUGINS += motion/object/default   motion/loader/default

PLUGINS.DYNAMIC += engine/3d
PLUGINS.DYNAMIC += engine/iso
PLUGINS.DYNAMIC += cslexan
PLUGINS.DYNAMIC += sequence
PLUGINS.DYNAMIC += video/renderer/line
PLUGINS.DYNAMIC += video/renderer/null
PLUGINS.DYNAMIC += video/renderer/inf
PLUGINS.DYNAMIC += video/loader/bmp
PLUGINS.DYNAMIC += video/loader/tga
PLUGINS.DYNAMIC += video/loader/sgi
PLUGINS.DYNAMIC += video/loader/wal
PLUGINS.DYNAMIC += net/driver/socket
PLUGINS.DYNAMIC += sound/loader
PLUGINS.DYNAMIC += console/output/standard
PLUGINS.DYNAMIC += console/output/fancy
PLUGINS.DYNAMIC += dungeon
PLUGINS.DYNAMIC += font/server/fontplex
#PLUGINS.DYNAMIC += font/server/freefont
#PLUGINS.DYNAMIC += csgame/gamecore

#-----------------------------------------------------------------------------
# Static Settings            *** TAKE NOTE ***
# After changing the settings in this section, you must re-run the makefile
# configuration step (for example, "make linux") in order for your changes
# to become effective.
#-----------------------------------------------------------------------------

# Should we build drivers/plugins as loadable modules?
ifndef USE_PLUGINS
  USE_PLUGINS=yes
endif

# Should we build libraries as shared/dynamic libraries?
# Currently only supported on Unix
ifndef USE_SHARED_LIBS
  USE_SHARED_LIBS=no
endif

# Default build mode
ifndef MODE
  MODE=optimize
endif

# Should we use NASM for assembly?
ifndef NASM.INSTALLED
  NASM.INSTALLED=no
endif

# If 'yes' include assembly optimizations in Crystal Space.  On systems that
# don't support this, setting this option to 'yes' does nothing.
DO_ASM=yes

# If "yes" include MMX support in software renderer
DO_MMX=yes

# If "yes", then in UNIX create core dumps on crash.
# Be warned, they are large (> 20MB)!
DO_COREDUMP=no

# Set to 1 to use Mesa instead of "real" OpenGL.  You can define MESA_PATH
# variable in environment to point to MesaGL base path. If Mesa is not
# used then you can use OPENGL_PATH to point to the base of the OpenGL
# libraries and includes.
USE_MESA=0

# The tool used to build dependencies. The possible values are:
# none  - Cannot build dependencies on this platform
# cc    - Use the C compiler (gcc -MM) for this
# mkdep - Use the makedep tool provided in the apps/makedep directory
ifndef DEPEND_TOOL
  DEPEND_TOOL=cc
endif
