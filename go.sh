#!/bin/bash
make clean && make && echo -e "\n\n" && ./a.out $1 && echo -e "\n\n" && cat out.asm && echo -e "\n\n" && spim out.asm
