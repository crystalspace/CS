# Application description
DESCRIPTION.bumptest = Crystal Space bumpmap test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make bumptest     Make the $(DESCRIPTION.bumptest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bumptest bumptestclean

all apps: bumptest
bumptest:
	$(MAKE_APP)
bumptestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

BUMPTEST.EXE=bumptest$(EXE)
DIR.BUMPTEST = apps/bumptest
OUT.BUMPTEST = $(OUT)/$(DIR.BUMPTEST)
INC.BUMPTEST = $(wildcard $(DIR.BUMPTEST)/*.h)
SRC.BUMPTEST = $(wildcard $(DIR.BUMPTEST)/*.cpp)
OBJ.BUMPTEST = $(addprefix $(OUT.BUMPTEST)/,$(notdir $(SRC.BUMPTEST:.cpp=$O)))
DEP.BUMPTEST = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.BUMPTEST = $(foreach d,$(DEP.BUMPTEST),$($d.LIB))
CFG.BUMPTEST = data/config/csbumptest.cfg

#TO_INSTALL.EXE += $(BUMPTEST.EXE)

MSVC.DSP += BUMPTEST
DSP.BUMPTEST.NAME = bumptest
DSP.BUMPTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.bumptest bumptestclean bumptestcleandep

all: $(BUMPTEST.EXE)
build.bumptest: $(OUT.BUMPTEST) $(BUMPTEST.EXE)
clean: bumptestclean

$(OUT.BUMPTEST)/%$O: $(DIR.BUMPTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(BUMPTEST.EXE): $(DEP.EXE) $(OBJ.BUMPTEST) $(LIB.BUMPTEST)
	$(DO.LINK.EXE)

$(OUT.BUMPTEST):
	$(MKDIRS)

bumptestclean:
	-$(RMDIR) $(BUMPTEST.EXE) $(OBJ.BUMPTEST)

cleandep: bumptestcleandep
bumptestcleandep:
	-$(RM) $(OUT.BUMPTEST)/bumptest.dep

ifdef DO_DEPEND
dep: $(OUT.BUMPTEST) $(OUT.BUMPTEST)/bumptest.dep
$(OUT.BUMPTEST)/bumptest.dep: $(SRC.BUMPTEST)
	$(DO.DEPEND)
else
-include $(OUT.BUMPTEST)/bumptest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
