# Application description
DESCRIPTION.walk = Crystal Space WalkTest demo executable

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make walk         Make the $(DESCRIPTION.walk)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: walk

all apps: walk
walk:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/walktest apps/support

WALKTEST.EXE=walktest$(EXE)
SRC.WALKTEST = $(wildcard apps/walktest/*.cpp) \
  apps/support/static.cpp apps/support/cspace.cpp apps/support/command.cpp
OBJ.WALKTEST = $(addprefix $(OUT),$(notdir $(SRC.WALKTEST:.cpp=$O)))
DESCRIPTION.$(WALKTEST.EXE) = $(DESCRIPTION.walk)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: walk walktestclean

all: $(WALKTEST.EXE)
walk: $(OUTDIRS) $(WALKTEST.EXE)
clean: walkclean

$(WALKTEST.EXE): $(DEP.EXE) $(OBJ.WALKTEST) \
  $(CSPARSER.LIB) $(CSENGINE.LIB) \
  $(CSGEOM.LIB) $(CSSNDLDR.LIB) $(CSGFXLDR.LIB) \
  $(CSUTIL.LIB) $(CSCOM.LIB) $(CSSYS.LIB) $(CSINPUT.LIB) $(CSOBJECT.LIB)
	$(DO.LINK.EXE)

walkclean:
	-$(RM) $(WALKTEST.EXE)

ifdef DO_DEPEND
$(OUTOS)walktest.dep: $(SRC.WALKTEST)
	$(DO.DEP)
endif

-include $(OUTOS)walktest.dep

endif # ifeq ($(MAKESECTION),targets)
