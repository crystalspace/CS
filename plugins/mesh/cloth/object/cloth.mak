DESCRIPTION.cloth = Cloth mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cloth        Make the $(DESCRIPTION.cloth)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cloth clothclean
plugins meshes all: cloth

clothclean:
	$(MAKE_CLEAN)
cloth:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/cloth/object

ifeq ($(USE_PLUGINS),yes)
  CLOTH = $(OUTDLL)/cloth$(DLL)
  LIB.CLOTH = $(foreach d,$(DEP.CLOTH),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CLOTH)
else
  CLOTH = $(OUT)/$(LIB_PREFIX)cloth$(LIB)
  DEP.EXE += $(CLOTH)
  SCF.STATIC += cloth
  TO_INSTALL.STATIC_LIBS += $(CLOTH)
endif

INC.CLOTH = $(wildcard plugins/mesh/cloth/object/*.h)
SRC.CLOTH = $(wildcard plugins/mesh/cloth/object/*.cpp)
OBJ.CLOTH = $(addprefix $(OUT)/,$(notdir $(SRC.CLOTH:.cpp=$O)))
DEP.CLOTH = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += CLOTH
DSP.CLOTH.NAME = cloth
DSP.CLOTH.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cloth clothclean
cloth: $(OUTDIRS) $(CLOTH)

$(CLOTH): $(OBJ.CLOTH) $(LIB.CLOTH)
	$(DO.PLUGIN)

clean: clothclean
clothclean:
	-$(RM) $(CLOTH) $(OBJ.CLOTH)

ifdef DO_DEPEND
dep: $(OUTOS)/cloth.dep
$(OUTOS)/cloth.dep: $(SRC.CLOTH)
	$(DO.DEP)
else
-include $(OUTOS)/cloth.dep
endif

endif # ifeq ($(MAKESECTION),targets)
