#!/bin/bash

as --32 vga16.s -o vga16.o
ld -T vga.ld vga16.o -o vga16
dd bs=1 if=vga16 of=vga16_test skip=4096 count=512
