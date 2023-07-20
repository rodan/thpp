
TARGET = thpp.pdf pm8006_report.pdf

all: $(TARGET)

%.pdf: %.tex
	@latexmk -pdf $<

clean:
	rm -f *.aux *.log *.out *.toc *.fdb_latexmk *.fls *.glo *.ist $(TARGET)
