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

PLUGINS += video/renderer video/canvas
PLUGINS += filesys/vfs
PLUGINS += simpcon
PLUGINS += font/server/csfont 
PLUGINS += colldet/rapid
PLUGINS += meshobj/cube meshldr/cube
PLUGINS += meshobj/spr2d meshldr/spr2d
PLUGINS += meshobj/fountain meshldr/fountain
PLUGINS += meshobj/explo meshldr/explo
PLUGINS += meshobj/fire meshldr/fire
PLUGINS += perfstat
PLUGINS.DYNAMIC += video/renderer/line video/renderer/null video/renderer/inf
PLUGINS.DYNAMIC += sound/loader
PLUGINS.DYNAMIC += cscon
PLUGINS.DYNAMIC += csclear
PLUGINS.DYNAMIC += dungeon
PLUGINS.DYNAMIC += engine
PLUGINS.DYNAMIC += net/driver/socket
#PLUGINS.DYNAMIC += font/server/freefont
PLUGINS.DYNAMIC += font/server/fontplex
PLUGINS.DYNAMIC += metaball
PLUGINS.DYNAMIC += motion
#PLUGINS.DYNAMIC += csstdldr
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

# If 'yes' include support for PNG graphic files (required for MazeD).
DO_PNG=yes

# If 'yes' include support for GIF graphic files (required for all current
# data files).
DO_GIF=yes

# If 'yes' include support for BMP graphic files (not required currently).
DO_BMP=yes

# If 'yes' include support for TGA graphic files (not required currently).
DO_TGA=yes

# If 'yes' include support for JPG graphic files (not required currently).
DO_JPG=yes

# If 'yes' include support for WAL graphic files (not required currently).
DO_WAL=yes

# If 'yes' include support for SGI graphic files (not required currently).
DO_SGI=yes

# If 'yes' include sound support.
DO_SOUND=yes

# If "yes" include AIFF support
DO_AIFF=yes

# If "yes" include IFF support
DO_IFF=yes

# If "yes" include AU support
DO_AU=yes

# If "yes" include WAV support
DO_WAV=yes

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
