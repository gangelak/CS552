#!/bin/bash

make clean
make pcr
qemu-system-i386 -m 128 -kernel fifos -serial file:output
