#!/bin/bash

rm memos-1.o memos-1 memos-1_test

as --32 memos-1.s -o memos-1.o
ld -T memos-1.ld memos-1.o -o memos-1
dd bs=1 if=memos-1 of=memos-1_test skip=4096 count=512
