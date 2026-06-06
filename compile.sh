#!/bin/fish
eval gcc noteblock.c -o noteblock (pkg-config --cflags --libs gtk+-3.0 libmpdclient)
