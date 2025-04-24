CC = gcc
CFLAGS = -Wall -Wextra -O1 -g
LDFLAGS = -ldl
BUILD_DIR = build

INJECTOR_SRC = injector.c
INJECTOR_BIN = $(BUILD_DIR)/injector

SAMPLE_SRC = sample.c
SAMPLE_SO = $(BUILD_DIR)/sample.so

TARGET_SRC = target.c
TARGET_BIN = $(BUILD_DIR)/target

.PHONY: all clean dirs

all: dirs $(INJECTOR_BIN) $(SAMPLE_SO) $(TARGET_BIN)

dirs:
	mkdir -p $(BUILD_DIR)

$(INJECTOR_BIN): $(INJECTOR_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(SAMPLE_SO): $(SAMPLE_SRC)
	$(CC) $(CFLAGS) -fPIC -shared -ldl -o $@ $<

$(TARGET_BIN): $(TARGET_SRC)
	$(CC) $(CFLAGS) -rdynamic -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

