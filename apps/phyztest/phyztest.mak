# Application description
DESCRIPTION.phyz = Phyziks example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make phyz         Make the $(DESCRIPTION.phyz)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phyz phyzclean

all apps: phyz
phyz:
	$(MAKE_TARGET)
phyzclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/phyztest apps/support

PHYZTEST.EXE = phyztest$(EXE)
INC.PHYZTEST = $(wildcard apps/phyztest/*.h)
SRC.PHYZTEST = $(wildcard apps/phyztest/*.cpp)
OBJ.PHYZTEST = $(addprefix $(OUT),$(notdir $(SRC.PHYZTEST:.cpp=$O)))
DEP.PHYZTEST = CSPARSER CSFX CSENGINE CSFX CSGFX CSPHYZIK CSUTIL CSSYS CSGEOM \
  CSUTIL
LIB.PHYZTEST = $(foreach d,$(DEP.PHYZTEST),$($d.LIB))

#TO_INSTALL.EXE += $(PHYZTEST.EXE)

MSVC.DSP += PHYZTEST
DSP.PHYZTEST.NAME = phyztest
DSP.PHYZTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: phyz phyzclean

all: $(PHYZTEST.EXE)
phyz: $(OUTDIRS) $(PHYZTEST.EXE)
clean: phyzclean

$(PHYZTEST.EXE): $(DEP.EXE) $(OBJ.PHYZTEST) $(LIB.PHYZTEST)
	$(DO.LINK.EXE)

phyzclean:
	-$(RM) $(PHYZTEST.EXE) $(OBJ.PHYZTEST)

ifdef DO_DEPEND
dep: $(OUTOS)phyztest.dep
$(OUTOS)phyztest.dep: $(SRC.PHYZTEST)
	$(DO.DEP)
else
-include $(OUTOS)phyztest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
