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

vpath %.cpp apps/bumptest apps/support

BUMPTEST.EXE=bumptest$(EXE)
INC.BUMPTEST = $(wildcard apps/bumptest/*.h)
SRC.BUMPTEST = $(wildcard apps/bumptest/*.cpp)
OBJ.BUMPTEST = $(addprefix $(OUT)/,$(notdir $(SRC.BUMPTEST:.cpp=$O)))
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

.PHONY: build.bumptest bumptestclean

all: $(BUMPTEST.EXE)
build.bumptest: $(OUTDIRS) $(BUMPTEST.EXE)
clean: bumptestclean

$(BUMPTEST.EXE): $(DEP.EXE) $(OBJ.BUMPTEST) $(LIB.BUMPTEST)
	$(DO.LINK.EXE)

bumptestclean:
	-$(RM) $(BUMPTEST.EXE) $(OBJ.BUMPTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/bumptest.dep
$(OUTOS)/bumptest.dep: $(SRC.BUMPTEST)
	$(DO.DEP)
else
-include $(OUTOS)/bumptest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
