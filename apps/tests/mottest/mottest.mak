# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.mottest = Motion Manager Test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make mottest      Make the $(DESCRIPTION.mottest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mottest mottestclean

all apps: mottest
mottest:
	$(MAKE_APP)
mottestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/mottest apps/support

MOTTEST.EXE = mottest$(EXE)
INC.MOTTEST = $(wildcard apps/tests/mottest/*.h)
SRC.MOTTEST = $(wildcard apps/tests/mottest/*.cpp)
OBJ.MOTTEST = $(addprefix $(OUT)/,$(notdir $(SRC.MOTTEST:.cpp=$O)))
DEP.MOTTEST = CSTOOL CSUTIL CSSYS CSUTIL CSGEOM CSGFX
LIB.MOTTEST = $(foreach d,$(DEP.MOTTEST),$($d.LIB))
CFG.MOTTEST = data/config/mottest.cfg

#TO_INSTALL.EXE    += $(MOTTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.MOTTEST)

MSVC.DSP += MOTTEST
DSP.MOTTEST.NAME = mottest
DSP.MOTTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.mottest mottestclean

all: $(MOTTEST.EXE)
build.mottest: $(OUTDIRS) $(MOTTEST.EXE)
clean: mottestclean

$(MOTTEST.EXE): $(DEP.EXE) $(OBJ.MOTTEST) $(LIB.MOTTEST)
	$(DO.LINK.EXE)

mottestclean:
	-$(RMDIR) $(MOTTEST.EXE) $(OBJ.MOTTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/mottest.dep
$(OUTOS)/mottest.dep: $(SRC.MOTTEST)
	$(DO.DEP)
else
-include $(OUTOS)/mottest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))
