# Application description
DESCRIPTION.wstest = Crystal Space Windowing System test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make wstest       Make the $(DESCRIPTION.wstest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: wstest wstestclean

all apps: wstest
wstest:
	$(MAKE_TARGET)
wstestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/cswstest apps/support

CSWSTEST.EXE = cswstest$(EXE)
INC.CSWSTEST = $(wildcard apps/cswstest/*.h)
SRC.CSWSTEST = $(wildcard apps/cswstest/*.cpp)
OBJ.CSWSTEST = $(addprefix $(OUT),$(notdir $(SRC.CSWSTEST:.cpp=$O)))
DEP.CSWSTEST = CSWS CSGEOM CSSYS CSGEOM CSUTIL CSFX
LIB.CSWSTEST = $(foreach d,$(DEP.CSWSTEST),$($d.LIB))
CFG.CSWSTEST = data/config/cswstest.cfg

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += CSWSTEST
DSP.CSWSTEST.NAME = cswstest
DSP.CSWSTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: wstest wstestclean

all: $(CSWSTEST.EXE)
wstest: $(OUTDIRS) $(CSWSTEST.EXE)
clean: wstestclean

$(CSWSTEST.EXE): $(DEP.EXE) $(OBJ.CSWSTEST) $(LIB.CSWSTEST)
	$(DO.LINK.EXE)

wstestclean:
	-$(RM) $(CSWSTEST.EXE) $(OBJ.CSWSTEST)

ifdef DO_DEPEND
dep: $(OUTOS)cswstest.dep
$(OUTOS)cswstest.dep: $(SRC.CSWSTEST)
	$(DO.DEP)
else
-include $(OUTOS)cswstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
