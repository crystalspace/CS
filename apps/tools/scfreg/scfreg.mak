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

.PHONY: scfreg scfrclean

all apps: scfreg
scfreg:
	$(MAKE_APP)
scfrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/scfreg

SCFREG.EXE = scfreg$(EXE)
INC.SCFREG =
SRC.SCFREG = apps/tools/scfreg/scfreg.cpp
OBJ.SCFREG = $(addprefix $(OUT)/,$(notdir $(SRC.SCFREG:.cpp=$O)))
DEP.SCFREG = CSSYS CSUTIL CSGEOM
LIB.SCFREG = $(foreach d,$(DEP.SCFREG),$($d.LIB))

TO_INSTALL.EXE += $(SCFREG.EXE)

MSVC.DSP += SCFREG
DSP.SCFREG.NAME = scfreg
DSP.SCFREG.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.scfreg scfrclean

all: $(SCFREG.EXE)
build.scfreg: $(OUTDIRS) $(SCFREG.EXE)
clean: scfrclean

$(SCFREG.EXE): $(OBJ.SCFREG) $(LIB.SCFREG)
	$(DO.LINK.CONSOLE.EXE)

scfrclean:
	-$(RM) $(SCFREG.EXE) $(OBJ.SCFREG)

ifdef DO_DEPEND
dep: $(OUTOS)/scfreg.dep
$(OUTOS)/scfreg.dep: $(SRC.SCFREG)
	$(DO.DEP)
else
-include $(OUTOS)/scfreg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq ($(USE_PLUGINS),no)
