# Application description
DESCRIPTION.tutsimpcd = Crystal Space CD tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tutsimpcd    Make the $(DESCRIPTION.tutsimpcd)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tutsimpcd tutsimpcdclean

all apps: tutsimpcd
tutsimpcd:
	$(MAKE_TARGET)
tutsimpcdclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simpcd

SIMPCD.EXE = simpcd$(EXE)
INC.SIMPCD = $(wildcard apps/tutorial/simpcd/*.h)
SRC.SIMPCD = $(wildcard apps/tutorial/simpcd/*.cpp)
OBJ.SIMPCD = $(addprefix $(OUT),$(notdir $(SRC.SIMPCD:.cpp=$O)))
DEP.SIMPCD = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.SIMPCD = $(foreach d,$(DEP.SIMPCD),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPCD.EXE)

MSVC.DSP += SIMPCD
DSP.SIMPCD.NAME = simpcd
DSP.SIMPCD.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tutsimpcd tutsimpcdclean

all: $(SIMPCD.EXE)
tutsimpcd: $(OUTDIRS) $(SIMPCD.EXE)
clean: tutsimpcdclean

$(SIMPCD.EXE): $(DEP.EXE) $(OBJ.SIMPCD) $(LIB.SIMPCD)
	$(DO.LINK.EXE)

tutsimpcdclean:
	-$(RM) $(SIMPCD.EXE) $(OBJ.SIMPCD)

ifdef DO_DEPEND
dep: $(OUTOS)simpcd.dep
$(OUTOS)simpcd.dep: $(SRC.SIMPCD)
	$(DO.DEP)
else
-include $(OUTOS)simpcd.dep
endif

endif # ifeq ($(MAKESECTION),targets)
