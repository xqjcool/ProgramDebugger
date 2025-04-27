CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -ldl

INJECTOR = samples/injector
INJECTOR_C = src/injector.c

.PHONY: all clean samples

all: $(INJECTOR) samples

$(INJECTOR): $(INJECTOR_C)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

samples:
	@for dir in samples/*; do \
		[ -f $$dir/Makefile ] && $(MAKE) -C $$dir; \
	done

clean:
	rm -rf $(INJECTOR)
	@for dir in samples/*; do \
		[ -f $$dir/Makefile ] && $(MAKE) -C $$dir clean; \
	done

