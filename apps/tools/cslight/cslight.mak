# Application description
DESCRIPTION.cslght = Crystal Space Lighting Calculator

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make cslght       Make the $(DESCRIPTION.cslght)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cslght cslghtclean

all apps: cslght
cslght:
	$(MAKE_TARGET)
cslghtclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/cslight

CSLIGHT.EXE = cslight$(EXE)
INC.CSLIGHT = $(wildcard apps/tools/cslight/*.h)
SRC.CSLIGHT = $(wildcard apps/tools/cslight/*.cpp)
OBJ.CSLIGHT = $(addprefix $(OUT),$(notdir $(SRC.CSLIGHT:.cpp=$O)))
DEP.CSLIGHT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSLIGHT = $(foreach d,$(DEP.CSLIGHT),$($d.LIB))

#TO_INSTALL.EXE += $(CSLIGHT.EXE)

MSVC.DSP += CSLIGHT
DSP.CSLIGHT.NAME = cslight
DSP.CSLIGHT.TYPE = appcon
DSP.CSLIGHT.RESOURCES = apps/tools/cslight/cslight.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cslght cslghtclean

all: $(CSLIGHT.EXE)
cslght: $(OUTDIRS) $(CSLIGHT.EXE)
clean: cslghtclean

$(CSLIGHT.EXE): $(DEP.EXE) $(OBJ.CSLIGHT) $(LIB.CSLIGHT)
	$(DO.LINK.EXE)

cslghtclean:
	-$(RM) $(CSLIGHT.EXE) $(OBJ.CSLIGHT)

ifdef DO_DEPEND
dep: $(OUTOS)cslight.dep
$(OUTOS)cslight.dep: $(SRC.CSLIGHT)
	$(DO.DEP)
else
-include $(OUTOS)cslight.dep
endif

endif # ifeq ($(MAKESECTION),targets)
