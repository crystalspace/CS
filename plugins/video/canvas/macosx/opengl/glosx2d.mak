#==============================================================================
# This is a MacOS X 2D graphics driver makefile that uses openGL to
# do most of it's work
# Copyright (C) 2001 by Matt Reda <mreda@mac.com>
#==============================================================================

DESCRIPTION.glosx2d = Crystal Space $(NEXT.DESCRIPTION) OpenGL 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glosx2d      Make the $(DESCRIPTION.glosx2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glosx2d glosx2dclean
all softcanvas plugins drivers drivers2d: glosx2d

glosx2d:
	$(MAKE_TARGET) MAKE_DLL=yes DO_GLOSX2D=yes
glosx2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_GLOSX2D_PATHS = plugins/video/canvas/macosx/opengl plugins/video/canvas/macosx/common
NEXT.HEADER_GLOSX2D_PATHS = $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_GLOSX2D_PATHS))

# Only add header search paths if actually building this plug-in or if
# USE_PLUGINS=no, in which case this module might be built as the dependency
# of some other module (rather than being built explicitly by the `glosx2d'
# target).
ifeq ($(USE_PLUGINS),no)
  CFLAGS.INCLUDE += $(NEXT.HEADER_GLOSX2D_PATHS)
else
ifeq ($(DO_GLOSX2D),yes)
  CFLAGS.INCLUDE += $(NEXT.HEADER_GLOSX2D_PATHS)
endif
endif

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

#vpath %.cpp $(NEXT.SOURCE_GLOSX2D_PATHS)
#vpath %.m   $(NEXT.SOURCE_GLOSX2D_PATHS)

ifeq ($(USE_PLUGINS),yes)
  GLOSX2D = $(OUTDLL)glosx2d$(DLL)
  LIB.GLOSX2D = $(foreach d,$(DEP.GLOSX2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GLOSX2D)
else
  GLOSX2D = $(OUT)$(LIB_PREFIX)glosx2d$(LIB)
  DEP.EXE += $(GLOSX2D)
  SCF.STATIC += glosx2d
  TO_INSTALL.STATIC_LIBS += $(GLOSX2D)
endif

INC.GLOSX2D = $(wildcard $(INC.COMMON.DRV2D) $(INC.COMMON.DRV2D.OPENGL) \
  $(addsuffix /*.h,$(NEXT.SOURCE_GLOSX2D_PATHS)))
SRC.GLOSX2D = $(wildcard $(SRC.COMMON.DRV2D) $(SRC.COMMON.DRV2D.OPENGL) \
  $(addsuffix /*.cpp,$(NEXT.SOURCE_GLOSX2D_PATHS)) \
  $(addsuffix /*.m,$(NEXT.SOURCE_GLOSX2D_PATHS)))
OBJ.GLOSX2D = $(addprefix $(OUT), \
  $(notdir $(subst .cpp,$O,$(SRC.GLOSX2D:.m=$O))))
DEP.GLOSX2D = CSSYS CSUTIL

# Define constants to indicate where OpenGL headers/framework are
CFLAGS.GLOSX2D = -DCS_OPENGL_PATH=OpenGL
LIB.GLOSX2D.OPENGL = $(LIBS.OPENGL.SYSTEM)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glosx2d glosx2dclean

glosx2d: $(OUTDIRS) $(GLOSX2D)

# Rule to make common OpenGL source
$(OUT)%$O: plugins/video/canvas/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLOSX2D)

# Rules to make OSX sources
$(OUT)%$O: plugins/video/canvas/macosx/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLOSX2D)

$(OUT)%$O: plugins/video/canvas/macosx/opengl/%.m
	$(DO.COMPILE.C) $(CFLAGS.GLOSX2D)

$(GLOSX2D): $(OBJ.GLOSX2D) $(LIB.GLOSX2D)
	$(DO.PLUGIN.PREAMBLE) $(DO.PLUGIN.CORE) $(LIB.GLOSX2D.OPENGL) $(DO.PLUGIN.POSTAMBLE)


clean: glosx2dclean
glosx2dclean:
	$(RM) $(GLOSX2D) $(OBJ.GLOSX2D)

ifdef DO_DEPEND
dep: $(OUTOS)glosx2d.dep
$(OUTOS)glosx2d.dep: $(SRC.GLOSX2D)
	$(DO.DEP1) $(CFLAGS.GLOSX2D) $(NEXT.HEADER_GLOSX2D_PATHS) $(DO.DEP2)
else
-include $(OUTOS)glosx2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)


