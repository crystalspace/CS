# Application description
DESCRIPTION.tutsimp2 = Crystal Space tutorial part two (sprite)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutsimp2     Make the $(DESCRIPTION.tutsimp2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutsimp2 tutsimp2clean

all apps: tutsimp2
tutsimp2:
	$(MAKE_TARGET)
tutsimp2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simple2

SIMPLE2.EXE = simple2$(EXE)
INC.SIMPLE2 = $(wildcard apps/tutorial/simple2/*.h)
SRC.SIMPLE2 = $(wildcard apps/tutorial/simple2/*.cpp)
OBJ.SIMPLE2 = $(addprefix $(OUT),$(notdir $(SRC.SIMPLE2:.cpp=$O)))
DEP.SIMPLE2 = CSPARSER CSFX CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.SIMPLE2 = $(foreach d,$(DEP.SIMPLE2),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPLE2.EXE)

MSVC.DSP += SIMPLE2
DSP.SIMPLE2.NAME = simple2
DSP.SIMPLE2.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutsimp2 tutsimp2clean

all: $(SIMPLE2.EXE)
tutsimp2: $(OUTDIRS) $(SIMPLE2.EXE)
clean: tutsimp2clean

$(SIMPLE2.EXE): $(DEP.EXE) $(OBJ.SIMPLE2) $(LIB.SIMPLE2)
	$(DO.LINK.EXE)

tutsimp2clean:
	-$(RM) $(SIMPLE2.EXE) $(OBJ.SIMPLE2)

ifdef DO_DEPEND
dep: $(OUTOS)simple2.dep
$(OUTOS)simple2.dep: $(SRC.SIMPLE2)
	$(DO.DEP)
else
-include $(OUTOS)simple2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
