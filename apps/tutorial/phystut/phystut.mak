# Application description
DESCRIPTION.phystut = Crystal Space physics tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make phystut      Make the $(DESCRIPTION.phystut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phystut phystutclean

all apps: phystut
phystut:
	$(MAKE_APP)
phystutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PHYSTUT.EXE = phystut$(EXE)
DIR.PHYSTUT = apps/tutorial/phystut
OUT.PHYSTUT = $(OUT)/$(DIR.PHYSTUT)
INC.PHYSTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PHYSTUT)/*.h ))
SRC.PHYSTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PHYSTUT)/*.cpp ))
OBJ.PHYSTUT = $(addprefix $(OUT.PHYSTUT)/,$(notdir $(SRC.PHYSTUT:.cpp=$O)))
DEP.PHYSTUT = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.PHYSTUT = $(foreach d,$(DEP.PHYSTUT),$($d.LIB))

OUTDIRS += $(OUT.PHYSTUT)

TO_INSTALL.EXE += $(PHYSTUT.EXE)

MSVC.DSP += PHYSTUT
DSP.PHYSTUT.NAME = phystut
DSP.PHYSTUT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.phystut phystutclean phystutcleandep

build.phystut: $(OUTDIRS) $(PHYSTUT.EXE)
clean: phystutclean

$(OUT.PHYSTUT)/%$O: $(SRCDIR)/$(DIR.PHYSTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(PHYSTUT.EXE): $(DEP.EXE) $(OBJ.PHYSTUT) $(LIB.PHYSTUT)
	$(DO.LINK.EXE)

phystutclean:
	-$(RM) phystut.txt
	-$(RMDIR) $(PHYSTUT.EXE) $(OBJ.PHYSTUT)

cleandep: phystutcleandep
phystutcleandep:
	-$(RM) $(OUT.PHYSTUT)/phystut.dep

ifdef DO_DEPEND
dep: $(OUT.PHYSTUT) $(OUT.PHYSTUT)/phystut.dep
$(OUT.PHYSTUT)/phystut.dep: $(SRC.PHYSTUT)
	$(DO.DEPEND)
else
-include $(OUT.PHYSTUT)/phystut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
