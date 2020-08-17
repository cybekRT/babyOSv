import sys

templateKeyEnum = """
namespace Keyboard
{{
	enum class KeyCode
	{{
		None = 0,
		{},

		Total
	}};

	const char* KeyCode2Str(Keyboard::KeyCode key);
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
namespace Keyboard
{{
	const char* KeyCode2Str(Keyboard::KeyCode key)
	{{
		switch(key)
		{{
	{}
			default:
				return "Invalid";
		}}
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

	index = int(parts[0], 16)
	key = parts[1]

	keys[index] = key

if pathOut.endswith(".h"):
	# Write header file
	with open(pathOut, "w") as f:
		f.write("/// File is auto generated!\n")
		f.write("#pragma once\n")
		f.write(templateKeyEnum.format(",\n\t\t".join(keysNames)))
elif pathOut.endswith(".cpp"):
	# Write source file
	with open(pathOut, "w") as f:
		f.write("/// File is auto generated!\n")
		data = ""
		for v in keys:
			data = data + "\t\tKeyboard::KeyCode::{},\n".format(v)
		f.write(templateScanCode2Key.format(data))

		data = ""
		for v in keysNames:
			data = data + templateKeyCode2StrPart.format(v, v)
		f.write(templateKeyCode2Str.format(data))
