# Application description
DESCRIPTION.milk2spr = Milkshape ASCII Model conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make milk2spr     Make the $(DESCRIPTION.milk2spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: milk2spr milk2sprclean

all apps: milk2spr
milk2spr:
	$(MAKE_APP)
milk2sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/milk2spr

MILK2SPR.EXE = milk2spr$(EXE)
INC.MILK2SPR = $(wildcard apps/import/milk2spr/*.h)
SRC.MILK2SPR = $(wildcard apps/import/milk2spr/*.cpp)
OBJ.MILK2SPR = $(addprefix $(OUT)/,$(notdir $(SRC.MILK2SPR:.cpp=$O)))
DEP.MILK2SPR = CSGFX CSUTIL CSSYS CSUTIL CSGEOM
LIB.MILK2SPR = $(foreach d,$(DEP.MILK2SPR),$($d.LIB))

TO_INSTALL.EXE += $(MILK2SPR.EXE)

MSVC.DSP += MILK2SPR
DSP.MILK2SPR.NAME = milk2spr
DSP.MILK2SPR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.milk2spr milk2sprclean

all: $(MILK2SPR.EXE)
build.milk2spr: $(OUTDIRS) $(MILK2SPR.EXE)
clean: milk2sprclean

$(MILK2SPR.EXE): $(OBJ.MILK2SPR) $(LIB.MILK2SPR)
	$(DO.LINK.CONSOLE.EXE)

milk2sprclean:
	-$(RM) $(MILK2SPR.EXE) $(OBJ.MILK2SPR)

ifdef DO_DEPEND
dep: $(OUTOS)/milk2spr.dep
$(OUTOS)/milk2spr.dep: $(SRC.MILK2SPR)
	$(DO.DEP)
else
-include $(OUTOS)/milk2spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
