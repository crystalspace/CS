# Target descriptions
DESCRIPTION.doc = developer class reference via Doc++
DESCRIPTION.api = public API reference via Doc++
DESCRIPTION.html = documentation as HTML
DESCRIPTION.pdf = documentation as PDF
DESCRIPTION.text = documentation as ASCII text
DESCRIPTION.doczips = ZIP packages of all documentation

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
PSEUDOHELP += $(NEWLINE)echo $"  make doc          Make the $(DESCRIPTION.doc)$" \
              $(NEWLINE)echo $"  make api          Make the $(DESCRIPTION.api)$" \
              $(NEWLINE)echo $"  make html         Make the $(DESCRIPTION.html)$" \
              $(NEWLINE)echo $"  make pdf          Make the $(DESCRIPTION.pdf)$" \
              $(NEWLINE)echo $"  make text         Make the $(DESCRIPTION.text)$" \
              $(NEWLINE)echo $"  make doczips      Make the $(DESCRIPTION.doczips)$" \
              $(NEWLINE)echo $"  make cleandoc     Clean all generated documentation$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: all doc api html pdf text doczips cleandoc

doc api html pdf text doczips:
	$(MAKE_TARGET)
cleandoc:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DOCDIRS = docs newdoc
CSAPI_TOPICS = interfaces $(filter-out %.h %.am CVS,$(notdir $(wildcard include/*)))
CSAPI_FILES = $(wildcard include/*.h include/*/*.h include/*/*/*.h)
CSDOC_FILES = $(wildcard *.h */*.h */*/*.h */*/*/*.h */*/*/*/*.h)
TEX_FILES = $(wildcard $(foreach dir,$(DOCDIRS),$(dir)/*.tex))
TTX_FILES = $(wildcard include/$(*F)/*.h)
TOPIC_FILES = cs-tut.ZZZ cs-inst.ZZZ cs-thry.ZZZ cs-def.ZZZ cs-debu.ZZZ \
  $(addsuffix .ZZZ,$(CSAPI_TOPICS))
PDF_FILES = $(TOPIC_FILES:.ZZZ=.pdf)
TEXT_FILES = $(TOPIC_FILES:.ZZZ=.txt)
CSAPI_DIR = csapi
CSDOC_DIR = csdoc
GENDOC_DIR = gendoc
HTML_DIR = $(GENDOC_DIR)/html
PDF_DIR = $(GENDOC_DIR)/pdf
TEXT_DIR = $(GENDOC_DIR)/text
PICS_DIR = $(GENDOC_DIR)/pics

DO.PDFLATEX=pdflatex -interaction=nonstopmode

vpath $(DOCDIRS)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: doc api html pdf text doczips cleandoc pics classpdf \
  cs-help-html.zip cs-help-pdf.zip cs-help-txt.zip

clean: cleandoc

%.pdf: %.tex
	$(DO.PDFLATEX) $<

%.ttx: $(TTX_FILES)
	doc++ -t -o $(*F).ttx $^

%.pdf: %.ttx
	$(DO.PDFLATEX) $<

%.txt: %.pdf
	pdftotext $<

$(CSAPI_DIR) $(CSDOC_DIR) $(GENDOC_DIR) $(PICS_DIR):
	$(MKDIR)

$(CSAPI_DIR)/index.html: $(CSAPI_DIR) newdoc/html/docxxbanner.html
	doc++ -F -B newdoc/html/docxxbanner.html -j -H -d $(CSAPI_DIR) -f $(CSAPI_FILES)

$(CSDOC_DIR)/index.html: $(CSDOC_DIR) newdoc/html/docxxbanner.html
	doc++ -F -T newdoc/html/docxxbanner.html -j -H -d $(CSDOC_DIR) -f $(CSDOC_FILES)
	$(RM) $(CSDOC_DIR)/HIERjava.html

$(HTML_DIR)/index.html:
	latex2html newdoc/html
	python bin/mshelp.py crystal

pics: $(PICS_DIR)
	cp newdoc/pics/* $(PICS_DIR)/

api: csapi/index.html pics

doc: csdoc/index.html pics

html: doc pics html/index.html

pdf: $(PDF_FILES)

text: $(TEXT_FILES)

cs-help-html.zip: html
	(cd $(HTML_DIR); zip -9 -rp cs-help-html.zip csdoc html pics crystal.hh*)
	mv $(HTML_DIR)/cs-help-html.zip ./

cs-help-pdf.zip: pdf
	(cd $(PDF_DIR); zip -9 cs-help-pdf.zip $(PDF_FILES))
	mv $(PDF_DIR)/cs-help-pdf.zip ./

cs-help-txt.zip: text
	(cd $(TEXT_DIR); zip -9 cs-help-text.zip $(TEXT_FILES))
	mv $(TEXT_DIR)/cs-help-text.zip ./

doczips: cs-help-html.zip cs-help-pdf.zip cs-help-book.zip cs-help-txt.zip

cleandoc:
	$(RMDIR) $(CSDOC_DIR) $(CSAPI_DIR) $(GENDOC_DIR)
	$(RM) *.aux *.log *.pdf *.idx *.toc *.ttx
	$(RM) crystal.hhc crystal.hhk crystal.hhp

endif # ifeq ($(MAKESECTION),targets)
