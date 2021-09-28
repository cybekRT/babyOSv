#!/bin/bash

SYMS=`/usr/local/osdev/bin/i386-elf-nm out/kernel.elf -CS --size-sort | cut -d' ' -f2`

echo $SYMS
size=0
for a in $SYMS; do
	echo $a
	size=$(($size + 0x$a))
done

echo "Size: $size"
