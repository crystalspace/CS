#=============================================================================
# This is an configuration file which describes options global to all Crystal
# Space programs and libraries.  You may edit it to suit your taste.  Also have
# a look at the platform-dependent makefile for system settings.
#=============================================================================

#-----------------------------------------------------------------------------
# If you prefer not to edit factory-supplied makefiles, such as this one, then
# you can instead create and edit the file CS/mk/local.mak or
# <builddir>/local.mak, where <builddir> is the directory in which you are
# building the project.  In local.mak, you can augment the PLUGINS and
# PLUGINS.DYNAMIC lists and override any options discovered automatically at
# project configuration time by the "configure" script.  Overrideable options
# can be found by looking in the generated CS/config.mak file after
# configuring the project.
#-----------------------------------------------------------------------------
-include $(SRCDIR)/mk/local.mak
-include local.mak


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
ifneq ($(USE_NEW_RENDERER),yes)
ifeq ($(GL.AVAILABLE),yes)
PLUGINS += video/renderer/opengl
endif
endif
PLUGINS += video/loader/mplex
PLUGINS += video/loader/gif
ifeq ($(PNG.AVAILABLE),yes)
PLUGINS += video/loader/png
endif
ifeq ($(JPEG.AVAILABLE),yes)
PLUGINS += video/loader/jpg
endif
PLUGINS += video/loader/dds
PLUGINS += video/loader/bmp
PLUGINS += video/loader/tga
PLUGINS += video/effects
PLUGINS += video/cursor
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
PLUGINS += movierecorder
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
ifeq ($(USE_NEW_RENDERER),yes)
PLUGINS += mesh/terrain/object     mesh/terrain/persist/standard
endif
PLUGINS += mesh/terrfunc/object    mesh/terrfunc/persist/standard
PLUGINS += mesh/terrbig
PLUGINS += mesh/lghtng/object
PLUGINS += mesh/null/object        mesh/null/persist/standard
PLUGINS += mesh/bcterr/object      mesh/bcterr/persist/standard
PLUGINS += mesh/metagen/object
PLUGINS += mesh/stars/object       mesh/stars/persist/standard
PLUGINS += mesh/haze/object        mesh/haze/persist/standard
PLUGINS += mesh/thing/object       mesh/thing/persist/standard
PLUGINS += mesh/bezier/object      mesh/bezier/persist/standard
PLUGINS += mesh/cloth/object
PLUGINS += mesh/particles/object   mesh/particles/persist/standard
PLUGINS += mesh/particles/physics/simple
PLUGINS += motion/standard/object  motion/standard/persist/standard
PLUGINS += engine/3d
PLUGINS += culling/dynavis
PLUGINS += culling/frustvis
PLUGINS += csparser
PLUGINS += csparser/services
PLUGINS += proctex/standard
PLUGINS += cssaver
PLUGINS += sequence
PLUGINS += engseq
PLUGINS += documentsystem/xmlread

ifeq ($(USE_NEW_RENDERER),yes)
PLUGINS.DYNAMIC += video/render3d/opengl
endif
PLUGINS.DYNAMIC += video/render3d/shader/shadermgr
PLUGINS.DYNAMIC += video/render3d/shader/shadercompiler/xmlshader
PLUGINS.DYNAMIC += video/render3d/shader/shaderplugins/glshader_arb
PLUGINS.DYNAMIC += video/render3d/shader/shaderplugins/glshader_fixed
ifeq ($(CG.AVAILABLE),yes)
PLUGINS.DYNAMIC += video/render3d/shader/shaderplugins/glshader_cg
endif
PLUGINS.DYNAMIC += engine/renderloop/loader
PLUGINS.DYNAMIC += engine/renderloop/stdsteps
PLUGINS.DYNAMIC += engine/renderloop/shadow/stencil

PLUGINS.DYNAMIC += engine/iso
PLUGINS.DYNAMIC += isoldr
PLUGINS.DYNAMIC += video/renderer/line
PLUGINS.DYNAMIC += video/renderer/null
PLUGINS.DYNAMIC += video/renderer/inf
PLUGINS.DYNAMIC += video/canvas/null2d
PLUGINS.DYNAMIC += video/canvas/memory
ifeq ($(SDL.AVAILABLE),yes)
PLUGINS.DYNAMIC += video/canvas/sdl
endif
PLUGINS.DYNAMIC += video/loader/sgi
PLUGINS.DYNAMIC += video/loader/wal
ifeq ($(MNG.AVAILABLE),yes)
PLUGINS.DYNAMIC += video/loader/jng
endif
PLUGINS.DYNAMIC += proctex/ptanimimg
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
ifeq ($(3DS.AVAILABLE),yes)
PLUGINS.DYNAMIC += mesh/impexp/3ds
endif
ifeq ($(HAS_CAL3D),yes)
PLUGINS.DYNAMIC += mesh/sprcal3d/object
PLUGINS.DYNAMIC += mesh/sprcal3d/persist
endif
PLUGINS.DYNAMIC += font/server/fontplex
ifeq ($(FT2.AVAILABLE),yes)
PLUGINS.DYNAMIC += font/server/freefnt2
endif
PLUGINS.DYNAMIC += aws
ifeq ($(PGSERVER.AVAILABLE), yes)
PLUGINS.DYNAMIC += picogui/server
endif
ifeq ($(VORBISFILE.AVAILABLE),yes)
PLUGINS.DYNAMIC += sound/loader/ogg
endif
ifeq ($(MIKMOD.AVAILABLE),yes)
PLUGINS.DYNAMIC += sound/loader/mod
endif
ifeq ($(OPENAL.AVAILABLE),yes)
PLUGINS.DYNAMIC += sound/renderer/openal
endif
ifeq ($(ODE.AVAILABLE),yes)
PLUGINS.DYNAMIC += physics/odedynam
PLUGINS.DYNAMIC += physics/loader
PLUGINS.DYNAMIC += mesh/particles/physics/ode
endif
ifeq ($(LINUXJOYSTICK.AVAILABLE),yes)
PLUGINS.DYNAMIC += device/joystick/linux
endif
ifeq ($(DIRECTX.AVAILABLE),yes)
PLUGINS.DYNAMIC += device/joystick/windows
endif
ifeq ($(VOS.AVAILABLE),yes)
PLUGINS.DYNAMIC += net/vos
endif

PLUGINS.DYNAMIC += documentsystem/binary
PLUGINS.DYNAMIC += documentsystem/xmltiny
PLUGINS.DYNAMIC += documentsystem/dsplex

ifeq ($(PYTHON.AVAILABLE),yes)
PLUGINS.DYNAMIC += cscript/cspython
endif

ifeq ($(PERL5.AVAILABLE),yes)
PLUGINS.DYNAMIC += cscript/csperl5
endif
