
DESCRIPTION.netman = Crystal Space Standard Network Manager

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make netman       Make the $(DESCRIPTION.netman)$"

endif

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: netman netmanclean

all plugins netdrivers: netman

clean: netmanclean

netman: ensocket
	$(MAKE_TARGET) MAKE_DLL=yes
netmanclean:
	$(MAKE_CLEAN)

endif

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION), postdefines)

vpath % plugins/net/netman

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

INC.NETMAN = $(wildcard plugins/net/netman/*.h)
SRC.NETMAN = $(wildcard plugins/net/netman/*.cpp)
OBJ.NETMAN = $(addprefix $(OUT)/,$(notdir $(SRC.NETMAN:.cpp=$O)))
DEP.NETMAN = CSUTIL CSSYS CSUTIL

MSVC.DSP += NETMAN
DSP.NETMAN.NAME = netman
DSP.NETMAN.TYPE = plugin
DSP.NETMAN.DEPEND = CSUTIL CSSYS CSUTIL

endif

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: netman netmanclean

netman: $(NETMAN)

$(NETMAN): $(OBJ.NETMAN) $(LIB.NETMAN)
	$(DO.PLUGIN)

$(OUT)/%$O: plugins/net/netman/%.cpp
	$(DO.COMPILE.CPP)

netmanclean:
	-$(RMDIR) $(NETMAN) $(OBJ.NETMAN)

ifdef DO_DEPEND
dep: $(OUTOS)/netman.dep
else
-include $(OUTOS)/netman.dep
endif

endif

