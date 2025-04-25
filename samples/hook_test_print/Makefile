CC = gcc
CFLAGS = -Wall -Wextra -O1 -g
LDFLAGS = -ldl
MODULE_NAME := $(notdir $(CURDIR))
SO_TARGET = $(MODULE_NAME).so
TEST_TARGET = $(MODULE_NAME)_target

TARGET_SRC := $(wildcard target.c)

all: $(SO_TARGET) $(TEST_TARGET)

$(SO_TARGET): hook.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TARGET_SRC)
	$(CC) -rdynamic -o $@ $^

clean:
	rm -f $(SO_TARGET) $(TEST_TARGET) *.so

