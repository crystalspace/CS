# Application description
DESCRIPTION.uninstexe = Uninstall program

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make uninstexe    Make the $(DESCRIPTION.uninstexe)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: uninstexe uninstexeclean

uninstexe:
	$(MAKE_TARGET)
uninstexeclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/uninst

UNINSTEXE.EXE = uninst$(EXE)
INC.UNINSTEXE =
SRC.UNINSTEXE = apps/tools/uninst/uninst.cpp
OBJ.UNINSTEXE = $(addprefix $(OUT),$(notdir $(SRC.UNINSTEXE:.cpp=$O)))
DEP.UNINSTEXE =
LIB.UNINSTEXE =

# Uninstall program is installed in the CS root rather than CS/bin.
TO_INSTALL.ROOT += $(UNINSTEXE.EXE)

#MSVC.DSP += UNINSTEXE
#DSP.UNINSTEXE.NAME = uninst
#DSP.UNINSTEXE.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: uninstexe uninstexeclean

all apps: uninstexe 
uninstexe: $(OUTDIRS) $(UNINSTEXE.EXE)
clean: uninstexeclean

$(UNINSTEXE.EXE): $(OBJ.UNINSTEXE) $(LIB.UNINSTEXE)
	$(DO.LINK.CONSOLE.EXE)

uninstexeclean:
	-$(RM) $(UNINSTEXE.EXE) $(OBJ.UNINSTEXE)

ifdef DO_DEPEND
dep: $(OUTOS)uninstexe.dep
$(OUTOS)uninstexe.dep: $(SRC.UNINSTEXE)
	$(DO.DEP)
else
-include $(OUTOS)uninstexe.dep
endif

endif # ifeq ($(MAKESECTION),targets)
