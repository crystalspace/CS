DESCRIPTION.joytest = Joystick test application

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make joytest      Make the $(DESCRIPTION.joytest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: joytest joytestclean

all apps: joytest
joytest:
	$(MAKE_APP)
joytestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

JOYTEST.EXE = joytest$(EXE)
DIR.JOYTEST = apps/tests/joytest
OUT.JOYTEST = $(OUT)/$(DIR.JOYTEST)
INC.JOYTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYTEST)/*.h))
SRC.JOYTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYTEST)/*.cpp))
OBJ.JOYTEST = $(addprefix $(OUT.JOYTEST)/,$(notdir $(SRC.JOYTEST:.cpp=$O)))
DEP.JOYTEST = CSTOOL CSGFX CSGEOM CSUTIL
LIB.JOYTEST = $(foreach d,$(DEP.JOYTEST),$($d.LIB))

OUTDIRS += $(OUT.JOYTEST)

MSVC.DSP += JOYTEST
DSP.JOYTEST.NAME = joytest
DSP.JOYTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.joytest joytestclean joytestcleandep

build.joytest: $(OUTDIRS) $(JOYTEST.EXE)

$(OUT.JOYTEST)/%$O: $(SRCDIR)/$(DIR.JOYTEST)/%.cpp
	$(DO.COMPILE.CPP) $(JOYTEST.CFLAGS)

$(JOYTEST.EXE): $(DEP.EXE) $(OBJ.JOYTEST) $(LIB.JOYTEST)
	$(DO.LINK.EXE)

clean: joytestclean
joytestclean:
	-$(RMDIR) $(JOYTEST.EXE) $(OBJ.JOYTEST) joytest.txt

cleandep: joytestcleandep
joytestcleandep:
	-$(RM) $(OUT.JOYTEST)/joytest.dep

ifdef DO_DEPEND
dep: $(OUT.JOYTEST) $(OUT.JOYTEST)/joytest.dep
$(OUT.JOYTEST)/joytest.dep: $(SRC.JOYTEST)
	$(DO.DEPEND)
else
-include $(OUT.JOYTEST)/joytest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
