# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring net/vos,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.vostest = Virtual Object System test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make vostest      Make the $(DESCRIPTION.vostest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vostest vostestclean

all apps: vostest
vostest:
	$(MAKE_APP)
vostestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

VOSTEST.CFLAGS = \
  $(CFLAGS.D)_REENTRANT $(CXXFLAGS.EXCEPTIONS.ENABLE) $(VOS.CFLAGS)

VOSTEST.EXE = vostest$(EXE)
DIR.VOSTEST = apps/tests/vostest
OUT.VOSTEST = $(OUT)/$(DIR.VOSTEST)
INC.VOSTEST = $(wildcard $(SRCDIR)/$(DIR.VOSTEST)/*.h)
SRC.VOSTEST = $(wildcard $(SRCDIR)/$(DIR.VOSTEST)/*.cpp)
OBJ.VOSTEST = $(addprefix $(OUT.VOSTEST)/,$(notdir $(SRC.VOSTEST:.cpp=$O)))
DEP.VOSTEST = CSTOOL CSGFX CSGEOM CSUTIL
LIB.VOSTEST = $(foreach d,$(DEP.VOSTEST),$($d.LIB))
LIB.VOSTEST.LOCAL = $(VOS.LFLAGS)

OUTDIRS += $(OUT.VOSTEST)

#TO_INSTALL.EXE += $(VOSTEST.EXE)

MSVC.DSP += VOSTEST
DSP.VOSTEST.NAME = vostest
DSP.VOSTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.vostest vostestclean vostestcleandep

build.vostest: $(OUTDIRS) $(VOSTEST.EXE)
clean: vostestclean

$(OUT.VOSTEST)/%$O: $(SRCDIR)/$(DIR.VOSTEST)/%.cpp
	$(DO.COMPILE.CPP) $(VOSTEST.CFLAGS)

$(VOSTEST.EXE): $(DEP.EXE) $(OBJ.VOSTEST) $(LIB.VOSTEST)
	$(DO.LINK.EXE) $(LIB.VOSTEST.LOCAL)

vostestclean:
	-$(RM) vostest.txt
	-$(RMDIR) $(VOSTEST.EXE) $(OBJ.VOSTEST)

cleandep: vostestcleandep
vostestcleandep:
	-$(RM) $(OUT.VOSTEST)/vostest.dep

ifdef DO_DEPEND
dep: $(OUT.VOSTEST) $(OUT.VOSTEST)/vostest.dep
$(OUT.VOSTEST)/vostest.dep: $(SRC.VOSTEST)
	$(DO.DEPEND1) $(VOSTEST.CFLAGS) $(DO.DEPEND2)
else
-include $(OUT.VOSTEST)/vostest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring net/vos,$(PLUGINS) $(PLUGINS.DYNAMIC)))
