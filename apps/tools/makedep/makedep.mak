# Application description
DESCRIPTION.makedep = Dependency generation tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make makedep      Make the $(DESCRIPTION.makedep)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: makedep makedepclean

#all apps: makedep
makedep:
	$(MAKE_APP)
makedepclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


MAKEDEP.EXE=makedep$(EXE.CONSOLE)
DIR.MAKEDEP = apps/tools/makedep
OUT.MAKEDEP = $(OUT)/$(DIR.MAKEDEP)
INC.MAKEDEP = $(wildcard $(DIR.MAKEDEP)/*.h )
SRC.MAKEDEP = $(wildcard $(DIR.MAKEDEP)/*.cpp )
OBJ.MAKEDEP = $(addprefix $(OUT.MAKEDEP)/,$(notdir $(SRC.MAKEDEP:.cpp=$O)))
DEP.MAKEDEP =
LIB.MAKEDEP = $(foreach d,$(DEP.MAKEDEP),$($d.LIB))

#TO_INSTALL.EXE+=$(MAKEDEP.EXE)

ifdef MAKEDEP.INCLUDES
CFLAGS.MAKEDEP = -DPREINCDIR=\"$(MAKEDEP.INCLUDES)\"
endif

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.makedep makedepclean makedepcleandep

all: $(MAKEDEP.EXE)
build.makedep: $(OUT.MAKEDEP) $(MAKEDEP.EXE)
clean: makedepclean

$(OUT.MAKEDEP)/%$O: $(DIR.MAKEDEP)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT)/main$O: apps/tools/makedep/main.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.MAKEDEP)

$(MAKEDEP.EXE): $(OBJ.MAKEDEP) $(LIB.MAKEDEP)
	$(DO.LINK.CONSOLE.EXE)

$(OUT.MAKEDEP):
	$(MKDIRS)

makedepclean:
	-$(RM) makedep.txt
	-$(RMDIR) $(MAKEDEP.EXE) $(OBJ.MAKEDEP)

cleandep: makedepcleandep
makedepcleandep:
	-$(RM) $(OUT.MAKEDEP)/makedep.dep

ifdef DO_DEPEND
dep: $(OUT.MAKEDEP) $(OUT.MAKEDEP)/makedep.dep
$(OUT.MAKEDEP)/makedep.dep: $(SRC.MAKEDEP)
	$(DO.DEPEND)
else
-include $(OUT.MAKEDEP)/makedep.dep
endif

endif # ifeq ($(MAKESECTION),targets)
