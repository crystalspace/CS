# Application description
DESCRIPTION.gfxtest = Crystal Space image manipulator

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make gfxtest      Make the $(DESCRIPTION.gfxtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gfxtest gfxtestclean

all apps: gfxtest
gfxtest:
	$(MAKE_APP)
gfxtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

GFXTEST.EXE = gfxtest$(EXE.CONSOLE)
DIR.GFXTEST = apps/tests/gfxtst
OUT.GFXTEST = $(OUT)/$(DIR.GFXTEST)
INC.GFXTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GFXTEST)/*.h))
SRC.GFXTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GFXTEST)/*.cpp))
OBJ.GFXTEST = $(addprefix $(OUT.GFXTEST)/,$(notdir $(SRC.GFXTEST:.cpp=$O)))
DEP.GFXTEST = CSGFX CSTOOL CSUTIL CSGEOM CSUTIL
LIB.GFXTEST = $(foreach d,$(DEP.GFXTEST),$($d.LIB))

OUTDIRS += $(OUT.GFXTEST)

#TO_INSTALL.EXE += $(GFXTEST.EXE)

MSVC.DSP += GFXTEST
DSP.GFXTEST.NAME = gfxtest
DSP.GFXTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.gfxtest gfxtestclean gfxtestcleandep

build.gfxtest: $(OUTDIRS) $(GFXTEST.EXE)
clean: gfxtestclean

$(OUT.GFXTEST)/%$O: $(SRCDIR)/$(DIR.GFXTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(GFXTEST.EXE): $(DEP.EXE) $(OBJ.GFXTEST) $(LIB.GFXTEST)
	$(DO.LINK.CONSOLE.EXE)

gfxtestclean:
	-$(RM) gfxtest.txt
	-$(RMDIR) $(GFXTEST.EXE) $(OBJ.GFXTEST)

cleandep: gfxtestcleandep
gfxtestcleandep:
	-$(RM) $(OUT.GFXTEST)/gfxtest.dep

ifdef DO_DEPEND
dep: $(OUT.GFXTEST) $(OUT.GFXTEST)/gfxtest.dep
$(OUT.GFXTEST)/gfxtest.dep: $(SRC.GFXTEST)
	$(DO.DEPEND)
else
-include $(OUT.GFXTEST)/gfxtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
