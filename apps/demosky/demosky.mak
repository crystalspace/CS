# Application description
DESCRIPTION.demosky = Crystal Space sky demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make demosky      Make the $(DESCRIPTION.demosky)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demosky demoskyclean

all apps: demosky
demosky:
	$(MAKE_APP)
demoskyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DEMOSKY.EXE=demosky$(EXE)
DIR.DEMOSKY = apps/demosky
OUT.DEMOSKY = $(OUT)/$(DIR.DEMOSKY)
INC.DEMOSKY = $(wildcard $(DIR.DEMOSKY)/*.h)
SRC.DEMOSKY = $(wildcard $(DIR.DEMOSKY)/*.cpp)
OBJ.DEMOSKY = $(addprefix $(OUT.DEMOSKY)/,$(notdir $(SRC.DEMOSKY:.cpp=$O)))
DEP.DEMOSKY = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.DEMOSKY = $(foreach d,$(DEP.DEMOSKY),$($d.LIB))

#TO_INSTALL.EXE += $(DEMOSKY.EXE)

MSVC.DSP += DEMOSKY
DSP.DEMOSKY.NAME = demosky
DSP.DEMOSKY.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.demosky demoskyclean demoskycleandep

all: $(DEMOSKY.EXE)
build.demosky: $(OUT.DEMOSKY) $(DEMOSKY.EXE)
clean: demoskyclean

$(OUT.DEMOSKY)/%$O: $(DIR.DEMOSKY)/%.cpp
	$(DO.COMPILE.CPP)

$(DEMOSKY.EXE): $(DEP.EXE) $(OBJ.DEMOSKY) $(LIB.DEMOSKY)
	$(DO.LINK.EXE)

$(OUT.DEMOSKY):
	$(MKDIRS)

demoskyclean:
	-$(RMDIR) $(DEMOSKY.EXE) $(OBJ.DEMOSKY)

cleandep: demoskycleandep
demoskycleandep:
	-$(RM) $(OUT.DEMOSKY)/demosky.dep

ifdef DO_DEPEND
dep: $(OUT.DEMOSKY) $(OUT.DEMOSKY)/demosky.dep
$(OUT.DEMOSKY)/demosky.dep: $(SRC.DEMOSKY)
	$(DO.DEPEND)
else
-include $(OUT.DEMOSKY)/demosky.dep
endif

endif # ifeq ($(MAKESECTION),targets)
