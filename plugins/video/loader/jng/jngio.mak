# Plug-in description
DESCRIPTION.jngimg = Crystal Space jng image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make jngimg       Make the $(DESCRIPTION.jngimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: jngimg jngimgclean
all plugins: jngimg

jngimg:
	$(MAKE_TARGET) MAKE_DLL=yes
jngimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/jng

LIB.JNGIMG.LOCAL += $(JNG_LIBS) $(Z_LIBS)

ifeq ($(USE_PLUGINS),yes)
  JNGIMG = $(OUTDLL)csjngimg$(DLL)
  LIB.JNGIMG = $(foreach d,$(DEP.JNGIMG),$($d.LIB))
  LIB.JNGIMG.SPECIAL += $(LIB.JNGIMG.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(JNGIMG)
else
  JNGIMG = $(OUT)$(LIB_PREFIX)csjngimg$(LIB)
  DEP.EXE += $(JNGIMG)
  LIBS.EXE += $(LIB.JNGIMG.LOCAL)
  SCF.STATIC += csjngimg
  TO_INSTALL.STATIC_LIBS += $(JNGIMG)
endif

INC.JNGIMG = $(wildcard plugins/video/loader/jng/*.h)
SRC.JNGIMG = $(wildcard plugins/video/loader/jng/*.cpp)

OBJ.JNGIMG = $(addprefix $(OUT),$(notdir $(SRC.JNGIMG:.cpp=$O)))
DEP.JNGIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += JNGIMG
DSP.JNGIMG.NAME = csjngimg
DSP.JNGIMG.TYPE = plugin
DSP.JNGIMG.LIBS = libmng zlib libjpeg

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: jngimg jngimgclean

jngimg: $(OUTDIRS) $(JNGIMG)

$(JNGIMG): $(OBJ.JNGIMG) $(LIB.JNGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.JNGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: jngimgclean
jngimgclean:
	$(RM) $(JNGIMG) $(OBJ.JNGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)jngimg.dep
$(OUTOS)jngimg.dep: $(SRC.JNGIMG)
	$(DO.DEP)
else
-include $(OUTOS)jngimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

