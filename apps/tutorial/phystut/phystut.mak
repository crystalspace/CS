# Application description
DESCRIPTION.tutphys = Crystal Space physics tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutphys     Make the $(DESCRIPTION.tutphys)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutphys tutphysclean

all apps: tutphys
tutphys:
	$(MAKE_TARGET)
tutphysclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/phystut

PHYS.EXE = phys$(EXE)
INC.PHYS = $(wildcard apps/tutorial/phystut/*.h)
SRC.PHYS = $(wildcard apps/tutorial/phystut/*.cpp)
OBJ.PHYS = $(addprefix $(OUT),$(notdir $(SRC.PHYS:.cpp=$O)))
DEP.PHYS = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.PHYS = $(foreach d,$(DEP.PHYS),$($d.LIB))

#TO_INSTALL.EXE += $(PHYS.EXE)

MSVC.DSP += PHYS
DSP.PHYS.NAME = phys
DSP.PHYS.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutphys tutphysclean

all: $(PHYS.EXE)
tutphys: $(OUTDIRS) $(PHYS.EXE)
clean: tutphysclean

$(PHYS.EXE): $(DEP.EXE) $(OBJ.PHYS) $(LIB.PHYS)
	$(DO.LINK.EXE)

tutphysclean:
	-$(RM) $(PHYS.EXE) $(OBJ.PHYS)

ifdef DO_DEPEND
dep: $(OUTOS)phys.dep
$(OUTOS)phys.dep: $(SRC.PHYS)
	$(DO.DEP)
else
-include $(OUTOS)phys.dep
endif

endif # ifeq ($(MAKESECTION),targets)
