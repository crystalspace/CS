# Application description
DESCRIPTION.vshell = Crystal Space Virtual Shell tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make vshell       Make the $(DESCRIPTION.vshell)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vshell vshellclean

all apps: vshell
vshell:
	$(MAKE_TARGET)
vshellclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/vsh

VSH.EXE = vsh$(EXE)
INC.VSH =
SRC.VSH = apps/tools/vsh/vsh.cpp
OBJ.VSH = $(addprefix $(OUT),$(notdir $(SRC.VSH:.cpp=$O)))
DEP.VSH = CSUTIL CSGFX CSTOOL CSSYS CSUTIL CSSYS CSGEOM
LIB.VSH = $(foreach d,$(DEP.VSH),$($d.LIB))

TO_INSTALL.EXE += $(VSH.EXE)

MSVC.DSP += VSH
DSP.VSH.NAME = vsh
DSP.VSH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vshell vshellclean

all: $(VSH.EXE)
vshell: $(OUTDIRS) $(VSH.EXE)
clean: vshellclean

$(VSH.EXE): $(DEP.EXE) $(OBJ.VSH) $(LIB.VSH)
	$(DO.LINK.CONSOLE.EXE)

vshellclean:
	-$(RM) $(VSH.EXE) $(OBJ.VSH)

ifdef DO_DEPEND
dep: $(OUTOS)vshell.dep
$(OUTOS)vshell.dep: $(SRC.VSH)
	$(DO.DEP)
else
-include $(OUTOS)vshell.dep
endif

endif # ifeq ($(MAKESECTION),targets)
