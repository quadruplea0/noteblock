#!/bin/fish

eval gcc test.c -o myprogram (pkg-config --cflags --libs gtk+-3.0)
