# This submakefile dynamically computes the name for all plugin, library, and
# application submakefiles and includes them.

# SUBMAKEFILES is cached along with other makefile elements.  If it is still
# empty after including cache.mak, then assume that nothing was cached and
# load the submakefiles manually (below).
SUBMAKEFILES = $(EMPTY)

include mk/cache.mak

ifneq ($(TARGET_MAKEFILE),)
  include $(TARGET_MAKEFILE)
endif

PLUGINS += video/renderer video/canvas # Special defines.
ifeq ($(USE_PLUGINS),yes)
  PLUGINS += $(PLUGINS.DYNAMIC)
endif

ifeq ($(strip $(SUBMAKEFILES)),)
  SUBMAKEFILES = $(sort \
    docs/docs.mak mk/install.mak mk/msvcgen/msvcgen.mak \
    $(wildcard $(addsuffix /*.mak,$(addprefix plugins/,$(sort $(PLUGINS)))) \
    apps/*/*.mak \
    apps/tests/*/*.mak \
    apps/tutorial/*/*.mak \
    apps/examples/*/*.mak \
    apps/import/*/*.mak \
    apps/tools/*/*.mak \
    libs/*/*.mak))
  include $(SUBMAKEFILES)
endif
