# Application description
DESCRIPTION.gfxtst = Crystal Space image manipulator

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make gfxtst       Make the $(DESCRIPTION.gfxtst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gfxtst gfxtstclean

all apps: gfxtst
gfxtst:
	$(MAKE_TARGET)
gfxtstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/gfxtst

GFXTEST.EXE = gfxtest$(EXE)
INC.GFXTEST =
SRC.GFXTEST = apps/tests/gfxtst/gfxtest.cpp
OBJ.GFXTEST = $(addprefix $(OUT),$(notdir $(SRC.GFXTEST:.cpp=$O)))
DEP.GFXTEST = CSGFX CSUTIL CSSYS CSUTIL CSTOOL CSUTIL CSSYS CSTOOL CSUTIL CSGEOM
LIB.GFXTEST = $(foreach d,$(DEP.GFXTEST),$($d.LIB))

TO_INSTALL.EXE += $(GFXTEST.EXE)

MSVC.DSP += GFXTEST
DSP.GFXTEST.NAME = gfxtest
DSP.GFXTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gfxtst gfxtstclean

gfxtst: $(OUTDIRS) $(GFXTEST.EXE)
clean: gfxtstclean

# Unfortunately, this command-line utility uses the plugin loading facilities
# of the system driver, and on some platforms, that requires linking via
# DO.LINK.EXE rather than the expected DO.LINK.CONSOLE.EXE.  We need to
# resolve this issue in the future so that DO.LINK.CONSOLE.EXE can be used.
$(GFXTEST.EXE): $(OBJ.GFXTEST) $(LIB.GFXTEST)
	$(DO.LINK.EXE)

gfxtstclean:
	-$(RM) $(GFXTEST.EXE) $(OBJ.GFXTEST)

ifdef DO_DEPEND
dep: $(OUTOS)gfxtst.dep
$(OUTOS)gfxtst.dep: $(SRC.GFXTEST)
	$(DO.DEP)
else
-include $(OUTOS)gfxtst.dep
endif

endif # ifeq ($(MAKESECTION),targets)
