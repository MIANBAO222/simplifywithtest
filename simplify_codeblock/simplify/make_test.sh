#!/bin/sh
clang main_test.c ./stl/libtstl2cl.a -L./stl -I./stl/include -lssl -o main&&./main

