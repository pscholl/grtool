CPPFLAGS=`pkg-config --cflags grt` -g -std=c++11 # -O0
LDFLAGS=`pkg-config --libs grt`
ALL=grt grt-train grt-predict grt-info grt-score

all: $(ALL) *.h

test: examples/*.md
	cd examples
	PATH=${PATH}:../ ../mdtest.py *.md

.PHONY: test

PREFIX=/usr
bindir=$(PREFIX)/bin
DESTDIR=
INSTALL_PROGRAM=install

install: grt grt-train grt-predict grt-info grt-score
	$(INSTALL_PROGRAM) -D grt "$(DESTDIR)$(bindir)/grt"
	$(INSTALL_PROGRAM) -D grt-train "$(DESTDIR)$(bindir)/grt-train"
	$(INSTALL_PROGRAM) -D grt-predict "$(DESTDIR)$(bindir)/grt-predict"
	$(INSTALL_PROGRAM) -D grt-score "$(DESTDIR)$(bindir)/grt-score"
	$(INSTALL_PROGRAM) -D grt-info "$(DESTDIR)$(bindir)/grt-info"

clean:
	rm -f $(ALL)
