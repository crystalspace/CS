DESCRIPTION.odivx = Crystal Space OpenDivX codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make odivx        Make the $(DESCRIPTION.odivx)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odivx odivxclean
all plugins: odivx

odivx:
	$(MAKE_TARGET) MAKE_DLL=yes
odivxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  ODIVX = $(OUTDLL)/odivx$(DLL)
  LIB.ODIVX = $(foreach d,$(DEP.ODIVX),$($d.LIB))
  LIB.ODIVX.LOCAL = $(OPENDIVX.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(ODIVX)
else
  ODIVX = $(OUT)/$(LIB_PREFIX)odivx$(LIB)
  DEP.EXE += $(ODIVX)
  LIBS.EXE += $(OPENDIVX.LFLAGS)
  SCF.STATIC += odivx
  TO_INSTALL.STATIC_LIBS += $(ODIVX)
endif

DIR.ODIVX = plugins/video/format/codecs/opendivx
OUT.ODIVX = $(OUT)/$(DIR.ODIVX)
INF.ODIVX = $(SRCDIR)/$(DIR.ODIVX)/odivx.csplugin
INC.ODIVX = $(wildcard $(SRCDIR)/$(DIR.ODIVX)/*.h)
SRC.ODIVX = $(wildcard $(SRCDIR)/$(DIR.ODIVX)/*.cpp)
OBJ.ODIVX = $(addprefix $(OUT.ODIVX)/,$(notdir $(SRC.ODIVX:.cpp=$O)))
DEP.ODIVX = CSUTIL

OUTDIRS += $(OUT.ODIVX)

MSVC.DSP += ODIVX
DSP.ODIVX.NAME = odivx
DSP.ODIVX.TYPE = plugin
DSP.ODIVX.LIBS = decore

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odivx odivxclean odivxcleandep

odivx: $(OUTDIRS) $(ODIVX)

$(OUT.ODIVX)/%$O: $(SRCDIR)/$(DIR.ODIVX)/%.cpp
	$(DO.COMPILE.CPP) (OPENDIVX.CFLAGS)

$(ODIVX): $(OBJ.ODIVX) $(LIB.ODIVX)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.ODIVX.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: odivxclean
odivxclean:
	-$(RMDIR) $(ODIVX) $(OBJ.ODIVX) $(OUTDLL)/$(notdir $(INF.ODIVX))

cleandep: odivxcleandep
odivxcleandep:
	-$(RM) $(OUT.ODIVX)/odivx.dep

ifdef DO_DEPEND
dep: $(OUT.ODIVX) $(OUT.ODIVX)/odivx.dep
$(OUT.ODIVX)/odivx.dep: $(SRC.ODIVX)
	$(DO.DEPEND1) $(OPENDIVX.CFLAGS) $(DO.DEPEND2)
else
-include $(OUT.ODIVX)/odivx.dep
endif

endif # ifeq ($(MAKESECTION),targets)
