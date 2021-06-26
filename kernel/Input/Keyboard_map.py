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

	struct KeyInfo
	{{
		const char* name;
		const u8 asciiLow;
		const u8 asciiHigh;
	}};

	//const char* KeyCode2Str(Keyboard::KeyCode key);
}}
"""

templateScanCode2Key = """
#include"Keyboard_map.hpp"
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

templateKeyInfo = """
namespace Keyboard
{{
	KeyInfo keyInfo[] = {{
		{{ nullptr, 0, 0 }},
{}
	}};
}}
"""

pathOut = sys.argv[1]
pathIn = sys.argv[2]

keys = ["None"] * 128
keysNames = []
keysInfo = {}

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

	asciiLow = "0"
	asciiHigh = "0"

	if len(parts) > 2:
		asciiLow = "'{}'".format(str(parts[2]).lower())
		print("x: {}".format(asciiLow))
		if asciiLow == "\\t":
			asciiLow = "\t"
		elif asciiLow == "\\n":
			asciiLow = "\n"
		elif asciiLow == "\" \"":
			asciiLow = " "
	if len(parts) > 3:
		asciiHigh = "'{}'".format(parts[3])
	else:
		if not asciiLow.startswith("'\\") and asciiLow.lower() != asciiLow.upper():
			print("     {}".format(asciiLow.startswith("\\")))
			asciiHigh = asciiLow.upper()

	if len(scancode.split(",")) > 1:
		continue;

	if name not in keysNames and name != "None":
		keysNames.append(name)

	keysInfo[name] = ( name, asciiLow, asciiHigh )

	index = int(parts[0], 16)
	key = parts[1]

	keys[index] = key

if pathOut.endswith(".hpp"):
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

		#data = ""
		#for v in keysNames:
		#	data = data + templateKeyCode2StrPart.format(v, v)
		#f.write(templateKeyCode2Str.format(data))

		data = ""
		for v in keysNames:
			name = keysInfo[v][0]
			asciiLow = keysInfo[v][1]
			asciiHigh = keysInfo[v][2]
			data = data + "\t\t{{ \"{}\", {}, {}, }},\n".format(name, asciiLow, asciiHigh)
		f.write(templateKeyInfo.format(data))
