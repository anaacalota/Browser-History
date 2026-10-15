# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS=browser

build: $(TARGETS)

browser: browser.c
	$(CC) $(CFLAGS) browser.c -o browser

pack:
	zip -FSr browser.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)

.PHONY: pack clean
