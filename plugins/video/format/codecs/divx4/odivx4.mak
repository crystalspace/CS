DESCRIPTION.odivx4 = Crystal Space OpenDivX4 codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make odivx4       Make the $(DESCRIPTION.odivx4)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odivx4 odivx4clean
all plugins: odivx4

odivx4:
	$(MAKE_TARGET) MAKE_DLL=yes
odivx4clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  ODIVX4 = $(OUTDLL)/odivx4$(DLL)
  LIB.ODIVX4 = $(foreach d,$(DEP.ODIVX4),$($d.LIB))
  LIB.ODIVX4.LOCAL = $(OPENDIVX4.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(ODIVX4)
else
  ODIVX4 = $(OUT)/$(LIB_PREFIX)odivx4$(LIB)
  DEP.EXE += $(ODIVX4)
  LIBS.EXE += $(OPENDIVX4.LFLAGS)
  SCF.STATIC += odivx4
  TO_INSTALL.STATIC_LIBS += $(ODIVX4)
endif

DIR.ODIVX4 = plugins/video/format/codecs/divx4
OUT.ODIVX4 = $(OUT)/$(DIR.ODIVX4)
INF.ODIVX4 = $(SRCDIR)/$(DIR.ODIVX4)/odivx4.csplugin
INC.ODIVX4 = $(wildcard $(SRCDIR)/$(DIR.ODIVX4)/*.h)
SRC.ODIVX4 = $(wildcard $(SRCDIR)/$(DIR.ODIVX4)/*.cpp)
OBJ.ODIVX4 = $(addprefix $(OUT.ODIVX4)/,$(notdir $(SRC.ODIVX4:.cpp=$O)))
DEP.ODIVX4 = CSUTIL

OUTDIRS += $(OUT.ODIVX4)

MSVC.DSP += ODIVX4
DSP.ODIVX4.NAME = odivx4
DSP.ODIVX4.TYPE = plugin
DSP.ODIVX4.LIBS = decore4

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odivx4 odivx4clean odivx4cleandep

odivx4: $(OUTDIRS) $(ODIVX4)

$(OUT.ODIVX4)/%$O: $(SRCDIR)/$(DIR.ODIVX4)/%.cpp
	$(DO.COMPILE.CPP) $(OPENDIVX4.CFLAGS)

$(ODIVX4): $(OBJ.ODIVX4) $(LIB.ODIVX4)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.ODIVX4.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: odivx4clean
odivx4clean:
	-$(RMDIR) $(ODIVX4) $(OBJ.ODIVX4) $(OUTDLL)/$(notdir $(INF.ODIVX4))

cleandep: odivx4cleandep
odivx4cleandep:
	-$(RM) $(OUT.ODIVX4)/odivx4.dep

ifdef DO_DEPEND
dep: $(OUT.ODIVX4) $(OUT.ODIVX4)/odivx4.dep
$(OUT.ODIVX4)/odivx4.dep: $(SRC.ODIVX4)
	$(DO.DEPEND1) $(OPENDIVX4.CFLAGS) $(DO.DEPEND2)
else
-include $(OUT.ODIVX4)/odivx4.dep
endif

endif # ifeq ($(MAKESECTION),targets)
