# Application description
DESCRIPTION.pathtut = Crystal Space Path Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make pathtut      Make the $(DESCRIPTION.pathtut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pathtut pathtutclean

all apps: pathtut
pathtut:
	$(MAKE_APP)
pathtutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PATHTUT.EXE = pathtut$(EXE)
DIR.PATHTUT = apps/tutorial/pathtut
OUT.PATHTUT = $(OUT)/$(DIR.PATHTUT)
INC.PATHTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PATHTUT)/*.h ))
SRC.PATHTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PATHTUT)/*.cpp ))
OBJ.PATHTUT = $(addprefix $(OUT.PATHTUT)/,$(notdir $(SRC.PATHTUT:.cpp=$O)))
DEP.PATHTUT = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.PATHTUT = $(foreach d,$(DEP.PATHTUT),$($d.LIB))

OUTDIRS += $(OUT.PATHTUT)

#TO_INSTALL.EXE += $(PATHTUT.EXE)

MSVC.DSP += PATHTUT
DSP.PATHTUT.NAME = pathtut
DSP.PATHTUT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pathtut pathtutclean pathtutcleandep

build.pathtut: $(OUTDIRS) $(PATHTUT.EXE)
clean: pathtutclean

$(OUT.PATHTUT)/%$O: $(SRCDIR)/$(DIR.PATHTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(PATHTUT.EXE): $(DEP.EXE) $(OBJ.PATHTUT) $(LIB.PATHTUT)
	$(DO.LINK.EXE)

pathtutclean:
	-$(RM) pathtut.txt
	-$(RMDIR) $(PATHTUT.EXE) $(OBJ.PATHTUT)

cleandep: pathtutcleandep
pathtutcleandep:
	-$(RM) $(OUT.PATHTUT)/pathtut.dep

ifdef DO_DEPEND
dep: $(OUT.PATHTUT) $(OUT.PATHTUT)/pathtut.dep
$(OUT.PATHTUT)/pathtut.dep: $(SRC.PATHTUT)
	$(DO.DEPEND)
else
-include $(OUT.PATHTUT)/pathtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

