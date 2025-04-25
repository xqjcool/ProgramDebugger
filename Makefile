CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -ldl
BUILD_DIR = build

INJECTOR = $(BUILD_DIR)/injector
TARGET = $(BUILD_DIR)/target

.PHONY: all clean samples

all: $(INJECTOR) $(TARGET) samples

$(INJECTOR): injector.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET): target.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -rdynamic -O1 -o $@ $<

samples:
	@for dir in samples/*; do \
		[ -f $$dir/Makefile ] && $(MAKE) -C $$dir; \
	done

clean:
	rm -rf $(BUILD_DIR)
	@for dir in samples/*; do \
		[ -f $$dir/Makefile ] && $(MAKE) -C $$dir clean; \
	done

