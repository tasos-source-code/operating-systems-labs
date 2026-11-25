CC = gcc
CFLAGS = -Wall -g -D_FILE_OFFSET_BITS=64
FUSE_FLAGS = $(shell pkg-config fuse --cflags --libs)
SSL_FLAGS = $(shell pkg-config libssl libcrypto --cflags --libs)

TARGET = myfs
SRC = myfile_system.c log.c

ROOTDIR = ./fs/rootdir
MOUNTDIR = ./fs/mountdir
BLOCKSDIR = $(ROOTDIR)/blocks
METADATADIR = $(ROOTDIR)/metadata

TESTBENCH_SRC = ../experiments/testbench.c
TESTBENCH = testbench

.PHONY: all dirs mount unmount test clean clean-fs

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(FUSE_FLAGS) $(SSL_FLAGS) -o $(TARGET)

dirs:
	mkdir -p $(ROOTDIR)
	mkdir -p $(MOUNTDIR)
	mkdir -p $(BLOCKSDIR)
	mkdir -p $(METADATADIR)

mount: $(TARGET) dirs
	./$(TARGET) $(ROOTDIR) $(MOUNTDIR)

test: mount
	$(CC) -Wall -g $(TESTBENCH_SRC) -o $(TESTBENCH)
	./$(TESTBENCH) $(MOUNTDIR)

unmount:
	-fusermount -u $(MOUNTDIR)

clean: unmount
	rm -f $(TARGET)
	rm -f $(TESTBENCH)
	rm -rf ./fs