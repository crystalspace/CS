# This is a subinclude file used to define the rules needed
# to build the dynavis plug-in.

# Driver description
DESCRIPTION.dynavis = Crystal Space Dynamic Visibility System

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make dynavis      Make the $(DESCRIPTION.dynavis)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: dynavis dynavisclean
all plugins: dynavis

dynavis:
	$(MAKE_TARGET) MAKE_DLL=yes
dynavisclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/culling/dynavis

ifeq ($(USE_PLUGINS),yes)
  DYNAVIS = $(OUTDLL)/dynavis$(DLL)
  LIB.DYNAVIS = $(foreach d,$(DEP.DYNAVIS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(DYNAVIS)
else
  DYNAVIS = $(OUT)/$(LIB_PREFIX)dynavis$(LIB)
  DEP.EXE += $(DYNAVIS)
  SCF.STATIC += dynavis
  TO_INSTALL.STATIC_LIBS += $(DYNAVIS)
endif

INF.DYNAVIS = $(SRCDIR)/plugins/culling/dynavis/dynavis.csplugin
INC.DYNAVIS = $(wildcard $(addprefix $(SRCDIR)/,plugins/culling/dynavis/*.h))
SRC.DYNAVIS = $(wildcard $(addprefix $(SRCDIR)/,plugins/culling/dynavis/*.cpp))
OBJ.DYNAVIS = $(addprefix $(OUT)/,$(notdir $(SRC.DYNAVIS:.cpp=$O)))
DEP.DYNAVIS = CSUTIL CSGEOM CSUTIL CSUTIL

MSVC.DSP += DYNAVIS
DSP.DYNAVIS.NAME = dynavis
DSP.DYNAVIS.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: dynavis dynavisclean

dynavis: $(OUTDIRS) $(DYNAVIS)

$(DYNAVIS): $(OBJ.DYNAVIS) $(LIB.DYNAVIS)
	$(DO.PLUGIN)

clean: dynavisclean
dynavisclean:
	-$(RMDIR) $(DYNAVIS) $(OBJ.DYNAVIS) $(OUTDLL)/$(notdir $(INF.DYNAVIS))

ifdef DO_DEPEND
dep: $(OUTOS)/dynavis.dep
$(OUTOS)/dynavis.dep: $(SRC.DYNAVIS)
	$(DO.DEP)
else
-include $(OUTOS)/dynavis.dep
endif

endif # ifeq ($(MAKESECTION),targets)
