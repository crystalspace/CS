#==============================================================================
#
#    Makefile caching facility for improving build speed
#    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
#------------------------------------------------------------------------------
# cache.mak
#
#	A makefile which implements makefile caching in order to speed the
#	build process on machines with slow disk subsystems.  In general, the
#	build process is slowed considerably by GNU make having to search out
#	and read subcomponent makefiles repeatedly during a build session.  At
#	the time of this writing, the makefile system invokes subs.mak a total
#	of five times in order to search out and read nearly every single
#	makefile in the project.  On machines with slow disk subsystems, this
#	repeated searching and reading can be agonizingly time-consuming.  In
#	order to alleviate this particular problem, makefile information is
#	cached so that it can be retrieved quickly when later needed.
#
# IMPORTS
#	Makefile caching is controlled by the USE_MAKEFILE_CACHE variable.  The
#	default value of this variable is set to `yes' in user.mak.  This
#	variable can also be set from the command line at makefile
#	configuration time and becomes a persistent setting in config.mak.
#	Example: make linux USE_MAKEFILE_CACHE=yes
#
#	In order to prevent the makefile cache from becoming outdated, it can
#	be refreshed automatically anytime any of its source makefiles is
#	changed.  However, monitoring the source makefiles for changes slows
#	down the build process slightly, so this option can be disabled, if
#	desired, by setting the MONITOR_MAKEFILE_CACHE variable to `no'.  By
#	default, for safety and accuracy, this variable is set to `yes' in
#	user.mak.  It can also be set from the command line at makefile
#	configuration time and becomes a persistent setting in config.mak.
#	Example: make linux MONITOR_MAKEFILE_CACHE=yes
#
#	Note that the makefile cache is also refreshed any time user.mak or the
#	configured platform-specific makefile is changed.  These two files are
#	monitored unconditionally, regardless of the setting of
#	MONITOR_MAKEFILE_CACHE.
#
# EXPORTS
#	The makefile target `recache' can be used to manually refresh the
#	makefile cache at any time.  This can be useful after editing a
#	makefile if automatic refresh has been disabled by setting
#	MONITOR_MAKEFILE_CACHE to `no'.
#
#	USE_MAKEFILE_CACHE and MONITOR_MAKEFILE_CACHE are written to config.mak
#	at makefile configuration time in order to make the settings
#	persistent.
#
#	The actual cache is stored in the file cache.mak in the root directory
#	of the project.  It is removed when the makefile targets `distclean'
#	and `unknown' are invoked.
#------------------------------------------------------------------------------

MAKEFILE_CACHE = cache.mak

# Anytime this submakefile is included by subs.mak, also try including the
# cache file, however take note of the following discussion.  If an included
# makefile does not exist, GNU make will try to rebuild it automatically.  This
# is actually desirable, since cache.mak should be rebuilt any time one of the
# source makefiles changes.  However, before the makefile system is configured,
# cache.mak does not exist, and we do _not_ want GNU make building it the first
# time through since we do not have sufficient information for a correct build.
# Therefore, we only invoke `include' if the file already exists.  This means
# that GNU make will only check the dependencies of cache.mak after it has been
# successfully built, but will not try to automatically build it before we are
# ready to have it built.
ifeq ($(USE_MAKEFILE_CACHE),yes)
ifneq ($(wildcard $(MAKEFILE_CACHE)),)
  include $(MAKEFILE_CACHE)
endif
endif


#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

ifeq ($(USE_MAKEFILE_CACHE),yes)
PSEUDOHELP += \
  $(NEWLINE)echo $"  make recache      Refresh the makefile cache$"
endif

SYSMODIFIERS += \
  $(NEWLINE)echo $"  USE_MAKEFILE_CACHE=$(USE_MAKEFILE_CACHE)$" \
  $(NEWLINE)echo $"  MONITOR_MAKEFILE_CACHE=$(MONITOR_MAKEFILE_CACHE)$"

CACHE_DEPS = mk/user.mak $(TARGET_MAKEFILE)
ifeq ($(MONITOR_MAKEFILE_CACHE),yes)
  CACHE_DEPS += $(SUBMAKEFILES)
endif

# Do not remove blank lines; they force needed newlines in the output file.
define CACHE.BODY.NAME
  echo SUBMAKEFILES += $r>>$(MAKEFILE_CACHE)

endef

define CACHE.BODY.CONTENT
  cat $r>>$(MAKEFILE_CACHE)

endef

define CACHE.BODY
  echo SUBMAKEFILES = >$(MAKEFILE_CACHE)
  $(foreach r,$(SUBMAKEFILES),$(CACHE.BODY.NAME))
  echo >>$(MAKEFILE_CACHE)
  $(foreach r,$(SUBMAKEFILES),$(CACHE.BODY.CONTENT))
endef

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

ifeq ($(USE_MAKEFILE_CACHE),yes)

.PHONY: configure
configure: $(MAKEFILE_CACHE)
$(MAKEFILE_CACHE): $(CACHE_DEPS)
	@$(CACHE.BODY)

.PHONY: recache
recache:
	$(RM) $(MAKEFILE_CACHE)
	@$(MAKE) $(RECMAKEFLAGS) $(MAKEFILE_CACHE)
	@echo $"Makefile cache refreshed.$"

endif # ifeq ($(USE_MAKEFILE_CACHE),yes)

.PHONY: cleancache
unknown: cleancache
distclean: cleancache
cleancache:
	$(RM) $(MAKEFILE_CACHE)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSMODIFIERSHELP += \
  $(NEWLINE)echo $"  USE_MAKEFILE_CACHE=yes$|no$" \
  $(NEWLINE)echo $"      Cache makefile information for speedier builds.$" \
  $(NEWLINE)echo $"  MONITOR_MAKEFILE_CACHE=yes$|no$" \
  $(NEWLINE)echo $"      Automatically refresh makefile cache when outdated.$" \
  $(NEWLINE)echo $"      Caution: Monitoring cache slows build process slightly.$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#

ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)

SYSCONFIG += \
  $(NEWLINE)@echo USE_MAKEFILE_CACHE = $(USE_MAKEFILE_CACHE)>>config.tmp \
  $(NEWLINE)@echo MONITOR_MAKEFILE_CACHE = $(MONITOR_MAKEFILE_CACHE)>>config.tmp

endif # ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)
