DESCRIPTION.ieplex = Model Import/Export multiplexer plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ieplex       Make the $(DESCRIPTION.ieplex)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ieplex ieplexclean
plugins meshes all: ieplex

ieplexclean:
	$(MAKE_CLEAN)
ieplex:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/multiplex

ifeq ($(USE_PLUGINS),yes)
  IEPLEX = $(OUTDLL)ieplex$(DLL)
  LIB.IEPLEX = $(foreach d,$(DEP.IEPLEX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IEPLEX)
else
  IEPLEX = $(OUT)$(LIB_PREFIX)ieplex$(LIB)
  DEP.EXE += $(IEPLEX)
  SCF.STATIC += ieplex
  TO_INSTALL.STATIC_LIBS += $(IEPLEX)
endif

INC.IEPLEX = $(wildcard plugins/mesh/impexp/multiplex/*.h)
SRC.IEPLEX = $(wildcard plugins/mesh/impexp/multiplex/*.cpp)
OBJ.IEPLEX = $(addprefix $(OUT),$(notdir $(SRC.IEPLEX:.cpp=$O)))
DEP.IEPLEX = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += IEPLEX
DSP.IEPLEX.NAME = ieplex
DSP.IEPLEX.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ieplex ieplexclean
ieplex: $(OUTDIRS) $(IEPLEX)

$(IEPLEX): $(OBJ.IEPLEX) $(LIB.IEPLEX)
	$(DO.PLUGIN)

clean: ieplexclean
ieplexclean:
	-$(RM) $(IEPLEX) $(OBJ.IEPLEX)

ifdef DO_DEPEND
dep: $(OUTOS)ieplex.dep
$(OUTOS)ieplex.dep: $(SRC.IEPLEX)
	$(DO.DEP)
else
-include $(OUTOS)ieplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)
