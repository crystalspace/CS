# This is a subinclude file used to define the rules needed
# to build the Direct3D 6 driver -- d3ddx6

# Driver description
DESCRIPTION.d3ddx6 = Crystal Space Direct3D 6 driver

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

# The 3D Direct3D 6 driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  D3DDX6=$(OUTDLL)d3ddx61$(DLL)
  LIBS.D3DDX6=$(LIBS.LOCAL.D3DDX6)
  DEP.D3DDX6=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(D3DDX6)
else
  D3DDX6=$(OUT)$(LIB_PREFIX)d3ddx61$(LIB)
  DEP.EXE+=$(D3DDX6)
  LIBS.EXE+=$(LIBS.LOCAL.D3DDX6)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_D3DDX6
  TO_INSTALL.STATIC_LIBS+=$(D3DDX6)
endif
DESCRIPTION.$(D3DDX6) = $(DESCRIPTION.d3ddx6)
SRC.D3DDX6 = $(wildcard plugins/video/renderer/direct3d61/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp
OBJ.D3DDX6 = $(addprefix $(OUT),$(notdir $(SRC.D3DDX6:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: d3ddx6 d3ddx6clean

# Chain rules
clean: d3ddx6clean

d3ddx6: $(OUTDIRS) $(D3DDX6)

$(OUT)%$O: plugins/video/renderer/direct3d61/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.D3DDX6)
 
$(D3DDX6): $(OBJ.D3DDX6) $(DEP.D3DDX6)
	$(DO.PLUGIN) $(LIBS.D3DDX6)

d3ddx6clean:
	$(RM) $(D3DDX6) $(OBJ.D3DDX6) $(OUTOS)d3ddx6.dep

ifdef DO_DEPEND
depend: $(OUTOS)d3ddx6.dep
$(OUTOS)d3ddx6.dep: $(SRC.D3DDX6)
	$(DO.DEP1) $(CFLAGS.D3DDX6) $(DO.DEP2)
else
-include $(OUTOS)d3ddx6.dep
endif

endif # ifeq ($(MAKESECTION),targets)
