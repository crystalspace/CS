# Application description
DESCRIPTION.simplecloth = Crystal Space tutorial part two, sprite

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simplecloth      Make the $(DESCRIPTION.simplecloth)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simplecloth simpleclothclean

all apps: simplecloth
simplecloth:
	$(MAKE_APP)
simpleclothclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simplecloth

SIMPLECLOTH.EXE = simplecloth$(EXE)
INC.SIMPLECLOTH = $(wildcard apps/tutorial/simplecloth/*.h)
SRC.SIMPLECLOTH = $(wildcard apps/tutorial/simplecloth/*.cpp)
OBJ.SIMPLECLOTH = $(addprefix $(OUT)/,$(notdir $(SRC.SIMPLECLOTH:.cpp=$O)))
DEP.SIMPLECLOTH = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.SIMPLECLOTH = $(foreach d,$(DEP.SIMPLECLOTH),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPLECLOTH.EXE)

MSVC.DSP += SIMPLECLOTH
DSP.SIMPLECLOTH.NAME = simplecloth
DSP.SIMPLECLOTH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simplecloth simpleclothclean

all: $(SIMPLECLOTH.EXE)
build.simplecloth: $(OUTDIRS) $(SIMPLECLOTH.EXE)
clean: simpleclothclean

$(SIMPLECLOTH.EXE): $(DEP.EXE) $(OBJ.SIMPLECLOTH) $(LIB.SIMPLECLOTH)
	$(DO.LINK.EXE)

simpleclothclean:
	-$(RM) $(SIMPLECLOTH.EXE) $(OBJ.SIMPLECLOTH)

ifdef DO_DEPEND
dep: $(OUTOS)/simplecloth.dep
$(OUTOS)/simplecloth.dep: $(SRC.SIMPLECLOTH)
	$(DO.DEP)
else
-include $(OUTOS)/simplecloth.dep
endif

endif # ifeq ($(MAKESECTION),targets)
