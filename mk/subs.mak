# This submakefile dynamically compute the name for all
# plugins, libs, and apps submakefiles and includes them.

ifneq ($(TARGET_MAKEFILE),)
  include $(TARGET_MAKEFILE)
endif

ifeq ($(LIBRARY_SUBMAKEFILES),)
  LIBRARY_SUBMAKEFILES=$(wildcard libs/*/*.mak)
endif
ifneq ($(LIBRARY_SUBMAKEFILES),)
  include $(LIBRARY_SUBMAKEFILES)
endif

ifeq ($(PLUGINS_SUBMAKEFILES),)
  ifeq ($(USE_PLUGINS),yes)
    PLUGINS += $(PLUGINS.DYNAMIC)
  endif
  PLUGINS_SUBMAKEFILES=\
    $(wildcard $(addsuffix /*.mak,$(addprefix plugins/,$(sort $(PLUGINS)))))
endif
ifneq ($(PLUGINS_SUBMAKEFILES),)
  PLUGINS.AUGMENTATION += video/renderer video/canvas # Special defines.
  include $(wildcard $(addsuffix /*.mak,$(addprefix plugins/,\
    $(sort $(PLUGINS.AUGMENTATION))))) $(PLUGINS_SUBMAKEFILES)
endif

ifeq ($(APPLICATION_SUBMAKEFILES),)
  APPLICATION_SUBMAKEFILES=$(wildcard apps/*/*.mak)
endif
ifneq ($(APPLICATION_SUBMAKEFILES),)
  include $(APPLICATION_SUBMAKEFILES)
endif

ifeq ($(SUBAPPLICATION_SUBMAKEFILES),)
  SUBAPPLICATION_SUBMAKEFILES=$(wildcard apps/*/*/*.mak)
endif
ifneq ($(SUBAPPLICATION_SUBMAKEFILES),)
  include $(SUBAPPLICATION_SUBMAKEFILES)
endif

ifeq ($(COMPOSITE_SUBMAKEFILES),)
  COMPOSITE_SUBMAKEFILES=docs/docs.mak mk/install.mak mk/msvcgen/msvcgen.mak \
    mk/static.mak
endif
ifneq ($(COMPOSITE_SUBMAKEFILES),)
  include $(COMPOSITE_SUBMAKEFILES)
endif
