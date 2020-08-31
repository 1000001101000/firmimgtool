.PHONY: all clean

# Top directory for building complete system, fall back to this directory
ROOTDIR    ?= $(shell pwd)

NAME    = firmimgtool

CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc

CFLAGS ?= -Wall -Wextra #-Werror

objs = $(patsubst %.c, %.o, $(wildcard *.c))
hdrs = $(wildcard *.h)

%.o: %.c $(hdrs) Makefile
	@printf "  CC      $(subst $(ROOTDIR)/,,$(shell pwd)/$@)\n"
	@$(CC) $(CFLAGS) -c $< -o $@

firmimgtool: $(objs)
	@printf "  CC      $(subst $(ROOTDIR)/,,$(shell pwd)/$@)\n"
	@$(CC) -o $@ $^

all: firmimgtool

clean:
	@rm -f *.o
	@rm -f $(TARGET)


