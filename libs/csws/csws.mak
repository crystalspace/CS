DESCRIPTION.csws = Crystal Space Windowing System library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csws         Make the $(DESCRIPTION.csws)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csws cswsclean
all libs: csws

csws:
	$(MAKE_TARGET)
cswsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSWS.LIB = $(OUT)/$(LIB_PREFIX)csws$(LIB_SUFFIX)

DIR.CSWS = libs/csws
OUT.CSWS = $(OUT)/$(DIR.CSWS)
INC.CSWS = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.CSWS)/*.h $(DIR.CSWS)/skins/default/*.h include/csws/*.h))
SRC.CSWS = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.CSWS)/*.cpp $(DIR.CSWS)/skins/default/*.cpp))
OBJ.CSWS = $(addprefix $(OUT.CSWS)/,$(notdir $(SRC.CSWS:.cpp=$O)))
CFG.CSWS = $(SRCDIR)/data/config/csws.cfg

OUTDIRS += $(OUT.CSWS)

TO_INSTALL.STATIC_LIBS += $(CSWS.LIB)
TO_INSTALL.DATA += $(SRCDIR)/data/csws.zip
TO_INSTALL.CONFIG += $(CFG.CSWS)

MSVC.DSP += CSWS
DSP.CSWS.NAME = csws
DSP.CSWS.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csws cswsclean cswscleandep

csws: $(OUTDIRS) $(CSWS.LIB)

$(OUT.CSWS)/%$O: $(SRCDIR)/$(DIR.CSWS)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.CSWS)/%$O: $(SRCDIR)/$(DIR.CSWS)/skins/default/%.cpp
	$(DO.COMPILE.CPP)

$(CSWS.LIB): $(OBJ.CSWS)
	$(DO.LIBRARY)

clean: cswsclean
cswsclean:
	-$(RMDIR) $(CSWS.LIB) $(OBJ.CSWS)

cleandep: cswscleandep
cswscleandep:
	-$(RM) $(OUT.CSWS)/csws.dep

ifdef DO_DEPEND
dep: $(OUT.CSWS) $(OUT.CSWS)/csws.dep
$(OUT.CSWS)/csws.dep: $(SRC.CSWS)
	$(DO.DEPEND)
else
-include $(OUT.CSWS)/csws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
