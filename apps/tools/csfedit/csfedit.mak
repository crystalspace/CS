# Application description
DESCRIPTION.csfedit = Crystal Space font editor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make csfedit      Make the $(DESCRIPTION.csfedit)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfedit csfeditclean

all apps: csfedit
csfedit:
	$(MAKE_APP)
csfeditclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSFEDIT.EXE=csfedit$(EXE)
DIR.CSFEDIT = apps/tools/csfedit
OUT.CSFEDIT = $(OUT)/$(DIR.CSFEDIT)
INC.CSFEDIT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSFEDIT)/*.h ))
SRC.CSFEDIT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSFEDIT)/*.cpp ))
OBJ.CSFEDIT = $(addprefix $(OUT.CSFEDIT)/,$(notdir $(SRC.CSFEDIT:.cpp=$O)))
DEP.CSFEDIT = CSGFX CSWS CSTOOL CSUTIL CSGEOM CSUTIL
LIB.CSFEDIT = $(foreach d,$(DEP.CSFEDIT),$($d.LIB))

OUTDIRS += $(OUT.CSFEDIT)

#TO_INSTALL.EXE += $(CSFEDIT.EXE)

MSVC.DSP += CSFEDIT
DSP.CSFEDIT.NAME = csfedit
DSP.CSFEDIT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csfedit csfeditclean csfeditcleandep

all: $(CSFEDIT.EXE)
build.csfedit: $(OUTDIRS) $(CSFEDIT.EXE)
clean: csfeditclean

$(OUT.CSFEDIT)/%$O: $(SRCDIR)/$(DIR.CSFEDIT)/%.cpp
	$(DO.COMPILE.CPP)

$(CSFEDIT.EXE): $(DEP.EXE) $(OBJ.CSFEDIT) $(LIB.CSFEDIT)
	$(DO.LINK.EXE)

csfeditclean:
	-$(RM) csfedit.txt
	-$(RMDIR) $(CSFEDIT.EXE) $(OBJ.CSFEDIT)

cleandep: csfeditcleandep
csfeditcleandep:
	-$(RM) $(OUT.CSFEDIT)/csfedit.dep

ifdef DO_DEPEND
dep: $(OUT.CSFEDIT) $(OUT.CSFEDIT)/csfedit.dep
$(OUT.CSFEDIT)/csfedit.dep: $(SRC.CSFEDIT)
	$(DO.DEPEND)
else
-include $(OUT.CSFEDIT)/csfedit.dep
endif

endif # ifeq ($(MAKESECTION),targets)
