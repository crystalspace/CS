DESCRIPTION.csfont = Crystal Space default font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csfont       Make the $(DESCRIPTION.csfont)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfont csfontclean
all plugins: csfont
csfont:
	$(MAKE_TARGET) MAKE_DLL=yes
csfontclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/font/server/csfont

ifeq ($(USE_PLUGINS),yes)
  CSFONT = $(OUTDLL)csfont$(DLL)
  LIB.CSFONT = $(foreach d,$(DEP.CSFONT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSFONT)
else
  CSFONT = $(OUT)$(LIB_PREFIX)csfont$(LIB)
  DEP.EXE += $(CSFONT)
  SCF.STATIC += csfont
  TO_INSTALL.STATIC_LIBS += $(CSFONT)
endif

INC.CSFONT = $(wildcard plugins/font/server/csfont/*.h)
SRC.CSFONT = $(wildcard plugins/font/server/csfont/*.cpp)
OBJ.CSFONT = $(addprefix $(OUT),$(notdir $(SRC.CSFONT:.cpp=$O)))
DEP.CSFONT = CSUTIL CSSYS

MSVC.DSP += CSFONT
DSP.CSFONT.NAME = csfont
DSP.CSFONT.TYPE = plugin
DSP.CSFONT.RESOURCES = $(wildcard plugins/font/server/csfont/*.fnt)

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csfont csfontclean
csfont: $(OUTDIRS) $(CSFONT)

$(CSFONT): $(OBJ.CSFONT) $(LIB.CSFONT)
	$(DO.PLUGIN)

clean: csfontclean
csfontclean:
	-$(RM) $(CSFONT) $(OBJ.CSFONT)

ifdef DO_DEPEND
dep: $(OUTOS)csfont.dep
$(OUTOS)csfont.dep: $(SRC.CSFONT)
	$(DO.DEP)
else
-include $(OUTOS)csfont.dep
endif

endif # ifeq ($(MAKESECTION),targets)
