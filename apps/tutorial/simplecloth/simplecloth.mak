# Application description
DESCRIPTION.simplecloth = Crystal Space Cloth demonstration

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simplecloth  Make the $(DESCRIPTION.simplecloth)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simplecloth simpleclothclean

all apps: simplecloth
simplecloth:
	$(MAKE_APP)
simpleclothclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPLECLOTH.EXE = simplecloth$(EXE)
DIR.SIMPLECLOTH = apps/tutorial/simplecloth
OUT.SIMPLECLOTH = $(OUT)/$(DIR.SIMPLECLOTH)
INC.SIMPLECLOTH = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLECLOTH)/*.h ))
SRC.SIMPLECLOTH = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLECLOTH)/*.cpp ))
OBJ.SIMPLECLOTH = \
  $(addprefix $(OUT.SIMPLECLOTH)/,$(notdir $(SRC.SIMPLECLOTH:.cpp=$O)))
DEP.SIMPLECLOTH = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPLECLOTH = $(foreach d,$(DEP.SIMPLECLOTH),$($d.LIB))

OUTDIRS += $(OUT.SIMPLECLOTH)

#TO_INSTALL.EXE += $(SIMPLECLOTH.EXE)

MSVC.DSP += SIMPLECLOTH
DSP.SIMPLECLOTH.NAME = simplecloth
DSP.SIMPLECLOTH.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simplecloth simpleclothclean simpleclothcleandep

build.simplecloth: $(OUTDIRS) $(SIMPLECLOTH.EXE)
clean: simpleclothclean

$(OUT.SIMPLECLOTH)/%$O: $(SRCDIR)/$(DIR.SIMPLECLOTH)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLECLOTH.EXE): $(DEP.EXE) $(OBJ.SIMPLECLOTH) $(LIB.SIMPLECLOTH)
	$(DO.LINK.EXE)

simpleclothclean:
	-$(RM) simplecloth.txt
	-$(RMDIR) $(SIMPLECLOTH.EXE) $(OBJ.SIMPLECLOTH)

cleandep: simpleclothcleandep
simpleclothcleandep:
	-$(RM) $(OUT.SIMPLECLOTH)/simplecloth.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLECLOTH) $(OUT.SIMPLECLOTH)/simplecloth.dep
$(OUT.SIMPLECLOTH)/simplecloth.dep: $(SRC.SIMPLECLOTH)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLECLOTH)/simplecloth.dep
endif

endif # ifeq ($(MAKESECTION),targets)
