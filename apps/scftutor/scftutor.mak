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

.PHONY: zoo scftut scftutclean

all apps: scftut
zoo: scftut
scftut: scftutdlls
	$(MAKE_TARGET)
scftutdlls:
	$(MAKE_TARGET) MAKE_DLL=yes
scftutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DIR.SCFTUTOR = apps/scftutor
OUT.SCFTUTOR = $(OUT)/$(DIR.SCFTUTOR)
DEP.SCFTUTOR = CSUTIL CSSYS CSUTIL CSGEOM
LIB.SCFTUTOR = $(foreach d,$(DEP.SCFTUTOR),$($d.LIB))

ZOO.EXE = zoo$(EXE.CONSOLE)
INC.ZOO = $(wildcard $(DIR.SCFTUTOR)/*.h)
SRC.ZOO = $(DIR.SCFTUTOR)/zoo.cpp $(DIR.SCFTUTOR)/frog.cpp
OBJ.ZOO = $(addprefix $(OUT.SCFTUTOR)/,$(notdir $(SRC.ZOO:.cpp=$O)))
DEP.ZOO = $(DEP.SCFTUTOR)
LIB.ZOO = $(LIB.SCFTUTOR)

INC.DOG = $(DIR.SCFTUTOR)/idog.h $(DIR.SCFTUTOR)/iname.h
SRC.DOG = $(DIR.SCFTUTOR)/dog.cpp
OBJ.DOG = $(addprefix $(OUT.SCFTUTOR)/,$(notdir $(SRC.DOG:.cpp=$O)))
DEP.DOG = $(DEP.SCFTUTOR)
LIB.DOG = $(LIB.SCFTUTOR)

INC.WORM = $(DIR.SCFTUTOR)/iworm.h
SRC.WORM = $(DIR.SCFTUTOR)/worm.cpp
OBJ.WORM = $(addprefix $(OUT.SCFTUTOR)/,$(notdir $(SRC.WORM:.cpp=$O)))
DEP.WORM = $(DEP.SCFTUTOR)
LIB.WORM = $(LIB.SCFTUTOR)

ifeq ($(USE_PLUGINS),yes)
  DOG.DLL  = $(OUTDLL)/dog$(DLL)
  WORM.DLL = $(OUTDLL)/worm$(DLL)
  #TO_INSTALL.DYNAMIC_LIBS += $(DOG.DLL) $(WORM.DLL)
else
  SRC.ZOO += $(SRC.DOG) $(SRC.WORM)
  OBJ.ZOO += $(OBJ.DOG) $(OBJ.WORM)
endif

#TO_INSTALL.EXE += $(ZOO.EXE)

MSVC.DSP += ZOO
DSP.ZOO.NAME = zoo
DSP.ZOO.TYPE = appcon
# Those descriptions are really only for the Windows version info...
DESCRIPTION.zoo = The CrystalSpace Zoo

MSVC.DSP += DOG
DSP.DOG.NAME = dog
DSP.DOG.TYPE = plugin
DESCRIPTION.dog = The incredible Dog plug-in

MSVC.DSP += WORM
DSP.WORM.NAME = worm
DSP.WORM.TYPE = plugin
DESCRIPTION.worm = The fearsome Worm plug-in

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: scftut scftutclean scftutcleandep

all: $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL)
scftut: $(OUT.SCFTUTOR) $(ZOO.EXE)
scftutdlls: $(OUT.SCFTUTOR) $(DOG.DLL) $(WORM.DLL)
clean: scftutclean

$(OUT.SCFTUTOR)/%$O: $(DIR.SCFTUTOR)/%.cpp
	$(DO.COMPILE.CPP)

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

$(OUT.SCFTUTOR):
	$(MKDIRS)

scftutclean:
	-$(RMDIR) $(ZOO.EXE) $(DOG.DLL) $(WORM.DLL) $(OBJ.ZOO) $(OBJ.DOG) \
	$(OBJ.WORM)

cleandep: scftutcleandep
scftutcleandep:
	-$(RM) $(OUT.SCFTUTOR)/scftutor.dep

ifdef DO_DEPEND
dep: $(OUT.SCFTUTOR) $(OUT.SCFTUTOR)/scftutor.dep
$(OUT.SCFTUTOR)/scftutor.dep: $(SRC.ZOO) $(SRC.DOG) $(SRC.WORM)
	$(DO.DEPEND)
else
-include $(OUT.SCFTUTOR)/scftutor.dep
endif

endif # ifeq ($(MAKESECTION),targets)
