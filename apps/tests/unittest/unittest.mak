# Application description
DESCRIPTION.unittest = Crystal Space unit tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make unittest     Make the $(DESCRIPTION.unittest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: unittest unittestclean

all apps: unittest
check: unittestcheck
unittest:
	$(MAKE_APP)
unittestclean:
	$(MAKE_CLEAN)
unittestcheck:
	$(MAKE_CHECK)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


UNITTEST.EXE = unittest$(EXE.CONSOLE)
DIR.UNITTEST = apps/tests/unittest
OUT.UNITTEST = $(OUT)/$(DIR.UNITTEST)
INC.UNITTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.UNITTEST)/*.h ))
SRC.UNITTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.UNITTEST)/*.cpp ))
OBJ.UNITTEST = $(addprefix $(OUT.UNITTEST)/,$(notdir $(SRC.UNITTEST:.cpp=$O)))
DEP.UNITTEST = CSTOOL CSGEOM CSTOOL CSGFX CSUTIL CSUTIL
LIB.UNITTEST = $(foreach d,$(DEP.UNITTEST),$($d.LIB))

OUTDIRS += $(OUT.UNITTEST)

#TO_INSTALL.EXE += $(UNITTEST.EXE)

MSVC.DSP += UNITTEST
DSP.UNITTEST.NAME = unittest
DSP.UNITTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.unittest unittestclean unittestcleandep

all: $(UNITTEST.EXE)
build.unittest: $(OUTDIRS) $(UNITTEST.EXE)
clean: unittestclean
check: unittestcheck

$(OUT.UNITTEST)/%$O: $(SRCDIR)/$(DIR.UNITTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(UNITTEST.EXE): $(DEP.EXE) $(OBJ.UNITTEST) $(LIB.UNITTEST)
	$(DO.LINK.CONSOLE.EXE)

unittestclean:
	-$(RM) unittest.txt
	-$(RMDIR) $(UNITTEST.EXE) $(OBJ.UNITTEST)

cleandep: unittestcleandep
unittestcleandep:
	-$(RM) $(OUT.UNITTEST)/unittest.dep

unittestcheck: $(UNITTEST.EXE)
	$(RUN_TEST)$(UNITTEST.EXE)

ifdef DO_DEPEND
dep: $(OUT.UNITTEST) $(OUT.UNITTEST)/unittest.dep
$(OUT.UNITTEST)/unittest.dep: $(SRC.UNITTEST)
	$(DO.DEPEND)
else
-include $(OUT.UNITTEST)/unittest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
