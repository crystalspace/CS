DESCRIPTION.scftut     = SCF tutorial application
DESCRIPTION.scftutdlls = SCF tutorial plug-ins

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make scftut       Make the $(DESCRIPTION.scftut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: scftut scftutclean

all apps: scftut
scftut: scftutdlls
	$(MAKE_TARGET)
scftutdlls:
	$(MAKE_TARGET) MAKE_DLL=yes
scftutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/scftutor

DEP.SCFTUTOR = CSUTIL CSSYS CSUTIL
LIB.SCFTUTOR = $(foreach d,$(DEP.SCFTUTOR),$($d.LIB))

ZOO.EXE = zoo$(EXE)
INC.ZOO = $(wildcard apps/scftutor/*.h)
SRC.ZOO = apps/scftutor/zoo.cpp apps/scftutor/frog.cpp
OBJ.ZOO = $(addprefix $(OUT),$(notdir $(SRC.ZOO:.cpp=$O)))
DEP.ZOO = $(DEP.SCFTUTOR)
LIB.ZOO = $(LIB.SCFTUTOR)

INC.DOG = apps/scftutor/idog.h apps/scftutor/iname.h
SRC.DOG = apps/scftutor/dog.cpp
OBJ.DOG = $(addprefix $(OUT),$(notdir $(SRC.DOG:.cpp=$O)))
DEP.DOG = $(DEP.SCFTUTOR)
LIB.DOG = $(LIB.SCFTUTOR)

INC.WORM = apps/scftutor/iworm.h
SRC.WORM = apps/scftutor/worm.cpp
OBJ.WORM = $(addprefix $(OUT),$(notdir $(SRC.WORM:.cpp=$O)))
DEP.WORM = $(DEP.SCFTUTOR)
LIB.WORM = $(LIB.SCFTUTOR)

ifeq ($(USE_PLUGINS),yes)
  DOG.DLL  = $(OUTDLL)dog$(DLL)
  WORM.DLL = $(OUTDLL)worm$(DLL)
  #TO_INSTALL.DYNAMIC_LIBS += $(DOG.DLL) $(WORM.DLL)
else
  SRC.ZOO += $(SRC.DOG) $(SRC.WORM)
  OBJ.ZOO += $(OBJ.DOG) $(OBJ.WORM)
endif

#TO_INSTALL.EXE+=$(ZOO.EXE)

MSVC.DSP += ZOO
DSP.ZOO.NAME = zoo
DSP.ZOO.TYPE = appcon

MSVC.DSP += DOG
DSP.DOG.NAME = dog
DSP.DOG.TYPE = plugin

MSVC.DSP += WORM
DSP.WORM.NAME = worm
DSP.WORM.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: scftut scftutclean

all: $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL)
scftut: $(OUTDIRS) $(ZOO.EXE)
scftutdlls: $(OUTDIRS) $(DOG.DLL) $(WORM.DLL)
clean: scftutclean

$(ZOO.EXE): $(OBJ.ZOO) $(LIB.ZOO)
	$(DO.LINK.CONSOLE.EXE)
$(DOG.DLL): $(OBJ.DOG) $(LIB.DOG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) \
	$(DO.PLUGIN.POSTAMBLE)
$(WORM.DLL): $(OBJ.WORM) $(LIB.WORM)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) \
	$(DO.PLUGIN.POSTAMBLE)

scftutclean:
	-$(RM) $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL) $(OBJ.ZOO) $(OBJ.DOG) \
	$(OBJ.WORM)

ifdef DO_DEPEND
dep: $(OUTOS)scftut.dep
$(OUTOS)scftut.dep: $(SRC.ZOO) $(SRC.DOG) $(SRC.WORM)
	$(DO.DEP)
else
-include $(OUTOS)scftut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
