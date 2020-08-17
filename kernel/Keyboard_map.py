import sys

#print("Test {}".format(sys.argv[1]))

templateKeyEnum = """
namespace Keyboard
{{
	enum class KeyCode
	{{
		None = 0,
		{},

		Total
	}};
}}
"""

templateScanCode2Key = """

#include"Keyboard_map.h"
namespace Keyboard
{{
	KeyCode scanCode2Key[] = {{
{}
	}};
}}

"""

templateKeyCode2Str = """
char* KeyCode2Str(Keyboard::KeyCode key)
{{
	switch(key)
	{{
{}
		default:
			return "Invalid";
	}}
}}
"""

templateKeyCode2StrPart = """
		case Keyboard::KeyCode::{}:
			return "{}";
"""

pathOut = sys.argv[1]
pathIn = sys.argv[2]

keys = ["None"] * 128
keysNames = []
#print(keys)
#print(", ".join(keys))

with open(pathIn, "r") as f:
	lines = f.readlines()

for line in lines:
	if line.startswith("#"):
		continue

	parts = list(filter(None, [ a.strip(",\t\n") for a in line.split("\t") ]))

	if len(parts) < 1:
		continue

	scancode = parts[0]
	name = parts[1]

	if len(scancode.split(",")) > 1:
		continue;

	if name not in keysNames and name != "None":
		keysNames.append(name)

	print(parts)

	index = int(parts[0], 16)
	key = parts[1]

	keys[index] = key

	#print(parts)

	# try:
	# 	while True:
	# 		try:
	# 			line = f.readline()
	# 		except:
	# 			break

	# 		args = line.split(" ")
	# 		print(args)
	# except Exception as e:
	# 	print("Exception " + str(e))
	# 	#pass

#print(keys)

print(keysNames)

if pathOut.endswith(".h"):
	with open(pathOut, "w") as f:
		f.write("/// File is auto generated!")
		f.write("#pragma once\n")
		f.write(templateKeyEnum.format(",\n\t\t".join(keysNames)))
elif pathOut.endswith(".cpp"):
	with open(pathOut, "w") as f:
		f.write("/// File is auto generated!")
		data = ""
		for v in keys:
			data = data + "\t\tKeyboard::KeyCode::{},\n".format(v)
		f.write(templateScanCode2Key.format(data))
