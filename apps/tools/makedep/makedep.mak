# Application description
DESCRIPTION.mkdep = Dependency generation tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make mkdep        Make the $(DESCRIPTION.mkdep)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mkdep mkdepclean

#all apps: mkdep
mkdep:
	$(MAKE_TARGET)
mkdepclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/makedep

MAKEDEP.EXE=makedep$(EXE)
INC.MAKEDEP = $(wildcard apps/tools/makedep/*.h)
SRC.MAKEDEP = $(wildcard apps/tools/makedep/*.cpp)
OBJ.MAKEDEP = $(addprefix $(OUT),$(notdir $(SRC.MAKEDEP:.cpp=$O)))
DEP.MAKEDEP =
LIB.MAKEDEP = $(foreach d,$(DEP.MAKEDEP),$($d.LIB))

#TO_INSTALL.EXE+=$(MAKEDEP.EXE)

ifdef MAKEDEP.INCLUDES
CFLAGS.MAKEDEP = -DPREINCDIR=\"$(MAKEDEP.INCLUDES)\"
endif

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mkdep mkdepclean

all: $(MAKEDEP.EXE)
mkdep: $(OUTDIRS) $(MAKEDEP.EXE)
clean: mkdepclean

$(OUT)main$O: apps/tools/makedep/main.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.MAKEDEP)

$(MAKEDEP.EXE): $(OBJ.MAKEDEP) $(LIB.MAKEDEP)
	$(DO.LINK.CONSOLE.EXE)

mkdepclean:
	-$(RM) $(MAKEDEP.EXE) $(OBJ.MAKEDEP)

ifdef DO_DEPEND
dep: $(OUTOS)mkdep.dep
$(OUTOS)mkdep.dep: $(SRC.MAKEDEP)
	$(DO.DEP)
else
-include $(OUTOS)mkdep.dep
endif

endif # ifeq ($(MAKESECTION),targets)
