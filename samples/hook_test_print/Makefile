CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O1 -g
LDFLAGS = -ldl
MODULE_NAME := $(notdir $(CURDIR))
TARGET = $(MODULE_NAME).so

all: $(TARGET)

$(TARGET): hook.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

