# This is a subinclude file used to define the rules needed
# to build the standard console input plug-in.

# Plug-in description
DESCRIPTION.csconin = Crystal Space standard input console

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csconin      Make the $(DESCRIPTION.csconin)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconin csconinclean
all plugins: csconin

csconin:
	$(MAKE_TARGET) MAKE_DLL=yes
csconinclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/console/input/standard

ifeq ($(USE_PLUGINS),yes)
  CSCONIN = $(OUTDLL)/csconin$(DLL)
  LIB.CSCONIN = $(foreach d,$(DEP.CSCONIN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSCONIN)
else
  CSCONIN = $(OUT)/$(LIB_PREFIX)csconin$(LIB)
  DEP.EXE += $(CSCONIN)
  SCF.STATIC += csconin
  TO_INSTALL.STATIC_LIBS += $(CSCONIN)
endif

INF.CSCONIN = $(SRCDIR)/plugins/console/input/standard/csconin.csplugin
INC.CSCONIN = $(wildcard $(addprefix $(SRCDIR)/,plugins/console/input/standard/*.h))
SRC.CSCONIN = $(wildcard $(addprefix $(SRCDIR)/,plugins/console/input/standard/*.cpp))
OBJ.CSCONIN = $(addprefix $(OUT)/,$(notdir $(SRC.CSCONIN:.cpp=$O)))
DEP.CSCONIN = CSUTIL CSUTIL

MSVC.DSP += CSCONIN
DSP.CSCONIN.NAME = csconin
DSP.CSCONIN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconin csconinclean

csconin: $(OUTDIRS) $(CSCONIN)

$(CSCONIN): $(OBJ.CSCONIN) $(LIB.CSCONIN)
	$(DO.PLUGIN)

clean: csconinclean
csconinclean:
	-$(RMDIR) $(CSCONIN) $(OBJ.CSCONIN) $(OUTDLL)/$(notdir $(INF.CSCONIN))

ifdef DO_DEPEND
dep: $(OUTOS)/csconin.dep
$(OUTOS)/csconin.dep: $(SRC.CSCONIN)
	$(DO.DEP)
else
-include $(OUTOS)/csconin.dep
endif

endif # ifeq ($(MAKESECTION),targets)
