# This is a subinclude file used to define the rules needed
# to build the Direct3D 5 driver -- d3ddx5

# Driver description
DESCRIPTION.d3ddx5 = Crystal Space Direct3D 5 driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make d3ddx5         Make the $(DESCRIPTION.d3ddx5)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: d3ddx5

all plugins drivers drivers3d: d3ddx5

d3ddx5:
	$(MAKE_TARGET) MAKE_DLL=yes
d3ddx5clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The 3D Direct3D 5 driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  D3DDX5=$(OUTDLL)d3ddx5$(DLL)
  LIBS.D3DDX5=$(LIBS.LOCAL.D3DDX5)
  DEP.D3DDX5=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  D3DDX5=$(OUT)$(LIB_PREFIX)d3ddx5$(LIB)
  DEP.EXE+=$(D3DDX5)
  LIBS.EXE+=$(LIBS.LOCAL.D3DDX5)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_OPEND3DDX5
endif
DESCRIPTION.$(D3DDX5) = $(DESCRIPTION.d3ddx5)
SRC.D3DDX5 = $(wildcard libs/cs3d/opengl/*.cpp) \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/dtmesh.cpp
OBJ.D3DDX5 = $(addprefix $(OUT),$(notdir $(SRC.D3DDX5:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: d3ddx5 d3ddx5clean

# Chain rules
clean: d3ddx5clean

d3ddx5: $(OUTDIRS) $(D3DDX5)

$(OUT)%$O: libs/cs3d/direct3d5/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.D3DDX5)
 
$(D3DDX5): $(OBJ.D3DDX5) $(DEP.D3DDX5)
	$(DO.PLUGIN) $(LIBS.D3DDX5)

d3ddx5clean:
	$(RM) $(D3DDX5) $(OBJ.D3DDX5)

ifdef DO_DEPEND
depend: $(OUTOS)d3ddx5.dep
$(OUTOS)d3ddx5.dep: $(SRC.D3DDX5)
	$(DO.DEP1) $(CFLAGS.D3DDX5) $(DO.DEP2)
else
-include $(OUTOS)d3ddx5.dep
endif

endif # ifeq ($(MAKESECTION),targets)
