#=============================================================================
# This is an configuration file which describes user options global to all
# Crystal Space programs and libraries.  Edit it to suit your taste.  Also
# have a look at the platform-dependent makefile for system settings.
#
# Documentation copyright (C)1999-2001 by Eric Sunshine.
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
PLUGINS += video/loader/bmp
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
PLUGINS += mesh/thing/object       mesh/thing/persist/classic
PLUGINS += motion/standard/object  motion/standard/persist/classic
PLUGINS += engine/3d
PLUGINS += csparser
PLUGINS += csparser/services

PLUGINS.DYNAMIC += engine/iso
PLUGINS.DYNAMIC += cslexan
PLUGINS.DYNAMIC += sequence
PLUGINS.DYNAMIC += video/renderer/line
PLUGINS.DYNAMIC += video/renderer/null
PLUGINS.DYNAMIC += video/renderer/inf
PLUGINS.DYNAMIC += video/loader/tga
PLUGINS.DYNAMIC += video/loader/sgi
PLUGINS.DYNAMIC += video/loader/wal
PLUGINS.DYNAMIC += net/driver/socket
PLUGINS.DYNAMIC += console/output/standard
PLUGINS.DYNAMIC += console/output/fancy
PLUGINS.DYNAMIC += mesh/impexp/ase
PLUGINS.DYNAMIC += mesh/impexp/md2
PLUGINS.DYNAMIC += mesh/impexp/mdl
PLUGINS.DYNAMIC += mesh/impexp/obj
PLUGINS.DYNAMIC += mesh/impexp/pov
PLUGINS.DYNAMIC += mesh/impexp/spr
#PLUGINS.DYNAMIC += mesh/impexp/3ds
PLUGINS.DYNAMIC += font/server/fontplex
#PLUGINS.DYNAMIC += font/server/freefont
PLUGINS.DYNAMIC += aws
#PLUGINS.DYNAMIC += sound/loader/ogg
#PLUGINS.DYNAMIC += sound/loader/mp3
#PLUGINS.DYNAMIC += sound/loader/mod

#-----------------------------------------------------------------------------
# Static Settings            *** TAKE NOTE ***
# After changing the settings in this section, you must re-run the makefile
# configuration step (for example, "make unknown" followed by "make linux")
# in order for your changes to become effective.
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

# Default build mode (either "optimize", "debug", or "profile").
ifndef MODE
  MODE=optimize
endif

# If MODE is "debug", should we use the extensive heap validating machinery?
# Do not enable this if you platform or your own code uses an overloaded `new'
# operator.
ifndef EXTENSIVE_MEMDEBUG
  EXTENSIVE_MEMDEBUG=no
endif

# Should we cache makefile information?  Caching information speeds up the
# build process by avoiding the time-consuming scanning which locates and then
# reads makefiles throughout the project.  Usually caching is desirable.
ifndef USE_MAKEFILE_CACHE
  USE_MAKEFILE_CACHE=yes
endif

# Should we monitor and automatically refresh an outdated makefile cache?
# The makefile cache is created when the makefile system is first configured,
# and then automatically refreshed when the chosen platform-specific makefile
# changes, user.mak changes, or one of the source makefiles for the cache
# changes.  Unfortunately, monitoring the source makefiles for changes adds
# some overhead and slows down the build process slightly.  If you have a slow
# disk subsystem, then you may want to disable the cache monitoring facility
# since even this small amount of extra overhead may become annoying.  In most
# cases, it is safe to disable this facility since makefiles rarely change,
# thus it is unlikely that the cache will become outdated.  Even if it does
# become outdated, you can still rebuild it manually by invoking the `recache'
# makefile target (i.e. "make recache").  Beware, however, that if you update
# the project frequently from the CVS repository, it is probably safest to
# leave the cache monitor enabled, just in case other developers modify
# makefiles in the repository.
ifndef MONITOR_MAKEFILE_CACHE
  MONITOR_MAKEFILE_CACHE=yes
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
