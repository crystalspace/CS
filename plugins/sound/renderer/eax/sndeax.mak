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

vpath %.cpp plugins/sound/renderer/eax

ifeq ($(USE_PLUGINS),yes)
  SNDEAX = $(OUTDLL)sndeax$(DLL)
  LIB.SNDEAX = $(foreach d,$(DEP.SNDEAX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDEAX)
else
  SNDEAX = $(OUT)$(LIB_PREFIX)sndeax$(LIB)
  DEP.EXE += $(SNDEAX)
  SCF.STATIC += sndeax
  TO_INSTALL.STATIC_LIBS += $(SNDEAX)
endif

INC.SNDEAX = $(wildcard plugins/sound/renderer/eax/*.h)
SRC.SNDEAX = $(wildcard plugins/sound/renderer/eax/*.cpp)
OBJ.SNDEAX = $(addprefix $(OUT),$(notdir $(SRC.SNDEAX:.cpp=$O)))
DEP.SNDEAX = CSUTIL CSGEOM CSSYS

MSVC.DSP += SNDEAX
DSP.SNDEAX.NAME = sndrdreax
DSP.SNDEAX.TYPE = plugin
DSP.SNDEAX.LIBS = eax.lib eaxguid.lib

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndeax sndeaxclean

sndeax: $(OUTDIRS) $(SNDEAX)

$(SNDEAX): $(OBJ.SNDEAX) $(LIB.SNDEAX)
	$(DO.PLUGIN)

clean: sndeaxclean
sndeaxclean:
	$(RM) $(SNDEAX) $(OBJ.SNDEAX)

ifdef DO_DEPEND
dep: $(OUTOS)sndeax.dep
$(OUTOS)sndeax.dep: $(SRC.SNDEAX)
	$(DO.DEP)
else
-include $(OUTOS)sndeax.dep
endif

endif # ifeq ($(MAKESECTION),targets)