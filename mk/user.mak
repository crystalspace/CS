#=============================================================================
# This is an configuration file which describes options global to all Crystal
# Space programs and libraries.  You may edit it to suit your taste.  Also have
# a look at the platform-dependent makefile for system settings.
#=============================================================================

#-----------------------------------------------------------------------------
# If you prefer not to edit factory-supplied makefiles, such as this one, then
# you can instead create and edit the file CS/mk/local.mak.  In this file, you
# can augment the PLUGINS and PLUGINS.DYNAMIC lists and override any options
# discovered automatically at project configuration time by the "configure"
# script.  Overrideable options can be found by looking in the generated
# CS/config.mak file after configuring the project.
#-----------------------------------------------------------------------------
-include mk/local.mak


#-----------------------------------------------------------------------------
# Default list of plugins to build.  The plugins listed by the PLUGINS variable
# are always built.  Plugins listed by PLUGINS.DYNAMIC are built only if the
# project is configured with --enable-plugins (which is the default).  Please
# think twice before adding anything to PLUGINS; in most cases you will want to
# add optional plugins to PLUGINS.DYNAMIC.  Be aware that the platform-specific
# makefile (CS/libs/cssys/*.mak) may augment the plugin list with plugins
# specific to the platform.
#
# If you want additional plugins to be built, you can augment the PLUGINS or
# PLUGINS.DYNAMIC variables in your CS/mk/local.mak file as noted above.
#
# Note that if the project was configured with --disable-plugins, then all
# modules which would otherwise have been independent plugins will get linked
# into each executable.
#-----------------------------------------------------------------------------
ifeq ($(ZLIB.AVAILABLE),yes)
PLUGINS += filesys/vfs
endif
PLUGINS += video/renderer/software
ifeq ($(GL.AVAILABLE),yes)
PLUGINS += video/renderer/opengl
endif
PLUGINS += video/loader/mplex
PLUGINS += video/loader/gif
ifeq ($(PNG.AVAILABLE),yes)
PLUGINS += video/loader/png
endif
ifeq ($(JPEG.AVAILABLE),yes)
PLUGINS += video/loader/jpg
endif
PLUGINS += video/loader/bmp
PLUGINS += video/loader/tga
PLUGINS += video/effects
PLUGINS += sound/loader/mplex
PLUGINS += sound/loader/au
PLUGINS += sound/loader/aiff
PLUGINS += sound/loader/iff
PLUGINS += sound/loader/wav
PLUGINS += font/server/csfont 
PLUGINS += console/output/simple
PLUGINS += console/input/standard
PLUGINS += collide/rapid
PLUGINS += collide/opcode
PLUGINS += perfstat
PLUGINS += bugplug
PLUGINS += reporter stdrep
PLUGINS += mesh/crossbld
PLUGINS += mesh/impexp/ieplex
PLUGINS += mesh/spr2d/object       mesh/spr2d/persist/standard
PLUGINS += mesh/spr3d/object       mesh/spr3d/persist/standard
PLUGINS +=                         mesh/spr3d/persist/binary
PLUGINS += mesh/fountain/object    mesh/fountain/persist/standard
PLUGINS += mesh/explo/object       mesh/explo/persist/standard
PLUGINS += mesh/fire/object        mesh/fire/persist/standard
PLUGINS += mesh/snow/object        mesh/snow/persist/standard
PLUGINS += mesh/rain/object        mesh/rain/persist/standard
PLUGINS += mesh/spiral/object      mesh/spiral/persist/standard
PLUGINS += mesh/ball/object        mesh/ball/persist/standard
PLUGINS += mesh/genmesh/object     mesh/genmesh/persist/standard
PLUGINS +=                         mesh/genmesh/persist/tree
PLUGINS += mesh/emit/object        mesh/emit/persist/standard
PLUGINS += mesh/metaball/object    mesh/metaball/persist/standard
PLUGINS += mesh/terrfunc/object    mesh/terrfunc/persist/standard
PLUGINS += mesh/terrbig
PLUGINS += mesh/null/object
PLUGINS += mesh/bcterr/object      mesh/bcterr/persist/standard
PLUGINS += mesh/metagen/object
PLUGINS += mesh/stars/object       mesh/stars/persist/standard
PLUGINS += mesh/haze/object        mesh/haze/persist/standard
PLUGINS += mesh/thing/object       mesh/thing/persist/standard
PLUGINS += mesh/cloth/object
PLUGINS += motion/standard/object  motion/standard/persist/standard
PLUGINS += engine/3d
PLUGINS += culling/dynavis
PLUGINS += culling/frustvis
PLUGINS += csparser
PLUGINS += csparser/services
PLUGINS += cssaver
PLUGINS += sequence
PLUGINS += engseq

PLUGINS.DYNAMIC += engine/iso
PLUGINS.DYNAMIC += isoldr
PLUGINS.DYNAMIC += video/renderer/line
PLUGINS.DYNAMIC += video/renderer/null
PLUGINS.DYNAMIC += video/renderer/inf
PLUGINS.DYNAMIC += video/canvas/null2d
PLUGINS.DYNAMIC += video/canvas/memory
PLUGINS.DYNAMIC += video/loader/sgi
PLUGINS.DYNAMIC += video/loader/wal
ifeq ($(MNG.AVAILABLE),yes)
PLUGINS.DYNAMIC += video/loader/jng
endif
ifeq ($(SOCKET.AVAILABLE),yes)
PLUGINS.DYNAMIC += net/driver/socket
PLUGINS.DYNAMIC += net/driver/ensocket
endif
PLUGINS.DYNAMIC += net/manager
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
#PLUGINS.DYNAMIC += font/server/freefnt2
PLUGINS.DYNAMIC += aws
#PLUGINS.DYNAMIC += sound/loader/ogg
#PLUGINS.DYNAMIC += sound/loader/mod
#PLUGINS.DYNAMIC += sound/renderer/openal
#PLUGINS.DYNAMIC += physics/odedynam
#PLUGINS.DYNAMIC += physics/loader
#PLUGINS.DYNAMIC += device/joystick/linux
#PLUGINS.DYNAMIC += video/render3d/opengl
#PLUGINS.DYNAMIC += video/render3d/shadermgr
#PLUGINS.DYNAMIC += video/render3d/shaderplugins/glshader_arb

ifeq ($(PYTHON.AVAILABLE),yes)
PLUGINS.DYNAMIC += cscript/cspython
endif

#PLUGINS.DYNAMIC += cscript/cslua

# Unfortunately, we can not yet enable this module automatically -- even if
# the configuration script detects its presence -- since it fails to build on
# most platforms.
ifeq ($(PERL.AVAILABLE),yes)
#PLUGINS.DYNAMIC += cscript/csperl5
endif
