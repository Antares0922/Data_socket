#!/bin/bash

gcc -w -c modules/sqlite3.c -o sqlite3.o -Imodules

g++ -g server.cpp sqlite3.o -o server.exe -Imodules -lpthread

rm sqlite3.o
