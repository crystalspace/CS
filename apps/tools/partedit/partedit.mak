DESCRIPTION.partedit = Crystal Space particle editor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make partedit     Make the $(DESCRIPTION.partedit)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: partedit parteditclean

all apps: partedit
partedit:
	$(MAKE_APP)
parteditclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PARTEDIT.EXE=partedit$(EXE)
DIR.PARTEDIT = apps/tools/partedit
OUT.PARTEDIT = $(OUT)/$(DIR.PARTEDIT)
INC.PARTEDIT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PARTEDIT)/*.h))
SRC.PARTEDIT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PARTEDIT)/*.cpp))
OBJ.PARTEDIT = $(addprefix $(OUT.PARTEDIT)/,$(notdir $(SRC.PARTEDIT:.cpp=$O)))
DEP.PARTEDIT = CSGFX CSTOOL CSGEOM CSUTIL
LIB.PARTEDIT = $(foreach d,$(DEP.PARTEDIT),$($d.LIB))

OUTDIRS += $(OUT.PARTEDIT)

TO_INSTALL.EXE	  += $(PARTEDIT.EXE)

MSVC.DSP += PARTEDIT
DSP.PARTEDIT.NAME = partedit
DSP.PARTEDIT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.partedit parteditclean parteditcleandep

all: $(PARTEDIT.EXE)
build.partedit: $(OUTDIRS) $(PARTEDIT.EXE)
clean: parteditclean

$(OUT.PARTEDIT)/%$O: $(SRCDIR)/$(DIR.PARTEDIT)/%.cpp
	$(DO.COMPILE.CPP)

$(PARTEDIT.EXE): $(DEP.EXE) $(OBJ.PARTEDIT) $(LIB.PARTEDIT)
	$(DO.LINK.EXE)

parteditclean:
	-$(RM) partedit.txt
	-$(RMDIR) $(PARTEDIT.EXE) $(OBJ.PARTEDIT)

cleandep: parteditcleandep
parteditcleandep:
	-$(RM) $(OUT.PARTEDIT)/partedit.dep

ifdef DO_DEPEND
dep: $(OUT.PARTEDIT) $(OUT.PARTEDIT)/partedit.dep
$(OUT.PARTEDIT)/partedit.dep: $(SRC.PARTEDIT)
	$(DO.DEPEND)
else
-include $(OUT.PARTEDIT)/partedit.dep
endif

endif # ifeq ($(MAKESECTION),targets)
