# Application description
DESCRIPTION.scftut = Crystal Space SCF tutorial application

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make scftut       Make the $(DESCRIPTION.scftut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: scftut

all apps: scftut
scftut:
	$(MAKE_TARGET)
scftutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/scftutor

ZOO.EXE = zoo$(EXE)
DOG.DLL = Dog$(DLL)
WORM.DLL = Worm$(DLL)
SRC.ZOO = apps/scftutor/zoo.cpp apps/scftutor/frog.cpp
OBJ.ZOO = $(addprefix $(OUT),$(notdir $(SRC.ZOO:.cpp=$O)))
SRC.DOG = apps/scftutor/dog.cpp
OBJ.DOG = $(addprefix $(OUT),$(notdir $(SRC.DOG:.cpp=$O)))
SRC.WORM = apps/scftutor/worm.cpp
OBJ.WORM = $(addprefix $(OUT),$(notdir $(SRC.WORM:.cpp=$O)))
DESCRIPTION.$(ZOO.EXE) = $(DESCRIPTION.scftut)
DESCRIPTION.$(DOG.DLL) = Sample Dog class
DESCRIPTION.$(WORM.DLL) = Sample Worm class

LIB.SCFTUTOR = $(CSUTIL.LIB) $(CSSYS.LIB)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: scftut scftutclean

all: $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL)
scftut: $(OUTDIRS) $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL)
clean: scftutclean

$(ZOO.EXE): $(DEP.EXE) $(OBJ.ZOO) $(LIB.SCFTUTOR)
	$(DO.LINK.CONSOLE.EXE) $(LIB.SCFTUTOR)
$(DOG.DLL): $(DEP.EXE) $(OBJ.DOG) $(LIB.SCFTUTOR)
	$(DO.SHARED.PLUGIN) $(LIB.SCFTUTOR)
$(WORM.DLL): $(DEP.EXE) $(OBJ.WORM) $(LIB.SCFTUTOR)
	$(DO.SHARED.PLUGIN) $(LIB.SCFTUTOR)

scftutclean:
	-$(RM) $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL) $(OBJ.ZOO) $(OBJ.DOG) $(OBJ.WORM)

ifdef DO_DEPEND
depend: $(OUTOS)csscftut.dep
$(OUTOS)csscftut.dep: $(SRC.REGSVR)
	$(DO.DEP)
else
-include $(OUTOS)csscftut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
