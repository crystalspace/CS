# Application description
DESCRIPTION.vsh = Crystal Space Virtual Shell tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make vsh          Make the $(DESCRIPTION.vsh)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vsh vshclean

all apps: vsh
vsh:
	$(MAKE_APP)
vshclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


VSH.EXE = vsh$(EXE.CONSOLE)
DIR.VSH = apps/tools/vsh
OUT.VSH = $(OUT)/$(DIR.VSH)
INC.VSH =
SRC.VSH = apps/tools/vsh/vsh.cpp
OBJ.VSH = $(addprefix $(OUT.VSH)/,$(notdir $(SRC.VSH:.cpp=$O)))
DEP.VSH = CSUTIL CSGFX CSTOOL CSSYS CSUTIL CSSYS CSGEOM
LIB.VSH = $(foreach d,$(DEP.VSH),$($d.LIB))

TO_INSTALL.EXE += $(VSH.EXE)

MSVC.DSP += VSH
DSP.VSH.NAME = vsh
DSP.VSH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.vsh vshclean vshcleandep

all: $(VSH.EXE)
build.vsh: $(OUT.VSH) $(VSH.EXE)
clean: vshclean

$(OUT.VSH)/%$O: $(DIR.VSH)/%.cpp
	$(DO.COMPILE.CPP)

$(VSH.EXE): $(DEP.EXE) $(OBJ.VSH) $(LIB.VSH)
	$(DO.LINK.CONSOLE.EXE)

$(OUT.VSH):
	$(MKDIRS)

vshclean:
	-$(RMDIR) $(VSH.EXE) $(OBJ.VSH)

cleandep: vshcleandep
vshcleandep:
	-$(RM) $(OUT.VSH)/vsh.dep

ifdef DO_DEPEND
dep: $(OUT.VSH) $(OUT.VSH)/vsh.dep
$(OUT.VSH)/vsh.dep: $(SRC.VSH)
	$(DO.DEPEND)
else
-include $(OUT.VSH)/vsh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
