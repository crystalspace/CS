# It has sense only for dynamicaly-linked libraries
ifneq ($(USE_PLUGINS),no)

# Application description
DESCRIPTION.scfreg = Crystal Space SCF registration server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make scfreg       Make the $(DESCRIPTION.scfreg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: scfreg scfregclean

all apps: scfreg
scfreg:
	$(MAKE_APP)
scfregclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SCFREG.EXE = scfreg$(EXE.CONSOLE)
DIR.SCFREG = apps/tools/scfreg
OUT.SCFREG = $(OUT)/$(DIR.SCFREG)
INC.SCFREG =
SRC.SCFREG = apps/tools/scfreg/scfreg.cpp
OBJ.SCFREG = $(addprefix $(OUT.SCFREG)/,$(notdir $(SRC.SCFREG:.cpp=$O)))
DEP.SCFREG = CSSYS CSUTIL CSGEOM
LIB.SCFREG = $(foreach d,$(DEP.SCFREG),$($d.LIB))

OUTDIRS += $(OUT.SCFREG)

TO_INSTALL.EXE += $(SCFREG.EXE)

MSVC.DSP += SCFREG
DSP.SCFREG.NAME = scfreg
DSP.SCFREG.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.scfreg scfregclean scfregcleandep

build.scfreg: $(OUTDIRS) $(SCFREG.EXE)
clean: scfregclean

$(OUT.SCFREG)/%$O: $(DIR.SCFREG)/%.cpp
	$(DO.COMPILE.CPP)

$(SCFREG.EXE): $(OBJ.SCFREG) $(LIB.SCFREG)
	$(DO.LINK.CONSOLE.EXE)

scfregclean:
	-$(RM) scfreg.txt
	-$(RMDIR) $(SCFREG.EXE) $(OBJ.SCFREG)

cleandep: scfregcleandep
scfregcleandep:
	-$(RM) $(OUT.SCFREG)/scfreg.dep

ifdef DO_DEPEND
dep: $(OUT.SCFREG) $(OUT.SCFREG)/scfreg.dep
$(OUT.SCFREG)/scfreg.dep: $(SRC.SCFREG)
	$(DO.DEPEND)
else
-include $(OUT.SCFREG)/scfreg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq ($(USE_PLUGINS),no)
