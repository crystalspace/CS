# Application description
DESCRIPTION.wstest = Crystal Space Windowing System test

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make wstest       Make the $(DESCRIPTION.wstest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: wstest

all apps: wstest
wstest:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/cswstest apps/support

CSWSTEST.EXE=cswstest$(EXE)
SRC.CSWSTEST = $(wildcard apps/cswstest/*.cpp)
OBJ.CSWSTEST = $(addprefix $(OUT),$(notdir $(SRC.CSWSTEST:.cpp=$O)))
DESCRIPTION.$(CSWSTEST.EXE) = $(DESCRIPTION.wstest)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: wstest wstestclean

all: $(CSWSTEST.EXE)
wstest: $(OUTDIRS) $(CSWSTEST.EXE)
clean: wstestclean

$(CSWSTEST.EXE): $(DEP.EXE) $(OBJ.CSWSTEST) $(CSWS.LIB) $(CSENGINE.LIB) \
  $(CSTERR.LIB) $(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSSFXLDR.LIB) $(CSCOM.LIB) \
  $(CSSYS.LIB) $(CSINPUT.LIB) $(CSOBJECT.LIB) $(CSUTIL.LIB)
	$(DO.LINK.EXE)

wstestclean:
	-$(RM) $(CSWSTEST.EXE)

ifdef DO_DEPEND
depend: $(OUTOS)cswstest.dep
$(OUTOS)cswstest.dep: $(SRC.CSWSTEST)
	$(DO.DEP)
else
-include $(OUTOS)cswstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
