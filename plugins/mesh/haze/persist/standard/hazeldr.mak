DESCRIPTION.hazeldr = Haze mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make hazeldr      Make the $(DESCRIPTION.hazeldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: hazeldr hazeldrclean
plugins meshes all: hazeldr

hazeldrclean:
	$(MAKE_CLEAN)
hazeldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/haze/persist/standard

ifeq ($(USE_PLUGINS),yes)
  HAZELDR = $(OUTDLL)/hazeldr$(DLL)
  LIB.HAZELDR = $(foreach d,$(DEP.HAZELDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(HAZELDR)
else
  HAZELDR = $(OUT)/$(LIB_PREFIX)hazeldr$(LIB)
  DEP.EXE += $(HAZELDR)
  SCF.STATIC += hazeldr
  TO_INSTALL.STATIC_LIBS += $(HAZELDR)
endif

INF.HAZELDR = $(SRCDIR)/plugins/mesh/haze/persist/standard/hazeldr.csplugin
INC.HAZELDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/haze/persist/standard/*.h))
SRC.HAZELDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/haze/persist/standard/*.cpp))
OBJ.HAZELDR = $(addprefix $(OUT)/,$(notdir $(SRC.HAZELDR:.cpp=$O)))
DEP.HAZELDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += HAZELDR
DSP.HAZELDR.NAME = hazeldr
DSP.HAZELDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: hazeldr hazeldrclean
hazeldr: $(OUTDIRS) $(HAZELDR)

$(HAZELDR): $(OBJ.HAZELDR) $(LIB.HAZELDR)
	$(DO.PLUGIN)

clean: hazeldrclean
hazeldrclean:
	-$(RMDIR) $(HAZELDR) $(OBJ.HAZELDR) $(OUTDLL)/$(notdir $(INF.HAZELDR))

ifdef DO_DEPEND
dep: $(OUTOS)/hazeldr.dep
$(OUTOS)/hazeldr.dep: $(SRC.HAZELDR)
	$(DO.DEP)
else
-include $(OUTOS)/hazeldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
