#!/bin/sh

( cd Microcode && ./build.sh ) && ( cd Assembler && ./build.sh )
