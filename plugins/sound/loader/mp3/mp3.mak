# Plug-in description
DESCRIPTION.sndmp3 = Crystal Space mp3 sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndmp3       Make the $(DESCRIPTION.sndmp3)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndmp3 mp3clean
all plugins drivers snddrivers: sndmp3

sndmp3:
	$(MAKE_TARGET) MAKE_DLL=yes
mp3clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/mp3 plugins/sound/loader/mp3/mpg123
ifneq ($(OS),WIN32)
  vpath %.cpp plugins/sound/loader/mp3/mpg123/xfermem
endif

ifeq ($(USE_PLUGINS),yes)
  SNDMP3 = $(OUTDLL)/sndmp3$(DLL)
  LIB.SNDMP3 = $(foreach d,$(DEP.SNDMP3),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDMP3)
else
  SNDMP3 = $(OUT)/$(LIB_PREFIX)sndmp3$(LIB)
  DEP.EXE += $(SNDMP3)
  SCF.STATIC += sndmp3
  TO_INSTALL.STATIC_LIBS += $(SNDMP3)
endif

MP3_USE_XFERMEM = yes
ifeq ($(OS),WIN32)
  MP3_USE_XFERMEM = no
endif
ifeq ($(DO_MSVCGEN),yes)
  MP3_USE_XFERMEM = no
endif

ifeq ($(MP3_USE_XFERMEM),yes)
  INC.SNDMP3 = $(wildcard \
    plugins/sound/loader/mp3/*.h \
    plugins/sound/loader/mp3/mpg123/*.h \
    plugins/sound/loader/mp3/mpg123/xfermem/*.h)
  SRC.SNDMP3 = $(wildcard \
    plugins/sound/loader/mp3/*.cpp \
    plugins/sound/loader/mp3/mpg123/*.cpp \
    plugins/sound/loader/mp3/mpg123/xfermem/*.cpp)
else
  INC.SNDMP3 = $(wildcard \
    plugins/sound/loader/mp3/*.h \
    plugins/sound/loader/mp3/mpg123/*.h)
  SRC.SNDMP3 = $(wildcard \
    plugins/sound/loader/mp3/*.cpp \
    plugins/sound/loader/mp3/mpg123/*.cpp)
endif

OBJ.SNDMP3 = $(addprefix $(OUT)/,$(notdir $(SRC.SNDMP3:.cpp=$O)))
DEP.SNDMP3 = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDMP3
DSP.SNDMP3.NAME = sndmp3
DSP.SNDMP3.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndmp3 mp3clean

sndmp3: $(OUTDIRS) $(SNDMP3)

$(SNDMP3): $(OBJ.SNDMP3) $(LIB.SNDMP3)
	$(DO.PLUGIN)

clean: mp3clean
mp3clean:
	$(RM) $(SNDMP3) $(OBJ.SNDMP3)

ifdef DO_DEPEND
dep: $(OUTOS)/sndmp3.dep
$(OUTOS)/sndmp3.dep: $(SRC.SNDMP3)
	$(DO.DEP)
else
-include $(OUTOS)/sndmp3.dep
endif

endif # ifeq ($(MAKESECTION),targets)
