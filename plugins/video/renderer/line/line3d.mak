# This is a subinclude file used to define the rules needed
# to build the 3D line rendering driver.

# Driver description
DESCRIPTION.line3d = Crystal Space line 3D renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make line3d       Make the $(DESCRIPTION.line3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: line3d
all plugins drivers drivers3d: line3d

line3d:
	$(MAKE_TARGET) MAKE_DLL=yes
line3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/renderer/line

ifeq ($(USE_PLUGINS),yes)
  LINE3D = $(OUTDLL)line3d$(DLL)
  LIB.LINE3D = $(foreach d,$(DEP.LINE3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(LINE3D)
else
  LINE3D = $(OUT)$(LIB_PREFIX)line3d$(LIB)
  DEP.EXE += $(LINE3D)
  SCF.STATIC += line3d
  TO_INSTALL.STATIC_LIBS += $(LINE3D)
endif

INC.LINE3D = $(wildcard plugins/video/renderer/line/*.h) \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h
SRC.LINE3D = $(wildcard plugins/video/renderer/line/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp
OBJ.LINE3D = $(addprefix $(OUT),$(notdir $(SRC.LINE3D:.cpp=$O)))
DEP.LINE3D = CSGEOM CSGFX CSUTIL CSSYS CSUTIL

MSVC.DSP += LINE3D
DSP.LINE3D.NAME = line3d
DSP.LINE3D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: line3d line3dclean

# Chain rules
all: $(LINE3D)
clean: line3dclean

line3d: $(OUTDIRS) $(LINE3D)

$(LINE3D): $(OBJ.LINE3D) $(LIB.LINE3D)
	$(DO.PLUGIN)

line3dclean:
	$(RM) $(LINE3D) $(OBJ.LINE3D)

ifdef DO_DEPEND
dep: $(OUTOS)line3d.dep
$(OUTOS)line3d.dep: $(SRC.LINE3D)
	$(DO.DEP)
else
-include $(OUTOS)line3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
