# Plug-in description
DESCRIPTION.sndeax = Crystal Space EAX sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndeax      Make the $(DESCRIPTION.sndeax)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndeax sndeaxclean
all plugins drivers snddrivers: sndeax

sndeax:
	$(MAKE_TARGET) MAKE_DLL=yes
sndeaxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/renderer/eax

ifeq ($(USE_PLUGINS),yes)
  SNDEAX = $(OUTDLL)/sndeax$(DLL)
  LIB.SNDEAX = $(foreach d,$(DEP.SNDEAX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDEAX)
else
  SNDEAX = $(OUT)/$(LIB_PREFIX)sndeax$(LIB)
  DEP.EXE += $(SNDEAX)
  SCF.STATIC += sndeax
  TO_INSTALL.STATIC_LIBS += $(SNDEAX)
endif

INF.SNDEAX = $(SRCDIR)/plugins/sound/renderer/eax/sndeax.csplugin
INC.SNDEAX = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/renderer/eax/*.h) \)
  $(wildcard plugins/sound/renderer/common/*.h)
SRC.SNDEAX = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/renderer/eax/*.cpp) \)
  $(wildcard plugins/sound/renderer/common/*.cpp)
OBJ.SNDEAX = $(addprefix $(OUT)/,$(notdir $(SRC.SNDEAX:.cpp=$O)))
DEP.SNDEAX = CSUTIL CSGEOM CSUTIL

MSVC.DSP += SNDEAX
DSP.SNDEAX.NAME = sndeax
DSP.SNDEAX.TYPE = plugin
DSP.SNDEAX.LIBS = dsound eax eaxguid

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndeax sndeaxclean

sndeax: $(OUTDIRS) $(SNDEAX)

$(SNDEAX): $(OBJ.SNDEAX) $(LIB.SNDEAX)
	$(DO.PLUGIN)

clean: sndeaxclean
sndeaxclean:
	-$(RMDIR) $(SNDEAX) $(OBJ.SNDEAX) $(OUTDLL)/$(notdir $(INF.SNDEAX))

ifdef DO_DEPEND
dep: $(OUTOS)/sndeax.dep
$(OUTOS)/sndeax.dep: $(SRC.SNDEAX)
	$(DO.DEP)
else
-include $(OUTOS)/sndeax.dep
endif

endif # ifeq ($(MAKESECTION),targets)
