# This is a subinclude file used to define the rules needed
# to build the Glide Displaydriver for GLX 2D driver -- oglglide

# Driver description
DESCRIPTION.oglglide = Crystal Space Glide GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oglglide     Make the $(DESCRIPTION.oglglide)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglglide oglglideclean
all plugins glxdisp: oglglide

oglglide:
	$(MAKE_TARGET) MAKE_DLL=yes

oglglideclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.OGLGLIDE += -I/usr/include/glide -I/usr/local/glide/include
LIB.OGLGLIDE.SYSTEM += -lglide2x

ifeq ($(USE_PLUGINS),yes)
  OGLGLIDE = $(OUTDLL)oglglide$(DLL)
  LIB.OGLGLIDE = $(foreach d,$(DEP.OGLGLIDE),$($d.LIB))
  LIB.OGLGLIDE.SPECIAL = $(LIB.OGLGLIDE.SYSTEM)
else
  OGLGLIDE = $(OUT)$(LIB_PREFIX)oglglide$(LIB)
  DEP.EXE += $(OGLGLIDE)
  LIBS.EXE += $(LIB.OGLGLIDE.SYSTEM) $(CSUTIL.LIB) $(CSSYS.LIB)
  SCF.STATIC += oglglide
endif

INC.OGLGLIDE = $(wildcard plugins/video/canvas/openglx/glide/*.h)
SRC.OGLGLIDE = $(wildcard plugins/video/canvas/openglx/glide/*.cpp)
OBJ.OGLGLIDE = $(addprefix $(OUT),$(notdir $(SRC.OGLGLIDE:.cpp=$O)))
DEP.OGLGLIDE = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglglide oglglideclean

oglglide: $(OUTDIRS) $(OGLGLIDE)

$(OUT)%$O: plugins/video/canvas/openglx/glide/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.OGLGLIDE)

$(OGLGLIDE): $(OBJ.OGLGLIDE) $(LIB.OGLGLIDE)
	$(DO.PLUGIN) $(LIB.OGLGLIDE.SPECIAL)

clean: oglglideclean
oglglideclean:
	$(RM) $(OGLGLIDE) $(OBJ.OGLGLIDE)

ifdef DO_DEPEND
dep: $(OUTOS)oglglide.dep
$(OUTOS)oglglide.dep: $(SRC.OGLGLIDE)
	$(DO.DEP1) $(CFLAGS.OGLGLIDE) $(DO.DEP2)
else
-include $(OUTOS)oglglide.dep
endif

endif # ifeq ($(MAKESECTION),targets)
