# Target description
DESCRIPTION.csconfig = cs-config compile help script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make csconfig     Build the $(DESCRIPTION.csconfig)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconfig csconfigclean

csconfig:
	$(MAKE_TARGET)
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean $(CSCONFIG.EXE)

all: $(CSCONFIG.EXE)
csconfig: $(OUTDIRS) $(CSCONFIG.EXE)
clean: csconfigclean

ifeq ($(LINK.PLUGIN),)
  LINK.PLUGIN = $(LINK)
endif

# create out/csconfig.tmp for the makevars that need to be transferred
$(CSCONFIG.EXE):
	@echo Generating cs-config script...
	@echo $"# Crystal Space config make variables for $(DESCRIPTION.$(TARGET)).$" > out/csconfig.tmp
	@echo $"# Note: automatically generated. $" >> out/csconfig.tmp
	@echo $"EXE=$(EXE)$" >> out/csconfig.tmp
	@echo $"DLL=$(DLL)$" >> out/csconfig.tmp
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL)$" >> out/csconfig.tmp
	@echo $"LFLAGS.EXE=$(LFLAGS.EXE")$" >> out/csconfig.tmp
	@echo $"DO.SHARED.PLUGIN.PREAMBLE=$(DO.SHARED.PLUGIN.PREAMBLE)$" >> out/csconfig.tmp
	@echo $"DO.SHARED.PLUGIN.POSTAMBLE=$(DO.SHARED.PLUGIN.POSTAMBLE)$" >> out/csconfig.tmp
	@echo $"LIBS.EXE.PLATFORM=$(LIBS.EXE.PLATFORM)$" >> out/csconfig.tmp
	@echo $"LINK.PLUGIN=$(LINK.PLUGIN)$" >> out/csconfig.tmp
	@echo $"PLUGIN.POSTFLAGS=$(PLUGIN.POSTFLAGS)$" >> out/csconfig.tmp

	scripts/cs-config/genscript.sh "$(INSTALL_DIR)" "$(CXXFLAGS)" \
	"$(CFLAGS)" "$(LIBS.EXE)" scripts/cs-config
	$(RM) out/csconfig.tmp

csconfigclean:
	-$(RM) $(CSCONFIG.EXE)

endif

