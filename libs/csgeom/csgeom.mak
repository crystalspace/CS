DESCRIPTION.csgeom = Crystal Space geometry library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP+=$(NEWLINE)echo $"  make csgeom       Make the $(DESCRIPTION.csgeom)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgeom csgeomclean
all libs: csgeom

csgeom:
	$(MAKE_TARGET)
csgeomclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSGEOM.LIB = $(OUT)/$(LIB_PREFIX)csgeom$(LIB_SUFFIX)

DIR.CSGEOM = libs/csgeom
OUT.CSGEOM = $(OUT)/$(DIR.CSGEOM)
INC.CSGEOM = \
  $(wildcard $(SRCDIR)/$(DIR.CSGEOM)/*.h $(SRCDIR)/include/csgeom/*.h)
SRC.CSGEOM = $(wildcard $(SRCDIR)/$(DIR.CSGEOM)/*.cpp)
OBJ.CSGEOM = $(addprefix $(OUT.CSGEOM)/,$(notdir $(SRC.CSGEOM:.cpp=$O)))

OUTDIRS += $(OUT.CSGEOM)

TO_INSTALL.STATIC_LIBS += $(CSGEOM.LIB)

MSVC.DSP += CSGEOM
DSP.CSGEOM.NAME = csgeom
DSP.CSGEOM.TYPE = library
DSP.CSGEOM.RESOURCES = $(wildcard $(SRCDIR)/libs/csgeom/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgeom csgeomclean csgeomcleandep

csgeom: $(OUTDIRS) $(CSGEOM.LIB)

$(OUT.CSGEOM)/%$O: $(SRCDIR)/$(DIR.CSGEOM)/%.cpp
	$(DO.COMPILE.CPP)

$(CSGEOM.LIB): $(OBJ.CSGEOM)
	$(DO.LIBRARY)

clean: csgeomclean
csgeomclean:
	-$(RMDIR) $(CSGEOM.LIB) $(OBJ.CSGEOM)

cleandep: csgeomcleandep
csgeomcleandep:
	-$(RM) $(OUT.CSGEOM)/csgeom.dep

ifdef DO_DEPEND
dep: $(OUT.CSGEOM) $(OUT.CSGEOM)/csgeom.dep
$(OUT.CSGEOM)/csgeom.dep: $(SRC.CSGEOM)
	$(DO.DEPEND)
else
-include $(OUT.CSGEOM)/csgeom.dep
endif

endif # ifeq ($(MAKESECTION),targets)
