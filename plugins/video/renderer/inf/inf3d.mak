# This is a subinclude file used to define the rules needed
# to build the 3D infinite rendering driver -- inf3d

# Driver description
DESCRIPTION.inf3d = Crystal Space infinite 3D renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make inf3d        Make the $(DESCRIPTION.inf3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: inf3d
all plugins drivers drivers3d: inf3d

inf3d:
	$(MAKE_TARGET) MAKE_DLL=yes
inf3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/renderer/inf

ifeq ($(USE_PLUGINS),yes)
  INF3D = $(OUTDLL)inf3d$(DLL)
  LIB.INF3D = $(foreach d,$(DEP.INF3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(INF3D)
else
  INF3D = $(OUT)$(LIB_PREFIX)inf$(LIB)
  DEP.EXE += $(INF3D)
  SCF.STATIC += inf3d
  TO_INSTALL.STATIC_LIBS += $(INF3D)
endif

INC.INF3D = $(wildcard plugins/video/renderer/inf/*.h \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h \
  plugins/video/renderer/common/vbufmgr.h \
  plugins/video/renderer/common/polybuf.h \
  $(INC.COMMON.DRV2D))
SRC.INF3D = $(wildcard plugins/video/renderer/inf/*.cpp \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp \
  plugins/video/renderer/common/vbufmgr.cpp \
  plugins/video/renderer/common/polybuf.cpp \
  $(SRC.COMMON.DRV2D))
OBJ.INF3D = $(addprefix $(OUT),$(notdir $(SRC.INF3D:.cpp=$O)))
DEP.INF3D = CSGEOM CSGFX CSUTIL CSSYS CSUTIL

MSVC.DSP += INF3D
DSP.INF3D.NAME = inf3d
DSP.INF3D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: inf3d inf3dclean

# Chain rules
all: $(INF3D)
clean: inf3dclean

inf3d: $(OUTDIRS) $(INF3D)

$(INF3D): $(OBJ.INF3D) $(LIB.INF3D)
	$(DO.PLUGIN)

inf3dclean:
	$(RM) $(INF3D) $(OBJ.INF3D)

ifdef DO_DEPEND
dep: $(OUTOS)inf3d.dep
$(OUTOS)inf3d.dep: $(SRC.INF3D)
	$(DO.DEP)
else
-include $(OUTOS)inf3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
