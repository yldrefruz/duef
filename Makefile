# Makefile for duef - A tool for uncompressing .uecrash files
# Simple Makefile for building duef with zlib dependency

# Compiler settings
CC = gcc
CFLAGS = -O2 -Wall -std=c99
LDFLAGS = 

# Directories
ZLIB_DIR = zlib-1.3.1
BUILD_DIR = build

# Target executable
TARGET = duef
SOURCES = duef.c
OBJECTS = $(SOURCES:.c=.o)

# zlib settings
ZLIB_STATIC = $(ZLIB_DIR)/libz.a
ZLIB_OBJS = adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o zutil.o \
           compress.o uncompr.o gzclose.o gzlib.o gzread.o gzwrite.o

# Include paths
INCLUDES = -I$(ZLIB_DIR)

# Platform-specific definitions
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D_GNU_SOURCE
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
endif

# Main target
all: $(TARGET)

# Build duef executable
$(TARGET): $(OBJECTS) $(ZLIB_STATIC)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(ZLIB_STATIC)

# Compile duef.c
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build zlib static library
$(ZLIB_STATIC): $(addprefix $(ZLIB_DIR)/, $(ZLIB_OBJS))
	cd $(ZLIB_DIR) && $(AR) rc libz.a $(ZLIB_OBJS)
	cd $(ZLIB_DIR) && ranlib libz.a

# Compile zlib source files
$(ZLIB_DIR)/%.o: $(ZLIB_DIR)/%.c
	$(CC) $(CFLAGS) -I$(ZLIB_DIR) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	cd $(ZLIB_DIR) && rm -f $(ZLIB_OBJS) libz.a

# Install target (optional)
install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)

# Uninstall target (optional)
uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build $(TARGET) (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install $(TARGET) to /usr/local/bin"
	@echo "  uninstall- Remove $(TARGET) from /usr/local/bin"
	@echo "  help     - Show this help message"

# Mark phony targets
.PHONY: all clean install uninstall help