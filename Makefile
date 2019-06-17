# Nikolay Nikolov
# ECE373 Makefile for Assignment 2
# The  makefile for the hello.c module
# Few Notes from the textbook "Linux Device Drivers" by John Madieu

# A Make file is a special file used to execute a set of actions.
# Most importantly is the compilation of programs.
# In almost every kernel makefile, you will see  at least one instance of
# an obg-<X> variable, where <X> should be either y,m left blank, or n.This is
# used by the kernel makefile from the head of the kernel build system in
# a general manner.

KERNEL_DIR =/lib/modules/$(shell uname -r)/build

obj-m += A3.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean



