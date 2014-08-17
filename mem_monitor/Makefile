#!/bin/sh
inc_dir=inc
src_dir=src
test_dir=test
objs_dir=objs
bin_dir=bin
CFLAGS += -I$(inc_dir)

MAKE = gcc $(CFLAGS)

objs=$(objs_dir)/mem_monitor.o $(objs_dir)/main.o
bin_mem_monitor=$(bin_dir)/mem_monitor

all: check $(bin_mem_monitor)

check:
	test -d $(bin_dir) || mkdir $(bin_dir)
	test -d $(objs_dir) || mkdir $(objs_dir)
	
$(bin_mem_monitor): $(objs)
	$(MAKE) -o $(bin_mem_monitor) $(objs)

$(objs_dir)/mem_monitor.o: $(src_dir)/mem_monitor.c
	$(MAKE) -c $(src_dir)/mem_monitor.c -o $(objs_dir)/mem_monitor.o

$(objs_dir)/main.o: $(test_dir)/main.c
	$(MAKE) -c $(test_dir)/main.c -o $(objs_dir)/main.o

clean:
	rm -rf bin objs
