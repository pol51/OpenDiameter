# $Id: Makefile,v 1.1 2004/06/24 15:57:43 vfajardo Exp $

TEX_PREFIX=main
LATEX2HTML_DIR=htmls

TARGET_DVI=$(TEX_PREFIX).dvi
TARGET_PDF=$(TEX_PREFIX).pdf
TARGET_HTML=$(LATEX2HTML_DIR)/index.html

TEX_FILES=$(TEX_PREFIX).tex libdiamparser.tex
TEX2PDF_FILES=architecture

all: $(TARGET_DVI) $(TARGET_HTML) $(TEX2PDF_FILES) 

$(TARGET_DVI) : $(TEX_FILES)
	latex $(TEX_PREFIX).tex
	latex $(TEX_PREFIX).tex
	dvips $(TEX_PREFIX) -o $(TEX_PREFIX).ps

$(TARGET_HTML): $(TEX_FILES) $(LATEX2HTML_DIR)
	latex2html -dir $(LATEX2HTML_DIR) $<
	tar cvzf $(LATEX2HTML_DIR).tgz ./$(LATEX2HTML_DIR)

$(LATEX2HTML_DIR):
	if [ ! -e $(LATEX2HTML_DIR) ]; then mkdir $(LATEX2HTML_DIR); fi 

$(TEX2PDF_FILES):
	latex $@.tex
	latex $@.tex
	dvips $@.dvi -o $@.ps 
	ps2pdf $@.ps

clean:
	-rm -rf *.dvi *~ *.log *.aux *.toc *.ps *.pdf *.tgz htmls
