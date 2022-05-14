#!/usr/bin/env python3
import re

files = [ "Array", "List" ]

content = []
for f in files:
	f = open(f + ".cpp", "r")
	content = content + [ f.read() ]
	f.close()

REGEX='TEST[_F]*\\((\w+).+?(\w+)\\)'

all_funcs = []
funcs_per_file = {}
for a in range(0, len(files)):
	matches = re.findall(REGEX, content[a])
	for match in matches:
		if not match[1] in all_funcs:
			all_funcs += [ match[1] ]

	funcs_per_file[files[a]] = [ match[1] for match in matches ]

all_funcs.sort()

maxFuncLen = max(len(s) for s in all_funcs)

print(" " * maxFuncLen, end="")
for f in files:
	print("{}\t".format(f), end="")
print()

for func in all_funcs:
	print(func + " " * (maxFuncLen - len(func)), end="")
	for f in files:
		cont = (func in funcs_per_file[f])
		print("{}\t".format( ("+" if cont else "-")), end="")
	print("")
