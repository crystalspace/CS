# This is a subinclude file used to define the rules needed
# to build the shadermgr plug-in.

# Driver description
DESCRIPTION.shadermgr = Crystal Space Shadermanager plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make shadermgr      Make the $(DESCRIPTION.shadermgr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: shadermgr shadermgrclean
all plugins: shadermgr

shadermgr:
	$(MAKE_TARGET) MAKE_DLL=yes
shadermgrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/render3d/shadermgr

ifeq ($(USE_PLUGINS),yes)
  SHADERMGR = $(OUTDLL)/shadermgr$(DLL)
  LIB.SHADERMGR = $(foreach d,$(DEP.SHADERMGR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SHADERMGR)
else
  SHADERMGR = $(OUT)/$(LIB_PREFIX)shadermgr$(LIB)
  DEP.EXE += $(SHADERMGR)
  SCF.STATIC += shadermgr
  TO_INSTALL.STATIC_LIBS += $(SHADERMGR)
endif

INC.SHADERMGR = $(wildcard plugins/video/render3d/shadermgr/*.h)
SRC.SHADERMGR = $(wildcard plugins/video/render3d/shadermgr/*.cpp)
OBJ.SHADERMGR = $(addprefix $(OUT)/,$(notdir $(SRC.SHADERMGR:.cpp=$O)))
DEP.SHADERMGR = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL

TO_INSTALL.CONFIG += $(CFG.SHADERMGR)

MSVC.DSP += SHADERMGR
DSP.SHADERMGR.NAME = shadermgr
DSP.SHADERMGR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: shadermgr shadermgrclean

shadermgr: $(OUTDIRS) $(SHADERMGR)

$(SHADERMGR): $(OBJ.SHADERMGR) $(LIB.SHADERMGR)
	$(DO.PLUGIN)

clean: shadermgrclean
shadermgrclean:
	$(RM) $(SHADERMGR) $(OBJ.SHADERMGR)

ifdef DO_DEPEND
dep: $(OUTOS)/shadermgr.dep
$(OUTOS)/shadermgr.dep: $(SRC.SHADERMGR)
	$(DO.DEP)
else
-include $(OUTOS)/shadermgr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
