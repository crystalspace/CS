# Application description
DESCRIPTION.simpvs = Crystal Space tutorial, video selector

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simpvs       Make the $(DESCRIPTION.simpvs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpvs simpvsclean

all apps: simpvs
simpvs:
	$(MAKE_APP)
simpvsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPVS.EXE = simpvs$(EXE)
DIR.SIMPVS = apps/tutorial/simpvs
OUT.SIMPVS = $(OUT)/$(DIR.SIMPVS)
INC.SIMPVS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPVS)/*.h ))
SRC.SIMPVS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPVS)/*.cpp ))
OBJ.SIMPVS = $(addprefix $(OUT.SIMPVS)/,$(notdir $(SRC.SIMPVS:.cpp=$O)))
DEP.SIMPVS = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPVS = $(foreach d,$(DEP.SIMPVS),$($d.LIB))

OUTDIRS += $(OUT.SIMPVS)

#TO_INSTALL.EXE += $(SIMPVS.EXE)

MSVC.DSP += SIMPVS
DSP.SIMPVS.NAME = simpvs
DSP.SIMPVS.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simpvs simpvsclean simpvscleandep

build.simpvs: $(OUTDIRS) $(SIMPVS.EXE)
clean: simpvsclean

$(OUT.SIMPVS)/%$O: $(SRCDIR)/$(DIR.SIMPVS)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPVS.EXE): $(DEP.EXE) $(OBJ.SIMPVS) $(LIB.SIMPVS)
	$(DO.LINK.EXE)

simpvsclean:
	-$(RM) simpvs.txt
	-$(RMDIR) $(SIMPVS.EXE) $(OBJ.SIMPVS)

cleandep: simpvscleandep
simpvscleandep:
	-$(RM) $(OUT.SIMPVS)/simpvs.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPVS) $(OUT.SIMPVS)/simpvs.dep
$(OUT.SIMPVS)/simpvs.dep: $(SRC.SIMPVS)
	$(DO.DEPEND)
else
-include $(OUT.SIMPVS)/simpvs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
