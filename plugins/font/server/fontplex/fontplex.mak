DESCRIPTION.fontplex = Crystal Space multiplexing font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fontplex     Make the $(DESCRIPTION.fontplex)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fontplex fontplexclean
all plugins: fontplex
fontplex:
	$(MAKE_TARGET) MAKE_DLL=yes
fontplexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/font/server/fontplex

ifeq ($(USE_PLUGINS),yes)
  FONTPLEX = $(OUTDLL)fontplex$(DLL)
  LIB.FONTPLEX = $(foreach d,$(DEP.FONTPLEX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FONTPLEX)
else
  FONTPLEX = $(OUT)$(LIB_PREFIX)fontplex$(LIB)
  DEP.EXE += $(FONTPLEX)
  SCF.STATIC += fontplex
  TO_INSTALL.STATIC_LIBS += $(FONTPLEX)
endif

INC.FONTPLEX = $(wildcard plugins/font/server/fontplex/*.h)
SRC.FONTPLEX = $(wildcard plugins/font/server/fontplex/*.cpp)
OBJ.FONTPLEX = $(addprefix $(OUT),$(notdir $(SRC.FONTPLEX:.cpp=$O)))
DEP.FONTPLEX = CSUTIL CSSYS CSUTIL

MSVC.DSP += FONTPLEX
DSP.FONTPLEX.NAME = fontplex
DSP.FONTPLEX.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fontplex fontplexclean
fontplex: $(OUTDIRS) $(FONTPLEX)

$(FONTPLEX): $(OBJ.FONTPLEX) $(LIB.FONTPLEX)
	$(DO.PLUGIN)

clean: fontplexclean
fontplexclean:
	-$(RM) $(FONTPLEX) $(OBJ.FONTPLEX)

ifdef DO_DEPEND
dep: $(OUTOS)fontplex.dep
$(OUTOS)fontplex.dep: $(SRC.FONTPLEX)
	$(DO.DEP)
else
-include $(OUTOS)fontplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)
