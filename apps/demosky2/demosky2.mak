# Application description
DESCRIPTION.demosky2 = Crystal Space sky demo 2

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make demosky2     Make the $(DESCRIPTION.demosky2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demosky2 demosky2clean

all apps: demosky2
demosky2:
	$(MAKE_APP)
demosky2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DEMOSKY2.EXE=demosky2$(EXE)
DIR.DEMOSKY2 = apps/demosky2
OUT.DEMOSKY2 = $(OUT)/$(DIR.DEMOSKY2)
INC.DEMOSKY2 = $(wildcard $(DIR.DEMOSKY2)/*.h)
SRC.DEMOSKY2 = $(wildcard $(DIR.DEMOSKY2)/*.cpp)
OBJ.DEMOSKY2 = $(addprefix $(OUT.DEMOSKY2)/,$(notdir $(SRC.DEMOSKY2:.cpp=$O)))
DEP.DEMOSKY2 = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.DEMOSKY2 = $(foreach d,$(DEP.DEMOSKY2),$($d.LIB))

#TO_INSTALL.EXE += $(DEMOSKY2.EXE)

MSVC.DSP += DEMOSKY2
DSP.DEMOSKY2.NAME = demosky2
DSP.DEMOSKY2.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.demosky2 demosky2clean demosky2cleandep

all: $(DEMOSKY2.EXE)
build.demosky2: $(OUT.DEMOSKY2) $(DEMOSKY2.EXE)
clean: demosky2clean

$(OUT.DEMOSKY2)/%$O: $(DIR.DEMOSKY2)/%.cpp
	$(DO.COMPILE.CPP)

$(DEMOSKY2.EXE): $(DEP.EXE) $(OBJ.DEMOSKY2) $(LIB.DEMOSKY2)
	$(DO.LINK.EXE)

$(OUT.DEMOSKY2):
	$(MKDIRS)

demosky2clean:
	-$(RMDIR) $(DEMOSKY2.EXE) $(OBJ.DEMOSKY2)

cleandep: demosky2cleandep
demosky2cleandep:
	-$(RM) $(OUT.DEMOSKY2)/demosky2.dep

ifdef DO_DEPEND
dep: $(OUT.DEMOSKY2) $(OUT.DEMOSKY2)/demosky2.dep
$(OUT.DEMOSKY2)/demosky2.dep: $(SRC.DEMOSKY2)
	$(DO.DEPEND)
else
-include $(OUT.DEMOSKY2)/demosky2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
