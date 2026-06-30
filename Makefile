CC=gcc
STRIP=strip
CURDIR=$(PWD)
INCLUDEDIR=$(CURDIR)/include

FLAG=-O2 -I$(INCLUDEDIR) -g -fsanitize=address,leak -Wall
PROGNAME=test.t
SRCDIR=$(CURDIR)/src
all: main


main:
	@echo "Building test ..."
	@$(CC) -o $(CURDIR)/$(PROGNAME) $(FLAG) $(SRCDIR)/db_memory.c $(SRCDIR)/db.c main.c
	@echo "Finished"

.PHONY:
clean:
ifneq ("$(wildcard $(CURDIR)/$(PROGNAME))","")
	@echo "Removing $(CURDIR)/$(PROGNAME) ..."
	@rm -v $(CURDIR)/$(PROGNAME)
	@echo "Removed"
else
	@echo "Nothing to do. Skipping $(CURDIR)/$(PROGNAME)"
endif
