# This is a subinclude file used to define the rules needed
# to build the Direct3D 6 driver -- d3ddx61

# Driver description
DESCRIPTION.d3ddx6 = Crystal Space Direct3D V6 renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make d3ddx6       Make the $(DESCRIPTION.d3ddx6)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: d3ddx6 d3ddx6clean
all plugins drivers drivers3d: d3ddx6

d3ddx6:
	$(MAKE_TARGET) MAKE_DLL=yes
d3ddx6clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  D3DDX6 = $(OUTDLL)d3ddx61$(DLL)
  LIB.D3DDX6 = $(foreach d,$(DEP.D3DDX6),$($d.LIB))
  LIB.D3DDX6.SPECIAL = $(LIB.D3DDX6.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(D3DDX6)
else
  D3DDX6 = $(OUT)$(LIB_PREFIX)d3ddx61$(LIB)
  DEP.EXE += $(D3DDX6)
  LIBS.EXE += $(LIB.D3DDX6.LOCAL)
  SCF.STATIC += d3ddx61
  TO_INSTALL.STATIC_LIBS += $(D3DDX6)
endif

INC.D3DDX6 = $(wildcard plugins/video/renderer/direct3d61/*.h) \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h
SRC.D3DDX6 = $(wildcard plugins/video/renderer/direct3d61/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp
OBJ.D3DDX6 = $(addprefix $(OUT),$(notdir $(SRC.D3DDX6:.cpp=$O)))
DEP.D3DDX6 = CSGEOM CSGFX CSUTIL CSSYS
CFG.D3DDX6 = data/config/direct3ddx6.cfg

TO_INSTALL.CONFIG += $(CFG.D3DDX6)

MSVC.DSP += D3DDX6
DSP.D3DDX6.NAME = d3ddx61
DSP.D3DDX6.TYPE = plugin
DSP.D3DDX6.RESOURCES = $(wildcard plugins/video/renderer/direct3d61/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: d3ddx6 d3ddx6clean
clean: d3ddx6clean

d3ddx6: $(OUTDIRS) $(D3DDX6)

$(OUT)%$O: plugins/video/renderer/direct3d61/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.D3DDX6)
 
$(D3DDX6): $(OBJ.D3DDX6) $(LIB.D3DDX6)
	$(DO.PLUGIN) $(LIB.D3DDX6.SPECIAL)

d3ddx6clean:
	$(RM) $(D3DDX6) $(OBJ.D3DDX6)

ifdef DO_DEPEND
depend: $(OUTOS)d3ddx6.dep
$(OUTOS)d3ddx6.dep: $(SRC.D3DDX6)
	$(DO.DEP1) $(CFLAGS.D3DDX6) $(DO.DEP2)
else
-include $(OUTOS)d3ddx6.dep
endif

endif # ifeq ($(MAKESECTION),targets)
