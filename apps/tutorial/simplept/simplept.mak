# Application description
DESCRIPTION.tutsimplept = Crystal Space procedural textures tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutsimplept  Make the $(DESCRIPTION.tutsimplept)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutsimplept tutsimpleptclean

all apps: tutsimplept
tutsimplept:
	$(MAKE_TARGET)
tutsimpleptclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simplept

SIMPLEPT.EXE = simplept$(EXE)
INC.SIMPLEPT = $(wildcard apps/tutorial/simplept/*.h)
SRC.SIMPLEPT = $(wildcard apps/tutorial/simplept/*.cpp)
OBJ.SIMPLEPT = $(addprefix $(OUT),$(notdir $(SRC.SIMPLEPT:.cpp=$O)))
DEP.SIMPLEPT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.SIMPLEPT = $(foreach d,$(DEP.SIMPLEPT),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPLEPT.EXE)

MSVC.DSP += SIMPLEPT
DSP.SIMPLEPT.NAME = simplept
DSP.SIMPLEPT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutsimplept tutsimpleptclean

all: $(SIMPLEPT.EXE)
tutsimplept: $(OUTDIRS) $(SIMPLEPT.EXE)
clean: tutsimpleptclean

$(SIMPLEPT.EXE): $(DEP.EXE) $(OBJ.SIMPLEPT) $(LIB.SIMPLEPT)
	$(DO.LINK.EXE)

tutsimpleptclean:
	-$(RM) $(SIMPLEPT.EXE) $(OBJ.SIMPLEPT)

ifdef DO_DEPEND
dep: $(OUTOS)simplept.dep
$(OUTOS)simplept.dep: $(SRC.SIMPLEPT)
	$(DO.DEP)
else
-include $(OUTOS)simplept.dep
endif

endif # ifeq ($(MAKESECTION),targets)
