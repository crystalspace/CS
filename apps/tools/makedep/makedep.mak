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

vpath %.cpp apps/tools/makedep

MAKEDEP.EXE=makedep$(EXE.CONSOLE)
INC.MAKEDEP = $(wildcard apps/tools/makedep/*.h)
SRC.MAKEDEP = $(wildcard apps/tools/makedep/*.cpp)
OBJ.MAKEDEP = $(addprefix $(OUT)/,$(notdir $(SRC.MAKEDEP:.cpp=$O)))
DEP.MAKEDEP =
LIB.MAKEDEP = $(foreach d,$(DEP.MAKEDEP),$($d.LIB))

#TO_INSTALL.EXE+=$(MAKEDEP.EXE)

ifdef MAKEDEP.INCLUDES
CFLAGS.MAKEDEP = -DPREINCDIR=\"$(MAKEDEP.INCLUDES)\"
endif

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.makedep makedepclean

all: $(MAKEDEP.EXE)
build.makedep: $(OUTDIRS) $(MAKEDEP.EXE)
clean: makedepclean

$(OUT)/main$O: apps/tools/makedep/main.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.MAKEDEP)

$(MAKEDEP.EXE): $(OBJ.MAKEDEP) $(LIB.MAKEDEP)
	$(DO.LINK.CONSOLE.EXE)

makedepclean:
	-$(RMDIR) $(MAKEDEP.EXE) $(OBJ.MAKEDEP)

ifdef DO_DEPEND
dep: $(OUTOS)/makedep.dep
$(OUTOS)/makedep.dep: $(SRC.MAKEDEP)
	$(DO.DEP)
else
-include $(OUTOS)/makedep.dep
endif

endif # ifeq ($(MAKESECTION),targets)
