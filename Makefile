CPPFLAGS=`pkg-config --cflags grt` -g -std=gnu++11 -fpermissive -O3
LDLIBS=-lstdc++ `pkg-config --libs grt`
ALL=grt train predict info score preprocess extract

all: $(ALL) *.h
#train: train.o grt_crf.o
#predict: predict.o grt_crf.o

# Installation targets
PREFIX=/usr
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/
DESTDIR=
INSTALL_PROGRAM=install

install: $(ALL) # install-doc
	$(INSTALL_PROGRAM) -D -T grt "$(DESTDIR)$(BINDIR)/grt"
	$(INSTALL_PROGRAM) -D -T train "$(DESTDIR)$(BINDIR)/grt-train"
	$(INSTALL_PROGRAM) -D -T train-skl "$(DESTDIR)$(BINDIR)/grt-train-skl"
	$(INSTALL_PROGRAM) -D -T predict "$(DESTDIR)$(BINDIR)/grt-predict"
	$(INSTALL_PROGRAM) -D -T predict-skl "$(DESTDIR)$(BINDIR)/grt-predict-skl"
	$(INSTALL_PROGRAM) -D -T preprocess "$(DESTDIR)$(BINDIR)/grt-preprocess"
	$(INSTALL_PROGRAM) -D -T postprocess "$(DESTDIR)$(BINDIR)/grt-postprocess"
	$(INSTALL_PROGRAM) -D -T extract "$(DESTDIR)$(BINDIR)/grt-extract"
	$(INSTALL_PROGRAM) -D -T score "$(DESTDIR)$(BINDIR)/grt-score"
	$(INSTALL_PROGRAM) -D -T score-tl "$(DESTDIR)$(BINDIR)/grt-score-tl"
	$(INSTALL_PROGRAM) -D -T info "$(DESTDIR)$(BINDIR)/grt-info"
	$(INSTALL_PROGRAM) -D -T plot "$(DESTDIR)$(BINDIR)/grt-plot"
	$(INSTALL_PROGRAM) -D -T segment "$(DESTDIR)$(BINDIR)/grt-segment"
	$(INSTALL_PROGRAM) -D -T pack "$(DESTDIR)$(BINDIR)/grt-pack"
	$(INSTALL_PROGRAM) -D -T unpack "$(DESTDIR)$(BINDIR)/grt-unpack"
	$(INSTALL_PROGRAM) -D -T montage "$(DESTDIR)$(BINDIR)/grt-montage"
ifneq (,$(wildcard train-dlib))
	$(INSTALL_PROGRAM) -D -T train-dlib "$(DESTDIR)$(BINDIR)/grt-train-dlib"
endif
ifneq (,$(wildcard predict-dlib))
	$(INSTALL_PROGRAM) -D -T predict-dlib "$(DESTDIR)$(BINDIR)/grt-predict-dlib"
endif

install-doc: doc/score.1 doc/train.1 doc/predict.1 doc/info.1 doc/grt.1 doc/preprocess.1 doc/extract.1 doc/postprocess.1 doc/unpack.1 doc/pack.1
	$(INSTALL_PROGRAM) -D doc/grt.1 "$(DESTDIR)$(MANDIR)/man1/grt.1"
	$(INSTALL_PROGRAM) -D doc/score.1 "$(DESTDIR)$(MANDIR)/man1/grt-score.1"
	$(INSTALL_PROGRAM) -D doc/info.1 "$(DESTDIR)$(MANDIR)/man1/grt-info.1"
	$(INSTALL_PROGRAM) -D doc/train.1 "$(DESTDIR)$(MANDIR)/man1/grt-train.1"
	$(INSTALL_PROGRAM) -D doc/preprocess.1 "$(DESTDIR)$(MANDIR)/man1/grt-preprocess.1"
	$(INSTALL_PROGRAM) -D doc/postprocess.1 "$(DESTDIR)$(MANDIR)/man1/grt-postprocess.1"
	$(INSTALL_PROGRAM) -D doc/extract.1 "$(DESTDIR)$(MANDIR)/man1/grt-extract.1"
	$(INSTALL_PROGRAM) -D doc/predict.1 "$(DESTDIR)$(MANDIR)/man1/grt-predict.1"
	$(INSTALL_PROGRAM) -D doc/pack.1 "$(DESTDIR)$(MANDIR)/man1/grt-pack.1"
	$(INSTALL_PROGRAM) -D doc/unpack.1 "$(DESTDIR)$(MANDIR)/man1/grt-unpack.1"

clean:
	rm -f $(ALL) *.o

test: doc/*.md regression/*.md install
	python3 doctest.py -p "$(DESTDIR)/$(BINDIR)" doc/*.md
	python3 doctest.py -p "$(DESTDIR)/$(BINDIR)" regression/*.md

doc/%.1: doc/%.md
	pandoc -s -t man $< -o $@

