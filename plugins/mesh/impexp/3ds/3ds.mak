DESCRIPTION.ie3ds = Model Import/Export 3ds plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ie3ds       Make the $(DESCRIPTION.ie3ds)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ie3ds ie3dsclean
plugins meshes all: ie3ds

ie3dsclean:
	$(MAKE_CLEAN)
ie3ds:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/3ds

ifeq ($(USE_PLUGINS),yes)
  IEPLEX = $(OUTDLL)ieplex$(DLL)
  LIB.IE3DS = $(foreach d,$(DEP.IE3DS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IE3DS)
else
  IEPLEX = $(OUT)$(LIB_PREFIX)ie3ds$(LIB)
  DEP.EXE += $(IE3DS)
  SCF.STATIC += ie3ds
  TO_INSTALL.STATIC_LIBS += $(IE3DS)
endif

INC.IE3DS = $(wildcard plugins/mesh/impexp/3ds/*.h)
SRC.IE3DS = $(wildcard plugins/mesh/impexp/3ds/*.cpp)
OBJ.IE3DS = $(addprefix $(OUT),$(notdir $(SRC.IE3DS:.cpp=$O)))
DEP.IE3DS = CSGEOM CSUTIL CSSYS CSTOOL CSUTIL

MSVC.DSP += IE3DS
DSP.IE3DS.NAME = ie3ds
DSP.IE3DS.TYPE = plugin
DSP.IE3DS.LIBS = lib3ds-101d

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ie3ds ie3dsclean
ie3ds: $(OUTDIRS) $(IE3DS)

$(IE3DS): $(OBJ.IE3DS) $(LIB.IE3DS)
	$(DO.PLUGIN)

clean: ie3dsclean
ie3dsclean:
	-$(RM) $(IE3DS) $(OBJ.IE3DS)

ifdef DO_DEPEND
dep: $(OUTOS)ie3ds.dep
$(OUTOS)ie3ds.dep: $(SRC.IE3DS)
	$(DO.DEP)
else
-include $(OUTOS)ie3ds.dep
endif

endif # ifeq ($(MAKESECTION),targets)
