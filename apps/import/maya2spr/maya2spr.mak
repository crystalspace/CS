# Application description
DESCRIPTION.maya2spr = Maya to CS Sprite convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make maya2spr     Make the $(DESCRIPTION.maya2spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: maya2spr maya2sprclean

all apps: maya2spr
maya2spr:
	$(MAKE_APP)
maya2sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/maya2spr

MAYA2SPR.EXE = maya2spr$(EXE.CONSOLE)
INC.MAYA2SPR = $(wildcard apps/import/maya2spr/*.h)
SRC.MAYA2SPR = $(wildcard apps/import/maya2spr/*.cpp)
OBJ.MAYA2SPR = $(addprefix $(OUT)/,$(notdir $(SRC.MAYA2SPR:.cpp=$O)))
DEP.MAYA2SPR = CSGFX CSUTIL CSSYS CSUTIL CSGEOM
LIB.MAYA2SPR = $(foreach d,$(DEP.MAYA2SPR),$($d.LIB))

TO_INSTALL.EXE += $(MAYA2SPR.EXE)

MSVC.DSP += MAYA2SPR
DSP.MAYA2SPR.NAME = maya2spr
DSP.MAYA2SPR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.maya2spr maya2sprclean

all: $(MAYA2SPR.EXE)
build.maya2spr: $(OUTDIRS) $(MAYA2SPR.EXE)
clean: maya2sprclean

$(MAYA2SPR.EXE): $(OBJ.MAYA2SPR) $(LIB.MAYA2SPR)
	$(DO.LINK.CONSOLE.EXE)

maya2sprclean:
	-$(RMDIR) $(MAYA2SPR.EXE) $(OBJ.MAYA2SPR)

ifdef DO_DEPEND
dep: $(OUTOS)/maya2spr.dep
$(OUTOS)/maya2spr.dep: $(SRC.MAYA2SPR)
	$(DO.DEP)
else
-include $(OUTOS)/maya2spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
