DESCRIPTION.hazeldr = Haze mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make hazeldr      Make the $(DESCRIPTION.hazeldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: hazeldr hazeldrclean
plugins meshes all: hazeldr

hazeldrclean:
	$(MAKE_CLEAN)
hazeldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/haze/persist/classic

ifeq ($(USE_PLUGINS),yes)
  HAZELDR = $(OUTDLL)hazeldr$(DLL)
  LIB.HAZELDR = $(foreach d,$(DEP.HAZELDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(HAZELDR)
else
  HAZELDR = $(OUT)$(LIB_PREFIX)hazeldr$(LIB)
  DEP.EXE += $(HAZELDR)
  SCF.STATIC += hazeldr
  TO_INSTALL.STATIC_LIBS += $(HAZELDR)
endif

INC.HAZELDR = $(wildcard plugins/mesh/haze/persist/classic/*.h)
SRC.HAZELDR = $(wildcard plugins/mesh/haze/persist/classic/*.cpp)
OBJ.HAZELDR = $(addprefix $(OUT),$(notdir $(SRC.HAZELDR:.cpp=$O)))
DEP.HAZELDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += HAZELDR
DSP.HAZELDR.NAME = hazeldr
DSP.HAZELDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: hazeldr hazeldrclean
hazeldr: $(OUTDIRS) $(HAZELDR)

$(HAZELDR): $(OBJ.HAZELDR) $(LIB.HAZELDR)
	$(DO.PLUGIN)

clean: hazeldrclean
hazeldrclean:
	-$(RM) $(HAZELDR) $(OBJ.HAZELDR)

ifdef DO_DEPEND
dep: $(OUTOS)hazeldr.dep
$(OUTOS)hazeldr.dep: $(SRC.HAZELDR)
	$(DO.DEP)
else
-include $(OUTOS)hazeldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
