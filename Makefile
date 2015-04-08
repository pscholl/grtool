CPPFLAGS=`pkg-config --cflags grt` -g -std=c++11 -O3
LDFLAGS=-lstdc++ -lcrfsuite `pkg-config --libs grt`
ALL=grt train predict info score preprocess extract postprocess

all: $(ALL) *.h
train: train.o grt_crf.o
predict: predict.o grt_crf.o


# Installation targets
PREFIX=/usr
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/
DESTDIR=
INSTALL_PROGRAM=install

install: $(ALL) #install-doc
	$(INSTALL_PROGRAM) -D grt "$(DESTDIR)$(BINDIR)/grt"
	$(INSTALL_PROGRAM) -D train "$(DESTDIR)$(BINDIR)/grt-train"
	$(INSTALL_PROGRAM) -D predict "$(DESTDIR)$(BINDIR)/grt-predict"
	$(INSTALL_PROGRAM) -D preprocess "$(DESTDIR)$(BINDIR)/grt-preprocess"
	$(INSTALL_PROGRAM) -D postprocess "$(DESTDIR)$(BINDIR)/grt-postprocess"
	$(INSTALL_PROGRAM) -D extract "$(DESTDIR)$(BINDIR)/grt-extract"
	$(INSTALL_PROGRAM) -D score "$(DESTDIR)$(BINDIR)/grt-score"
	$(INSTALL_PROGRAM) -D info "$(DESTDIR)$(BINDIR)/grt-info"
	$(INSTALL_PROGRAM) -D plot "$(DESTDIR)$(BINDIR)/grt-plot"
	$(INSTALL_PROGRAM) -D segment "$(DESTDIR)$(BINDIR)/grt-segment"

install-doc: doc/score.1 doc/train.1 doc/predict.1 doc/info.1 doc/grt.1 doc/preprocess.1 doc/extract.1 doc/postprocess.1
	$(INSTALL_PROGRAM) -D doc/grt.1 "$(DESTDIR)$(MANDIR)/man1/grt.1"
	$(INSTALL_PROGRAM) -D doc/score.1 "$(DESTDIR)$(MANDIR)/man1/grt-score.1"
	$(INSTALL_PROGRAM) -D doc/info.1 "$(DESTDIR)$(MANDIR)/man1/grt-info.1"
	$(INSTALL_PROGRAM) -D doc/train.1 "$(DESTDIR)$(MANDIR)/man1/grt-train.1"
	$(INSTALL_PROGRAM) -D doc/preprocess.1 "$(DESTDIR)$(MANDIR)/man1/grt-preprocess.1"
	$(INSTALL_PROGRAM) -D doc/postprocess.1 "$(DESTDIR)$(MANDIR)/man1/grt-postprocess.1"
	$(INSTALL_PROGRAM) -D doc/extract.1 "$(DESTDIR)$(MANDIR)/man1/grt-extract.1"
	$(INSTALL_PROGRAM) -D doc/predict.1 "$(DESTDIR)$(MANDIR)/man1/grt-predict.1"

clean:
	rm -f $(ALL) *.o

test: doc/*.md regression/*.md install
	python doctest.py -p "$(DESTDIR)/$(BINDIR)" doc/*.md
	python doctest.py -p "$(DESTDIR)/$(BINDIR)" regression/*.md

doc/%.1: doc/%.md
	pandoc -s -t man $< -o $@

