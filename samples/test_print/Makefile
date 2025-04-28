CC = gcc
CFLAGS = -Wall -O1 -g -I../../include
LDFLAGS = -ldl
MODULE_NAME := $(notdir $(CURDIR))
SO_TARGET = $(MODULE_NAME).so
TEST_TARGET = $(MODULE_NAME)_target
FUNCHOOK_C = ../../src/funchook.c
FUNCHOOK_O = ../../src/funchook.o

TARGET_SRC := $(wildcard target.c)

all: $(SO_TARGET) $(TEST_TARGET)

$(SO_TARGET): $(FUNCHOOK_C) hook.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TARGET_SRC)
	$(CC) -rdynamic -o $@ $^

clean:
	rm -f $(SO_TARGET) $(TEST_TARGET) $(FUNCHOOK_O)

