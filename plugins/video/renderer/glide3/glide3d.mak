# This is a subinclude file used to define the rules needed
# to build the Glide2 3D driver -- glide3d Version 3

# Driver description
DESCRIPTION.glide33 = Crystal Space Glide V3 3D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make glide33      Make the $(DESCRIPTION.glide33)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glide33

all plugins drivers drivers33: glide33

glide33:
	$(MAKE_TARGET) MAKE_DLL=yes
glide33clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.GLIDE33+= $(GLIDE3_PATH) -DGLIDE3 
LIBS._GLIDE33+=-lglide3x

# The Glide driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDE33=glide33d$(DLL)
  LIBS.GLIDE33=$(LIBS._GLIDE33)
  DEP.GLIDE33=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GLIDE33=$(OUT)$(LIB_PREFIX)glide33d$(LIB)
  DEP.EXE+=$(GLIDE33)
  LIBS.EXE+=$(LIBS._GLIDE33)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLIDE33D
endif
DESCRIPTION.$(GLIDE33) = $(DESCRIPTION.glide33)
SRC.GLIDE33 = $(wildcard libs/cs3d/glide3/*.cpp ) \
  libs/cs3d/common/texmem.cpp libs/cs3d/common/txtmgr.cpp \
  libs/cs3d/common/dtmesh.cpp libs/cs3d/common/dpmesh.cpp
OBJ.GLIDE33 = $(addprefix $(OUT),$(notdir $(SRC.GLIDE33:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glide33 glide33clean

# Chain rules
clean: glide33clean

glide33: $(OUTDIRS) $(GLIDE33)

$(OUT)%$O: libs/cs3d/glide3/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDE33)

$(GLIDE33): $(OBJ.GLIDE33) $(DEP.GLIDE33)
	$(DO.PLUGIN) $(LIBS.GLIDE33)

glide33clean:
	$(RM) $(GLIDE33) $(OBJ.GLIDE33)

ifdef DO_DEPEND
dep: $(OUTOS)glide33.dep
$(OUTOS)glide33.dep: $(SRC.GLIDE33)
	$(DO.DEP1) $(CFLAGS.GLIDE33) $(DO.DEP2)
else
-include $(OUTOS)glide33.dep
endif

endif # ifeq ($(MAKESECTION),targets)
