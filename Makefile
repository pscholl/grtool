CPPFLAGS=`pkg-config --cflags grt` -g -std=c++11 # -O0
#LDFLAGS=`pkg-config --libs grt` -lprofiler
LDFLAGS=`pkg-config --libs grt`
ALL=grt train predict info score preprocess extract

all: $(ALL) *.h

test: examples/*.md
	cd examples
	PATH=${PATH}:../ ../mdtest.py *.md

.PHONY: test

PREFIX=/usr
bindir=$(PREFIX)/bin
DESTDIR=
INSTALL_PROGRAM=install

install: $(ALL)
	$(INSTALL_PROGRAM) -D grt "$(DESTDIR)$(bindir)/grt"
	$(INSTALL_PROGRAM) -D train "$(DESTDIR)$(bindir)/grt-train"
	$(INSTALL_PROGRAM) -D predict "$(DESTDIR)$(bindir)/grt-predict"
	$(INSTALL_PROGRAM) -D preprocess "$(DESTDIR)$(bindir)/grt-preprocess"
	$(INSTALL_PROGRAM) -D extract "$(DESTDIR)$(bindir)/grt-extract"
	$(INSTALL_PROGRAM) -D score "$(DESTDIR)$(bindir)/grt-score"
	$(INSTALL_PROGRAM) -D info "$(DESTDIR)$(bindir)/grt-info"

clean:
	rm -f $(ALL)
