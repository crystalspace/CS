# Application description
DESCRIPTION.levtool = Crystal Space map tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make levtool      Make the $(DESCRIPTION.levtool)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: levtool levtoolclean

all apps: levtool
levtool:
	$(MAKE_APP)
levtoolclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LEVTOOL.EXE = levtool$(EXE.CONSOLE)
DIR.LEVTOOL = apps/tools/levtool
OUT.LEVTOOL = $(OUT)/$(DIR.LEVTOOL)
INC.LEVTOOL = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.LEVTOOL)/*.h ))
SRC.LEVTOOL = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.LEVTOOL)/*.cpp ))
OBJ.LEVTOOL = $(addprefix $(OUT.LEVTOOL)/,$(notdir $(SRC.LEVTOOL:.cpp=$O)))
DEP.LEVTOOL = CSTOOL CSGEOM CSTOOL CSGFX CSUTIL CSUTIL
LIB.LEVTOOL = $(foreach d,$(DEP.LEVTOOL),$($d.LIB))

OUTDIRS += $(OUT.LEVTOOL)

TO_INSTALL.EXE    += $(LEVTOOL.EXE)

MSVC.DSP += LEVTOOL
DSP.LEVTOOL.NAME = levtool
DSP.LEVTOOL.TYPE = appcon

#$(LEVTOOL.EXE).WINRSRC = $(SRCDIR)/libs/csutil/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.levtool levtoolclean levtoolcleandep

all: $(LEVTOOL.EXE)
build.levtool: $(OUTDIRS) $(LEVTOOL.EXE)
clean: levtoolclean

$(OUT.LEVTOOL)/%$O: $(SRCDIR)/$(DIR.LEVTOOL)/%.cpp
	$(DO.COMPILE.CPP)

$(LEVTOOL.EXE): $(DEP.EXE) $(OBJ.LEVTOOL) $(LIB.LEVTOOL)
	$(DO.LINK.CONSOLE.EXE)

levtoolclean:
	-$(RM) levtool.txt
	-$(RMDIR) $(LEVTOOL.EXE) $(OBJ.LEVTOOL)

cleandep: levtoolcleandep
levtoolcleandep:
	-$(RM) $(OUT.LEVTOOL)/levtool.dep

ifdef DO_DEPEND
dep: $(OUT.LEVTOOL) $(OUT.LEVTOOL)/levtool.dep
$(OUT.LEVTOOL)/levtool.dep: $(SRC.LEVTOOL)
	$(DO.DEPEND)
else
-include $(OUT.LEVTOOL)/levtool.dep
endif

endif # ifeq ($(MAKESECTION),targets)
