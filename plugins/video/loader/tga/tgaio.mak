# Plug-in description
DESCRIPTION.tgaimg = Crystal Space tga image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make tgaimg       Make the $(DESCRIPTION.tgaimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tgaimg tgaimgclean
all plugins: tgaimg

tgaimg:
	$(MAKE_TARGET) MAKE_DLL=yes
tgaimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/tga

ifeq ($(USE_PLUGINS),yes)
  TGAIMG = $(OUTDLL)cstgaimg$(DLL)
  LIB.TGAIMG = $(foreach d,$(DEP.TGAIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TGAIMG)
else
  TGAIMG = $(OUT)$(LIB_PREFIX)cstgaimg$(LIB)
  DEP.EXE += $(TGAIMG)
  SCF.STATIC += cstgaimg
  TO_INSTALL.STATIC_LIBS += $(TGAIMG)
endif

INC.TGAIMG = $(wildcard plugins/video/loader/tga/*.h)
SRC.TGAIMG = $(wildcard plugins/video/loader/tga/*.cpp)

OBJ.TGAIMG = $(addprefix $(OUT),$(notdir $(SRC.TGAIMG:.cpp=$O)))
DEP.TGAIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += TGAIMG
DSP.TGAIMG.NAME = cstgaimg
DSP.TGAIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tgaimg tgaimgclean

tgaimg: $(OUTDIRS) $(TGAIMG)

$(TGAIMG): $(OBJ.TGAIMG) $(LIB.TGAIMG)
	$(DO.PLUGIN)

clean: tgaimgclean
tgaimgclean:
	$(RM) $(TGAIMG) $(OBJ.TGAIMG)

ifdef DO_DEPEND
dep: $(OUTOS)tgaimg.dep
$(OUTOS)tgaimg.dep: $(SRC.TGAIMG)
	$(DO.DEP)
else
-include $(OUTOS)tgaimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

