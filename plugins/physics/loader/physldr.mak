# This is a subinclude file used to define the rules needed
# to build the physldr plug-in.

# Driver description
DESCRIPTION.physldr = Crystal Space Physics Loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make physldr      Make the $(DESCRIPTION.physldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: physldr physldrclean
all plugins: physldr

physldr:
	$(MAKE_TARGET) MAKE_DLL=yes
physldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/physics/loader

LIB.EXTERNAL.physldr = -lode

ifeq ($(USE_PLUGINS),yes)
  physldr = $(OUTDLL)/physldr$(DLL)
  LIB.physldr = $(foreach d,$(DEP.physldr),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(physldr)
else
  physldr = $(OUT)/$(LIB_PREFIX)physldr$(LIB)
  DEP.EXE += $(physldr)
  SCF.STATIC += physldr
  TO_INSTALL.STATIC_LIBS += $(physldr)
endif

INC.physldr = $(wildcard plugins/physics/loader/*.h)
SRC.physldr = $(wildcard plugins/physics/loader/*.cpp)
OBJ.physldr = $(addprefix $(OUT)/,$(notdir $(SRC.physldr:.cpp=$O)))
DEP.physldr = CSGEOM CSUTIL CSSYS

MSVC.DSP += physldr
DSP.physldr.NAME = physldr
DSP.physldr.TYPE = plugin
DSP.physldr.LIBS = ode

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: physldr physldrclean

physldr: $(OUTDIRS) $(physldr)

$(physldr): $(OBJ.physldr) $(LIB.physldr)
	$(DO.PLUGIN) $(LIB.EXTERNAL.physldr)

clean: physldrclean
physldrclean:
	$(RM) $(physldr) $(OBJ.physldr)

ifdef DO_DEPEND
dep: $(OUTOS)/physldr.dep
$(OUTOS)/physldr.dep: $(SRC.physldr)
	$(DO.DEP)
else
-include $(OUTOS)/physldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
