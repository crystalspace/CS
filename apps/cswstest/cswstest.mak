# Application description
DESCRIPTION.cswstest = Crystal Space Windowing System test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make cswstest     Make the $(DESCRIPTION.cswstest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cswstest cswstestclean

all apps: cswstest
cswstest:
	$(MAKE_APP)
cswstestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/cswstest apps/support

CSWSTEST.EXE = cswstest$(EXE)
INC.CSWSTEST = $(wildcard apps/cswstest/*.h)
SRC.CSWSTEST = $(wildcard apps/cswstest/*.cpp)
OBJ.CSWSTEST = $(addprefix $(OUT)/,$(notdir $(SRC.CSWSTEST:.cpp=$O)))
DEP.CSWSTEST = CSWS CSGFX CSGEOM CSSYS CSGEOM CSUTIL CSTOOL CSUTIL CSSYS CSUTIL
LIB.CSWSTEST = $(foreach d,$(DEP.CSWSTEST),$($d.LIB))
CFG.CSWSTEST = data/config/cswstest.cfg

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += CSWSTEST
DSP.CSWSTEST.NAME = cswstest
DSP.CSWSTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cswstest cswstestclean

all: $(CSWSTEST.EXE)
build.cswstest: $(OUTDIRS) $(CSWSTEST.EXE)
clean: cswstestclean

$(CSWSTEST.EXE): $(DEP.EXE) $(OBJ.CSWSTEST) $(LIB.CSWSTEST)
	$(DO.LINK.EXE)

cswstestclean:
	-$(RM) $(CSWSTEST.EXE) $(OBJ.CSWSTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/cswstest.dep
$(OUTOS)/cswstest.dep: $(SRC.CSWSTEST)
	$(DO.DEP)
else
-include $(OUTOS)/cswstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
