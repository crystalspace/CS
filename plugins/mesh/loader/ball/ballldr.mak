DESCRIPTION.ballldr = Ball mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ballldr      Make the $(DESCRIPTION.ballldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ballldr ballldrclean
plugins meshes all: ballldr

ballldrclean:
	$(MAKE_CLEAN)
ballldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/ball

ifeq ($(USE_PLUGINS),yes)
  BALLLDR = $(OUTDLL)ballldr$(DLL)
  LIB.BALLLDR = $(foreach d,$(DEP.BALLLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BALLLDR)
else
  BALLLDR = $(OUT)$(LIB_PREFIX)ballldr$(LIB)
  DEP.EXE += $(BALLLDR)
  SCF.STATIC += ballldr
  TO_INSTALL.STATIC_LIBS += $(BALLLDR)
endif

INC.BALLLDR = $(wildcard plugins/mesh/loader/ball/*.h)
SRC.BALLLDR = $(wildcard plugins/mesh/loader/ball/*.cpp)
OBJ.BALLLDR = $(addprefix $(OUT),$(notdir $(SRC.BALLLDR:.cpp=$O)))
DEP.BALLLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += BALLLDR
DSP.BALLLDR.NAME = ballldr
DSP.BALLLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ballldr ballldrclean
ballldr: $(OUTDIRS) $(BALLLDR)

$(BALLLDR): $(OBJ.BALLLDR) $(LIB.BALLLDR)
	$(DO.PLUGIN)

clean: ballldrclean
ballldrclean:
	-$(RM) $(BALLLDR) $(OBJ.BALLLDR)

ifdef DO_DEPEND
dep: $(OUTOS)ballldr.dep
$(OUTOS)ballldr.dep: $(SRC.BALLLDR)
	$(DO.DEP)
else
-include $(OUTOS)ballldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
