# This submakefile dynamically computes the name for all plugin, library, and
# application submakefiles and includes them.

include $(SRCDIR)/$(TARGET_MAKEFILE)

PLUGINS += $(PLUGINS.SEED)
PLUGINS += video/renderer video/canvas # Special defines; not optional.
ifeq ($(USE_PLUGINS),yes)
  PLUGINS += $(PLUGINS.DYNAMIC)
endif

# SUBMAKEFILES is cached along with other makefile elements.  If it is still
# empty after including cache.mak, then assume that nothing was cached and
# load the submakefiles manually.
SUBMAKEFILES = $(EMPTY)

include $(SRCDIR)/mk/cache.mak

ifeq ($(strip $(SUBMAKEFILES)),)
  SUBMAKEFILES = $(wildcard $(sort $(addprefix $(SRCDIR)/, \
    $(addprefix plugins/,$(addsuffix /*.mak,$(PLUGINS))) \
    apps/*/*.mak \
    apps/tests/*/*.mak \
    apps/tutorial/*/*.mak \
    apps/examples/*/*.mak \
    apps/import/*/*.mak \
    apps/tools/*/*.mak \
    docs/docs.mak \
    libs/*/*.mak \
    mk/install.mak \
    mk/msvcgen/msvcgen.mak \
    mk/static.mak \
    scripts/cs-config/csconf.mak)))
  include $(SUBMAKEFILES)
endif
