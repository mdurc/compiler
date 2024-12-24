#!/bin/bash
make clean && make && echo -e "\n\n" && leaks --atExit -- ./a.out $1
