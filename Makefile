.PHONY: build

SRCDIR=src
BINDIR=bin

build:
	mkdir -p $(BINDIR)
	gcc -Wall -I$(SRCDIR) -s $(SRCDIR)/nogdb.c -o $(BINDIR)/nogdb

static:
	mkdir -p $(BINDIR)
	gcc -Wall -I$(SRCDIR) -s -static $(SRCDIR)/nogdb.c -o $(BINDIR)/nogdb
