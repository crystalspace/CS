DESCRIPTION.odivx = Crystal Space OpenDivX codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make odivx        Make the $(DESCRIPTION.odivx)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odivx odivxclean
plugins all: odivx

odivx:
	$(MAKE_TARGET) MAKE_DLL=yes
odivxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/format/codecs/opendivx

LIB.EXTERNAL.ODIVX = -ldivxdecore
ifeq ($(USE_PLUGINS),yes)
  ODIVX = $(OUTDLL)odivx$(DLL)
  LIB.ODIVX = $(foreach d,$(DEP.ODIVX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ODIVX)
else
  ODIVX = $(OUT)$(LIB_PREFIX)odivx$(LIB)
  DEP.EXE += $(ODIVX)
  SCF.STATIC += odivx
  TO_INSTALL.STATIC_LIBS += $(ODIVX)
endif

INC.ODIVX = $(wildcard plugins/video/format/codecs/opendivx/*.h)
SRC.ODIVX = $(wildcard plugins/video/format/codecs/opendivx/*.cpp)
OBJ.ODIVX = $(addprefix $(OUT),$(notdir $(SRC.ODIVX:.cpp=$O)))
DEP.ODIVX = CSGFXLDR CSUTIL CSSYS
CFG.ODIVX = 

TO_INSTALL.CONFIG += $(CFG.ODIVX)
TO_INSTALL.DATA += 

MSVC.DSP += ODIVX
DSP.ODIVX.NAME = odivx
DSP.ODIVX.TYPE = plugin
DSP.ODIVX.LIBS = decore

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odivx odivxclean
odivx: $(OUTDIRS) $(ODIVX)

$(ODIVX): $(OBJ.ODIVX) $(LIB.ODIVX)
	$(DO.PLUGIN) $(LIB.EXTERNAL.ODIVX)

clean: odivxclean
odivxclean:
	-$(RM) $(ODIVX) $(OBJ.ODIVX)

ifdef DO_DEPEND
dep: $(OUTOS)odivx.dep
$(OUTOS)odivx.dep: $(SRC.ODIVX)
	$(DO.DEP)
else
-include $(OUTOS)odivx.dep
endif

endif # ifeq ($(MAKESECTION),targets)
