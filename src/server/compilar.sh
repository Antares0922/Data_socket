#!/bin/bash

gcc -w -c modules/sqlite3.c -o sqlite3.o -Imodules
gcc -c modules/datos.c -o datos.o -Imodules

g++ -g server.cpp datos.o sqlite3.o -o server.exe -Imodules -lpthread

rm datos.o sqlite3.o
