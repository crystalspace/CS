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

vpath %.cpp apps/tools/uninst

UNINST.EXE = uninst$(EXE.CONSOLE)
INC.UNINST =
SRC.UNINST = apps/tools/uninst/uninst.cpp
OBJ.UNINST = $(addprefix $(OUT)/,$(notdir $(SRC.UNINST:.cpp=$O)))
DEP.UNINST =
LIB.UNINST =

# Uninstall program is installed in the CS root rather than CS/bin.
TO_INSTALL.ROOT += $(UNINST.EXE)

#MSVC.DSP += UNINST
#DSP.UNINST.NAME = uninst
#DSP.UNINST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.uninst uninstclean

all apps: uninst
build.uninst: $(OUTDIRS) $(UNINST.EXE)
clean: uninstclean

$(UNINST.EXE): $(OBJ.UNINST) $(LIB.UNINST)
	$(DO.LINK.CONSOLE.EXE)

uninstclean:
	-$(RMDIR) $(UNINST.EXE) $(OBJ.UNINST)

ifdef DO_DEPEND
dep: $(OUTOS)/uninst.dep
$(OUTOS)/uninst.dep: $(SRC.UNINST)
	$(DO.DEP)
else
-include $(OUTOS)/uninst.dep
endif

endif # ifeq ($(MAKESECTION),targets)
