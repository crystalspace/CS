DESCRIPTION.netman = Crystal Space network manager

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make netman       Make the $(DESCRIPTION.netman)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: netman netmanclean
all plugins netdrivers: netman

netman:
	$(MAKE_TARGET) MAKE_DLL=yes
netmanclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/net/manager

ifeq ($(USE_PLUGINS),yes)
  NETMAN = $(OUTDLL)/netman$(DLL)
  LIB.NETMAN = $(foreach d,$(DEP.NETMAN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NETMAN)
else
  NETMAN = $(OUT)/$(LIBPREFIX)netman$(LIB)
  DEP.EXE += $(NETMAN)
  SCF.STATIC += netman
  TO_INSTALL.STATIC_LIBS += $(NETMAN)
endif

INF.NETMAN = $(SRCDIR)/plugins/net/manager/netman.csplugin
INC.NETMAN = $(wildcard $(addprefix $(SRCDIR)/,plugins/net/manager/*.h))
SRC.NETMAN = $(wildcard $(addprefix $(SRCDIR)/,plugins/net/manager/*.cpp))
OBJ.NETMAN = $(addprefix $(OUT)/,$(notdir $(SRC.NETMAN:.cpp=$O)))
DEP.NETMAN = CSUTIL CSUTIL

MSVC.DSP += NETMAN
DSP.NETMAN.NAME = netman
DSP.NETMAN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: netman netmanclean

netman: $(OUTDIRS) $(NETMAN)

$(NETMAN): $(OBJ.NETMAN) $(LIB.NETMAN)
	$(DO.PLUGIN)

clean: netmanclean
netmanclean:
	-$(RMDIR) $(NETMAN) $(OBJ.NETMAN) $(OUTDLL)/$(notdir $(INF.NETMAN))

ifdef DO_DEPEND
dep: $(OUTOS)/netman.dep
$(OUTOS)/netman.dep: $(SRC.NETMAN)
	$(DO.DEP)
else
-include $(OUTOS)/netman.dep
endif

endif # ifeq ($(MAKESECTION),targets)
