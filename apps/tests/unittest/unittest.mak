# Application description
DESCRIPTION.unittest = Crystal Space unittest Tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make unittest     Make the $(DESCRIPTION.unittest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: unittest unitclean

all apps: unittest
unittest:
	$(MAKE_APP)
unitclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/unittest

UNITTEST.EXE = unittest$(EXE)
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

#$(UNITTEST.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.unittest unitclean

all: $(UNITTEST.EXE)
build.unittest: $(OUTDIRS) $(UNITTEST.EXE)
clean: unitclean

$(UNITTEST.EXE): $(DEP.EXE) $(OBJ.UNITTEST) $(LIB.UNITTEST)
	$(DO.LINK.EXE)

unitclean:
	-$(RM) $(UNITTEST.EXE) $(OBJ.UNITTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/unittest.dep
$(OUTOS)/unittest.dep: $(SRC.UNITTEST)
	$(DO.DEP)
else
-include $(OUTOS)/unittest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
