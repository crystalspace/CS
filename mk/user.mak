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
PLUGINS += collide/rapid
PLUGINS += perfstat
PLUGINS += bugplug
PLUGINS += reporter stdrep
PLUGINS += mesh/crossbld
PLUGINS += mesh/impexp/multiplex
PLUGINS += mesh/impexp/3ds         mesh/impexp/ase
PLUGINS += mesh/impexp/dxf         mesh/impexp/hrc
PLUGINS += mesh/impexp/iv          mesh/impexp/md2
PLUGINS += mesh/impexp/obj         mesh/impexp/pov
PLUGINS += mesh/impexp/smf         mesh/impexp/spr
PLUGINS += mesh/impexp/stla        mesh/impexp/vla
PLUGINS += mesh/impexp/mdl
PLUGINS += mesh/cube/object        mesh/cube/persist/classic
PLUGINS += mesh/spr2d/object       mesh/spr2d/persist/classic
PLUGINS += mesh/spr3d/object       mesh/spr3d/persist/classic
PLUGINS += mesh/fountain/object    mesh/fountain/persist/classic
PLUGINS += mesh/explo/object       mesh/explo/persist/classic
PLUGINS += mesh/fire/object        mesh/fire/persist/classic
PLUGINS += mesh/snow/object        mesh/snow/persist/classic
PLUGINS += mesh/rain/object        mesh/rain/persist/classic
PLUGINS += mesh/spiral/object      mesh/spiral/persist/classic
PLUGINS += mesh/ball/object        mesh/ball/persist/classic
PLUGINS += mesh/surf/object        mesh/surf/persist/classic
PLUGINS += mesh/emit/object        mesh/emit/persist/classic
PLUGINS += mesh/metaball/object    mesh/metaball/persist/classic
PLUGINS += mesh/terrfunc/object    mesh/terrfunc/persist/classic
PLUGINS += mesh/metagen/object
PLUGINS += mesh/stars/object
PLUGINS += mesh/haze/object        mesh/haze/persist/classic
PLUGINS += mesh/thing/persist/classic
PLUGINS += motion/standard/object  motion/standard/persist/classic
PLUGINS += engine/3d
PLUGINS += csparser

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
PLUGINS.DYNAMIC += console/output/standard
PLUGINS.DYNAMIC += console/output/fancy
PLUGINS.DYNAMIC += font/server/fontplex
#PLUGINS.DYNAMIC += font/server/freefont
#PLUGINS.DYNAMIC += aws

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
