# Application description
DESCRIPTION.tbconvert = Crystal Space big terrain converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tbconvert    Make the $(DESCRIPTION.tbconvert)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tbconvert tbconvertclean

all apps: tbconvert
tbconvert:
	$(MAKE_APP)
tbconvertclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

TBCONVERT.EXE = tbconvert$(EXE.CONSOLE)
DIR.TBCONVERT = apps/tools/tbconv
OUT.TBCONVERT = $(OUT)/$(DIR.TBCONVERT)
INC.TBCONVERT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.TBCONVERT)/*.h))
SRC.TBCONVERT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.TBCONVERT)/*.cpp))
OBJ.TBCONVERT = \
  $(addprefix $(OUT.TBCONVERT)/,$(notdir $(SRC.TBCONVERT:.cpp=$O)))
DEP.TBCONVERT = CSTOOL CSGEOM CSTOOL CSGFX CSUTIL CSUTIL
LIB.TBCONVERT = $(foreach d,$(DEP.TBCONVERT),$($d.LIB))

OUTDIRS += $(OUT.TBCONVERT)

TO_INSTALL.EXE += $(TBCONVERT.EXE)

MSVC.DSP += TBCONVERT
DSP.TBCONVERT.NAME = tbconvert
DSP.TBCONVERT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.tbconvert tbconvertclean tbconvertcleandep

build.tbconvert: $(OUTDIRS) $(TBCONVERT.EXE)
clean: tbconvertclean

$(OUT.TBCONVERT)/%$O: $(SRCDIR)/$(DIR.TBCONVERT)/%.cpp
	$(DO.COMPILE.CPP)

$(TBCONVERT.EXE): $(DEP.EXE) $(OBJ.TBCONVERT) $(LIB.TBCONVERT)
	$(DO.LINK.CONSOLE.EXE)

tbconvertclean:
	-$(RM) tbconvert.txt
	-$(RMDIR) $(TBCONVERT.EXE) $(OBJ.TBCONVERT)

cleandep: tbconvertcleandep
tbconvertcleandep:
	-$(RM) $(OUT.TBCONVERT)/tbconvert.dep

ifdef DO_DEPEND
dep: $(OUT.TBCONVERT) $(OUT.TBCONVERT)/tbconvert.dep
$(OUT.TBCONVERT)/tbconvert.dep: $(SRC.TBCONVERT)
	$(DO.DEPEND)
else
-include $(OUT.TBCONVERT)/tbconvert.dep
endif

endif # ifeq ($(MAKESECTION),targets)
