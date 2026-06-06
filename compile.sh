#!/bin/fish

eval gcc test.c -o noteblock (pkg-config --cflags --libs gtk+-3.0)
