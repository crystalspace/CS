DESCRIPTION.csvosa3dl = Virtual Object System abstract 3D layer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csvosa3dl    Make the $(DESCRIPTION.csvosa3dl)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csvosa3dl csvosa3dlclean
all plugins: csvosa3dl

csvosa3dl:
	$(MAKE_TARGET) MAKE_DLL=yes
csvosa3dlclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSVOSA3DL.CFLAGS = $(CFLAGS.D)_REENTRANT $(CXXFLAGS.EXCEPTIONS.ENABLE)

ifeq ($(USE_PLUGINS),yes)
  CSVOSA3DL = $(OUTDLL)/csvosa3dl$(DLL)
  LIB.CSVOSA3DL = $(foreach d,$(DEP.CSVOSA3DL),$($d.LIB))
  LIB.CSVOSA3DL = $(VOS.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(CSVOSA3DL)
else
  CSVOSA3DL = $(OUT)/$(LIB_PREFIX)csvosa3dl$(LIB)
  DEP.EXE += $(CSVOSA3DL)
  LIBS.EXE += $(VOS.LFLAGS)
  SCF.STATIC += csvosa3dl
  TO_INSTALL.STATIC_LIBS += $(CSVOSA3DL)
endif

DIR.CSVOSA3DL = plugins/net/vos
OUT.CSVOSA3DL = $(OUT)/$(DIR.CSVOSA3DL)
INF.CSVOSA3DL = $(SRCDIR)/$(DIR.CSVOSA3DL)/csvosa3dl.csplugin
INC.CSVOSA3DL = $(wildcard $(SRCDIR)/$(DIR.CSVOSA3DL)/*.h)
SRC.CSVOSA3DL = $(wildcard $(SRCDIR)/$(DIR.CSVOSA3DL)/*.cpp)
OBJ.CSVOSA3DL = \
  $(addprefix $(OUT.CSVOSA3DL)/,$(notdir $(SRC.CSVOSA3DL:.cpp=$O)))
DEP.CSVOSA3DL = CSUTIL CSGEOM CSGFX CSTOOL

OUTDIRS += $(OUT.CSVOSA3DL)

MSVC.DSP += CSVOSA3DL
DSP.CSVOSA3DL.NAME = csvosa3dl
DSP.CSVOSA3DL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csvosa3dl csvosa3dlclean csvosa3dlcleandep

csvosa3dl: $(OUTDIRS) $(CSVOSA3DL)

$(OUT.CSVOSA3DL)/%$O: $(SRCDIR)/$(DIR.CSVOSA3DL)/%.cpp
	$(DO.COMPILE.CPP) $(VOS.CFLAGS) $(CSVOSA3DL.CFLAGS)

$(CSVOSA3DL): $(OBJ.CSVOSA3DL) $(LIB.CSVOSA3DL)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSVOSA3DL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: csvosa3dlclean
csvosa3dlclean:
	-$(RMDIR) $(CSVOSA3DL) $(OBJ.CSVOSA3DL) \
	$(OUTDLL)/$(notdir $(INF.CSVOSA3DL))

cleandep: csvosa3dlcleandep
csvosa3dlcleandep:
	-$(RM) $(OUT.CSVOSA3DL)/csvosa3dl.dep

ifdef DO_DEPEND
dep: $(OUT.CSVOSA3DL) $(OUT.CSVOSA3DL)/csvosa3dl.dep
$(OUT.CSVOSA3DL)/csvosa3dl.dep: $(SRC.CSVOSA3DL)
	$(DO.DEPEND1) \
	$(VOS.CFLAGS) $(CSVOSA3DL.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.CSVOSA3DL)/csvosa3dl.dep
endif

endif # ifeq ($(MAKESECTION),targets)
