# Application description
DESCRIPTION.tutsimpvs = Crystal Space tutorial part one

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutsimpvs    Make the $(DESCRIPTION.tutsimpvs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutsimpvs tutsimpvsclean

all apps: tutsimpvs
tutsimpvs:
	$(MAKE_TARGET)
tutsimpvsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simpvs

SIMPVS.EXE = simpvs$(EXE)
INC.SIMPVS = $(wildcard apps/tutorial/simpvs/*.h)
SRC.SIMPVS = $(wildcard apps/tutorial/simpvs/*.cpp)
OBJ.SIMPVS = $(addprefix $(OUT),$(notdir $(SRC.SIMPVS:.cpp=$O)))
DEP.SIMPVS = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.SIMPVS = $(foreach d,$(DEP.SIMPVS),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPVS.EXE)

MSVC.DSP += SIMPVS
DSP.SIMPVS.NAME = simpvs
DSP.SIMPVS.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutsimpvs tutsimpvsclean

all: $(SIMPVS.EXE)
tutsimpvs: $(OUTDIRS) $(SIMPVS.EXE)
clean: tutsimpvsclean

$(SIMPVS.EXE): $(DEP.EXE) $(OBJ.SIMPVS) $(LIB.SIMPVS)
	$(DO.LINK.EXE)

tutsimpvsclean:
	-$(RM) $(SIMPVS.EXE) $(OBJ.SIMPVS)

ifdef DO_DEPEND
dep: $(OUTOS)simpvs.dep
$(OUTOS)simpvs.dep: $(SRC.SIMPVS)
	$(DO.DEP)
else
-include $(OUTOS)simpvs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
