# It has sense only for dynamicaly-linked libraries
ifneq ($(USE_PLUGINS),no)

# Application description
DESCRIPTION.scfr = Crystal Space SCF registration server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make scfr         Make the $(DESCRIPTION.scfr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: scfr scfrclean

all apps: scfr
scfr:
	$(MAKE_TARGET)
scfrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/scfreg

REGSVR.EXE = scfreg$(EXE)
INC.REGSVR =
SRC.REGSVR = apps/tools/scfreg/scfreg.cpp
OBJ.REGSVR = $(addprefix $(OUT),$(notdir $(SRC.REGSVR:.cpp=$O)))
DEP.REGSVR = CSSYS CSUTIL CSGEOM
LIB.REGSVR = $(foreach d,$(DEP.REGSVR),$($d.LIB))

TO_INSTALL.EXE += $(REGSVR.EXE)

MSVC.DSP += REGSVR
DSP.REGSVR.NAME = scfreg
DSP.REGSVR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: scfr scfrclean

all: $(REGSVR.EXE)
scfr: $(OUTDIRS) $(REGSVR.EXE)
clean: scfrclean

$(REGSVR.EXE): $(OBJ.REGSVR) $(LIB.REGSVR)
	$(DO.LINK.CONSOLE.EXE)

scfrclean:
	-$(RM) $(REGSVR.EXE) $(OBJ.REGSVR)

ifdef DO_DEPEND
dep: $(OUTOS)scfreg.dep
$(OUTOS)scfreg.dep: $(SRC.REGSVR)
	$(DO.DEP)
else
-include $(OUTOS)scfreg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq ($(USE_PLUGINS),no)
