# Application description
DESCRIPTION.unittest = Crystal Space unittest Tester

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

vpath %.cpp apps/tests/unittest

UNITTEST.EXE = unittest$(EXE.CONSOLE)
INC.UNITTEST = $(wildcard apps/tests/unittest/*.h)
SRC.UNITTEST = $(wildcard apps/tests/unittest/*.cpp)
OBJ.UNITTEST = $(addprefix $(OUT)/,$(notdir $(SRC.UNITTEST:.cpp=$O)))
DEP.UNITTEST = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.UNITTEST = $(foreach d,$(DEP.UNITTEST),$($d.LIB))

TO_INSTALL.EXE    += $(UNITTEST.EXE)
TO_INSTALL.DATA   += data/stdtex.zip

MSVC.DSP += UNITTEST
DSP.UNITTEST.NAME = unittest
DSP.UNITTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.unittest unittestclean

all: $(UNITTEST.EXE)
build.unittest: $(OUTDIRS) $(UNITTEST.EXE)
clean: unittestclean
check: unittestcheck

$(UNITTEST.EXE): $(DEP.EXE) $(OBJ.UNITTEST) $(LIB.UNITTEST)
	$(DO.LINK.CONSOLE.EXE)

unittestclean:
	-$(RMDIR) $(UNITTEST.EXE) $(OBJ.UNITTEST)

unittestcheck: $(UNITTEST.EXE)
	$(RUN_TEST)$(UNITTEST.EXE)

ifdef DO_DEPEND
dep: $(OUTOS)/unittest.dep
$(OUTOS)/unittest.dep: $(SRC.UNITTEST)
	$(DO.DEP)
else
-include $(OUTOS)/unittest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
