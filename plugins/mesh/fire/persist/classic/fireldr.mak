DESCRIPTION.fireldr = Fire mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fireldr      Make the $(DESCRIPTION.fireldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fireldr fireldrclean
plugins meshes all: fireldr

fireldrclean:
	$(MAKE_CLEAN)
fireldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/fire/persist/classic

ifeq ($(USE_PLUGINS),yes)
  FIRELDR = $(OUTDLL)fireldr$(DLL)
  LIB.FIRELDR = $(foreach d,$(DEP.FIRELDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FIRELDR)
else
  FIRELDR = $(OUT)$(LIB_PREFIX)fireldr$(LIB)
  DEP.EXE += $(FIRELDR)
  SCF.STATIC += fireldr
  TO_INSTALL.STATIC_LIBS += $(FIRELDR)
endif

INC.FIRELDR = $(wildcard plugins/mesh/fire/persist/classic/*.h)
SRC.FIRELDR = $(wildcard plugins/mesh/fire/persist/classic/*.cpp)
OBJ.FIRELDR = $(addprefix $(OUT),$(notdir $(SRC.FIRELDR:.cpp=$O)))
DEP.FIRELDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += FIRELDR
DSP.FIRELDR.NAME = fireldr
DSP.FIRELDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fireldr fireldrclean
fireldr: $(OUTDIRS) $(FIRELDR)

$(FIRELDR): $(OBJ.FIRELDR) $(LIB.FIRELDR)
	$(DO.PLUGIN)

clean: fireldrclean
fireldrclean:
	-$(RM) $(FIRELDR) $(OBJ.FIRELDR)

ifdef DO_DEPEND
dep: $(OUTOS)fireldr.dep
$(OUTOS)fireldr.dep: $(SRC.FIRELDR)
	$(DO.DEP)
else
-include $(OUTOS)fireldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
