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

vpath %.cpp apps/tools/vsh

VSH.EXE = vsh$(EXE)
INC.VSH =
SRC.VSH = apps/tools/vsh/vsh.cpp
OBJ.VSH = $(addprefix $(OUT)/,$(notdir $(SRC.VSH:.cpp=$O)))
DEP.VSH = CSUTIL CSGFX CSTOOL CSSYS CSUTIL CSSYS CSGEOM
LIB.VSH = $(foreach d,$(DEP.VSH),$($d.LIB))

TO_INSTALL.EXE += $(VSH.EXE)

MSVC.DSP += VSH
DSP.VSH.NAME = vsh
DSP.VSH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.vsh vshclean

all: $(VSH.EXE)
build.vsh: $(OUTDIRS) $(VSH.EXE)
clean: vshclean

$(VSH.EXE): $(DEP.EXE) $(OBJ.VSH) $(LIB.VSH)
	$(DO.LINK.CONSOLE.EXE)

vshclean:
	-$(RM) $(VSH.EXE) $(OBJ.VSH)

ifdef DO_DEPEND
dep: $(OUTOS)/vsh.dep
$(OUTOS)/vsh.dep: $(SRC.VSH)
	$(DO.DEP)
else
-include $(OUTOS)/vsh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
