# This is a subinclude file used to define the rules needed
# to build the Glide2 3D driver -- glide3d

# Driver description
DESCRIPTION.glide3d = Crystal Space Glide 3D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make glide3d      Make the $(DESCRIPTION.glide3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glide3d

all plugins drivers drivers3d: glide3d

glide3d:
	$(MAKE_TARGET) MAKE_DLL=yes
glide3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.GLIDE3D+=-I/usr/local/glide/include -DDO_GLIDE -DGLIDE24_ONLY
LIBS._GLIDE3D+=-lglide2x

# The Glide driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDE3D=glide23d$(DLL)
  LIBS.GLIDE3D=$(LIBS._GLIDE3D)
  DEP.GLIDE3D=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GLIDE3D=$(OUT)$(LIB_PREFIX)glide23d$(LIB)
  DEP.EXE+=$(GLIDE3D)
  LIBS.EXE+=$(LIBS._GLIDE3D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLIDE3D
endif
DESCRIPTION.$(GLIDE3D) = $(DESCRIPTION.glide3d)
SRC.GLIDE3D = $(wildcard libs/cs3d/glide2/*.cpp) \
  libs/cs3d/common/texmem.cpp libs/cs3d/common/txtmgr.cpp \
  libs/cs3d/common/memheap.cpp
OBJ.GLIDE3D = $(addprefix $(OUT),$(notdir $(SRC.GLIDE3D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glide3d glide3dclean

# Chain rules
clean: glide3dclean

glide3d: $(OUTDIRS) $(GLIDE3D)

$(OUT)%$O: libs/cs3d/glide2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDE3D)

$(GLIDE3D): $(OBJ.GLIDE3D) $(DEP.GLIDE3D)
	$(DO.PLUGIN) $(LIBS.GLIDE3D)

glide3dclean:
	$(RM) $(GLIDE3D) $(OBJ.GLIDE3D)

ifdef DO_DEPEND
depend: $(OUTOS)glide3d.dep
$(OUTOS)glide3d.dep: $(SRC.GLIDE3D)
	$(DO.DEP)
else
-include $(OUTOS)glide3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
