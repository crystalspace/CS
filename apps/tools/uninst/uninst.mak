# Application description
DESCRIPTION.uninst = Uninstall program

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make uninst       Make the $(DESCRIPTION.uninst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: uninst uninstclean

uninst:
	$(MAKE_APP)
uninstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

UNINST.EXE = uninst$(EXE.CONSOLE)
DIR.UNINST = apps/tools/uninst
OUT.UNINST = $(OUT)/$(DIR.UNINST)
INC.UNINST = $(wildcard $(DIR.UNINST)/*.h)
SRC.UNINST = $(wildcard $(DIR.UNINST)/*.cpp)
OBJ.UNINST = $(addprefix $(OUT.UNINST)/,$(notdir $(SRC.UNINST:.cpp=$O)))
DEP.UNINST =
LIB.UNINST =

OUTDIRS += $(OUT.UNINST)

# Uninstall program is installed in the CS root rather than CS/bin.
TO_INSTALL.ROOT += $(UNINST.EXE)

#MSVC.DSP += UNINST
#DSP.UNINST.NAME = uninst
#DSP.UNINST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.uninst uninstclean uninstcleandep

all apps: uninst
build.uninst: $(OUTDIRS) $(UNINST.EXE)
clean: uninstclean

$(OUT.UNINST)/%$O: $(DIR.UNINST)/%.cpp
	$(DO.COMPILE.CPP)

$(UNINST.EXE): $(OBJ.UNINST) $(LIB.UNINST)
	$(DO.LINK.CONSOLE.EXE)

uninstclean:
	-$(RM) uninst.txt
	-$(RMDIR) $(UNINST.EXE) $(OBJ.UNINST)

cleandep: uninstcleandep
uninstcleandep:
	-$(RM) $(OUT.UNINST)/uninst.dep

ifdef DO_DEPEND
dep: $(OUT.UNINST) $(OUT.UNINST)/uninst.dep
$(OUT.UNINST)/uninst.dep: $(SRC.UNINST)
	$(DO.DEPEND)
else
-include $(OUT.UNINST)/uninst.dep
endif

endif # ifeq ($(MAKESECTION),targets)
