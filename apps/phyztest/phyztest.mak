# Application description
DESCRIPTION.phyztest = Phyziks example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make phyztest     Make the $(DESCRIPTION.phyztest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phyztest phyzclean

all apps: phyztest
phyztest:
	$(MAKE_APP)
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
DEP.PHYZTEST = CSTOOL CSENGINE CSTOOL CSGFX CSPHYZIK CSUTIL CSSYS CSGEOM \
  CSUTIL
LIB.PHYZTEST = $(foreach d,$(DEP.PHYZTEST),$($d.LIB))

#TO_INSTALL.EXE += $(PHYZTEST.EXE)

MSVC.DSP += PHYZTEST
DSP.PHYZTEST.NAME = phyztest
DSP.PHYZTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.phyztest phyzclean

all: $(PHYZTEST.EXE)
build.phyztest: $(OUTDIRS) $(PHYZTEST.EXE)
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
