# Application description
DESCRIPTION.unittest = Crystal Space unittest tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make unittest     Make the $(DESCRIPTION.unittest)$"

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
INC.UNITTEST = $(wildcard $(DIR.UNITTEST)/*.h )
SRC.UNITTEST = $(wildcard $(DIR.UNITTEST)/*.cpp )
OBJ.UNITTEST = $(addprefix $(OUT.UNITTEST)/,$(notdir $(SRC.UNITTEST:.cpp=$O)))
DEP.UNITTEST = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.UNITTEST = $(foreach d,$(DEP.UNITTEST),$($d.LIB))

#TO_INSTALL.EXE += $(UNITTEST.EXE)

MSVC.DSP += UNITTEST
DSP.UNITTEST.NAME = unittest
DSP.UNITTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.unittest unittestclean unittestcleandep

all: $(UNITTEST.EXE)
build.unittest: $(OUT.UNITTEST) $(UNITTEST.EXE)
clean: unittestclean

$(OUT.UNITTEST)/%$O: $(DIR.UNITTEST)/%.cpp
	$(DO.COMPILE.CPP)
check: unittestcheck

$(UNITTEST.EXE): $(DEP.EXE) $(OBJ.UNITTEST) $(LIB.UNITTEST)
	$(DO.LINK.CONSOLE.EXE)

$(OUT.UNITTEST):
	$(MKDIRS)

unittestclean:
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
