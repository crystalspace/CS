DESCRIPTION.fontplex = Crystal Space font server multiplexor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fontplex     Make the $(DESCRIPTION.fontplex)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fontplex fontplexclean
plugins all: fontplex
fontplex:
	$(MAKE_TARGET) MAKE_DLL=yes
fontplexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.FONTPLEX = $(wildcard plugins/font/server/fontplex/*.cpp)
OBJ.FONTPLEX = $(addprefix $(OUT),$(notdir $(SRC.FONTPLEX:.cpp=$O)))
LIB.FONTPLEX = $(CSUTIL.LIB) $(CSSYS.LIB)

ifeq ($(USE_SHARED_PLUGINS),yes)
  FONTPLEX=$(OUTDLL)fontplex$(DLL)
  DEP.FONTPLEX=$(LIB.FONTPLEX)
  TO_INSTALL.DYNAMIC_LIBS += $(FONTPLEX)
else
  FONTPLEX=$(OUT)$(LIB_PREFIX)csfntmngr$(LIB)
  DEP.EXE+=$(FONTPLEX)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_FONTPLEX
  TO_INSTALL.STATIC_LIBS += $(FONTPLEX)
endif
DESCRIPTION.$(FONTPLEX) = $(DESCRIPTION.fontplex)

vpath %.cpp $(sort $(dir $(SRC.FONTPLEX)))

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fontplex fontplexclean
fontplex: $(OUTDIRS) $(FONTPLEX)

$(FONTPLEX): $(OBJ.FONTPLEX) $(DEP.FONTPLEX)
	$(DO.PLUGIN)

clean: fontplexclean
fontplexclean:
	-$(RM) $(FONTPLEX) $(OBJ.FONTPLEX) $(OUTOS)fontplex.dep

ifdef DO_DEPEND
dep: $(OUTOS)fontplex.dep
$(OUTOS)fontplex.dep: $(SRC.FONTPLEX)
	$(DO.DEP)
else
-include $(OUTOS)fontplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)
