DESCRIPTION.odivx4 = Crystal Space DivX4 codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make odivx4       Make the $(DESCRIPTION.odivx4)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odivx4 odivx4clean
plugins all: odivx4

odivx4:
	$(MAKE_TARGET) MAKE_DLL=yes
odivx4clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/format/codecs/divx4

LIB.EXTERNAL.ODIVX4 = -ldivxdecore
ifeq ($(USE_PLUGINS),yes)
  ODIVX4 = $(OUTDLL)odivx4$(DLL)
  LIB.ODIVX4 = $(foreach d,$(DEP.ODIVX4),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ODIVX4)
else
  ODIVX4 = $(OUT)$(LIB_PREFIX)odivx4$(LIB)
  DEP.EXE += $(ODIVX4)
  SCF.STATIC += odivx4
  TO_INSTALL.STATIC_LIBS += $(ODIVX4)
endif

INC.ODIVX4 = $(wildcard plugins/video/format/codecs/divx4/*.h)
SRC.ODIVX4 = $(wildcard plugins/video/format/codecs/divx4/*.cpp)
OBJ.ODIVX4 = $(addprefix $(OUT),$(notdir $(SRC.ODIVX4:.cpp=$O)))
DEP.ODIVX4 = CSUTIL CSSYS
CFG.ODIVX4 =

TO_INSTALL.CONFIG += $(CFG.ODIVX4)
TO_INSTALL.DATA +=

MSVC.DSP += ODIVX4
DSP.ODIVX4.NAME = odivx4
DSP.ODIVX4.TYPE = plugin
DSP.ODIVX4.LIBS = decore4

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odivx4 odivx4clean
odivx4: $(OUTDIRS) $(ODIVX4)

$(ODIVX4): $(OBJ.ODIVX4) $(LIB.ODIVX4)
	$(DO.PLUGIN) $(LIB.EXTERNAL.ODIVX4)

clean: odivx4clean
odivx4clean:
	-$(RM) $(ODIVX4) $(OBJ.ODIVX4)

ifdef DO_DEPEND
dep: $(OUTOS)odivx4.dep
$(OUTOS)odivx4.dep: $(SRC.ODIVX4)
	$(DO.DEP)
else
-include $(OUTOS)odivx4.dep
endif

endif # ifeq ($(MAKESECTION),targets)
