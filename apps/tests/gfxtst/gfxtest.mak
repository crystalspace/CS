# Application description
DESCRIPTION.gfxtst = Crystal Space Graphics Loader library test

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make gfxtst       Make the $(DESCRIPTION.gfxtst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gfxtst

all apps: gfxtst
gfxtst:
	$(MAKE_TARGET)
gfxtstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/gfxtst

# csutil listed twice because of circular dependency between cssys and csutil.
LIB.GFXTEST = $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)

GFXTEST.EXE = gfxtstest$(EXE)
SRC.GFXTEST += apps/tests/gfxtst/gfxtest.cpp
OBJ.GFXTEST = $(addprefix $(OUT),$(notdir $(SRC.GFXTEST:.cpp=$O)))

DESCRIPTION.$(GFXTEST.EXE) = $(DESCRIPTION.gfxtst)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gfxtst gfxtstclean

gfxtst: $(OUTDIRS) $(GFXTEST.EXE)
clean: gfxtstclean

$(GFXTEST.EXE): $(OBJ.GFXTEST) $(LIB.GFXTEST)
	$(DO.LINK.CONSOLE.EXE)

gfxtstclean:
	-$(RM) $(GFXTEST.EXE) $(OBJ.GFXTEST)

ifdef DO_DEPEND
depend: $(OUTOS)gfxtst.dep
$(OUTOS)gfxtst.dep: $(SRC.GFXTEST)
	$(DO.DEP)
else
-include $(OUTOS)gfxtst.dep
endif

endif # ifeq ($(MAKESECTION),targets)
