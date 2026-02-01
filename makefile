# Makefile for CopyCat Deduplicator
CC = gcc
CFLAGS = -Wall -O3 -Iinclude -I/ucrt64/include/ncurses
LIBS = -lncursesw -lssl -lcrypto
TARGET = copycat.exe

SRCDIR = src
SRCS = $(SRCDIR)/main.c $(SRCDIR)/deduplicator.c $(SRCDIR)/hashing.c \
       $(SRCDIR)/datastruct.c $(SRCDIR)/filesystem.c $(SRCDIR)/tui_interface.c

OBJS = main.o deduplicator.o hashing.o datastruct.o filesystem.o tui_interface.o
RES_OBJ = res.o

all: $(TARGET)

$(TARGET): $(OBJS) $(RES_OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RES_OBJ) $(LIBS)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(RES_OBJ): resource.rc
	windres resource.rc $(RES_OBJ)

clean:
	rm -f $(TARGET) $(OBJS) $(RES_OBJ)

.PHONY: all clean