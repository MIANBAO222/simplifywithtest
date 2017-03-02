#!/bin/sh
clang main_test_with_map.c ./stl/libtstl2cl.a -L./stl -I./stl/include -lssl -lcrypto -Wall `pkg-config fuse3 --cflags --libs` -o main_map&&./main_map

