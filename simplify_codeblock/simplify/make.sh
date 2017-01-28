#!/bin/sh
gcc main.c ./stl/libtstl2cl.a -L./stl -I./stl/include -o main&&./main

