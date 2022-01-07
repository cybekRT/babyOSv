#pragma once

namespace VGA
{
	namespace Registers
	{
		struct Attributes
		{
			/**
			 * "These 6-bit registers allow a dynamic mapping between the text attribute or graphic color input value and the display color on the CRT screen.
			 * When set to 1, this bit selects the appropriate color.
			 * The Internal Palette registers should be modified only during the vertical retrace interval to avoid problems with the displayed image.
			 * These internal palette values are sent off-chip to the video DAC, where they serve as addresses into the DAC registers."
			 */
			u8 paletteIndex[16];

			/**
			 * "This bit selects the source for the P5 and P4 video bits that act as inputs to the video DAC.
			 * When this bit is set to 0, P5 and P4 are the outputs of the Internal Palette registers.
			 * When this bit is set to 1, P5 and P4 are bits 1 and 0 of the Color Select register."
			 */
			bool paletteBits54Select;

			/**
			 *  "When this bit is set to 1, the video data is sampled so that eight bits are available to select a color in the 256-color mode (0x13).
			 * This bit is set to 0 in all other modes."
			 */
			bool color8BitEnabled;

			/**
			 * This field allows the upper half of the screen to pan independently of the lower screen.
			 * If this field is set to 0 then nothing special occurs during a successful line compare (see the Line Compare field.)
			 * If this field is set to 1, then upon a successful line compare, the bottom portion of the screen is displayed as
			 * if the Pixel Shift Count and Byte Panning fields are set to 0.
			 */
			bool pixelPanningMode;

			/**
			 *  "When this bit is set to 0, the most-significant bit of the attribute selects the background intensity (allows 16 colors for background).
			 * When set to 1, this bit enables blinking."
			 */
			bool blinkEnabled;

			/**
			 * This field is used in 9 bit wide character modes to provide continuity for the horizontal line characters in the range C0h-DFh.
			 * If this field is set to 0, then the 9th column of these characters is replicated from the 8th column of the character.
			 * Otherwise, if it is set to 1 then the 9th column is set to the background like the rest of the characters.
			 */
			bool lineGraphicsEnabled;

			/**
			 * This field is used to store your favorite bit. According to IBM,
			 * "When this bit is set to 1, monochrome emulation mode is selected. When this bit is set to 0, color |emulation mode is selected."
			 * It is present and programmable in all of the hardware but it apparently does nothing.
			 * The internal palette is used to provide monochrome emulation instead.
			 */
			bool monochromeEmulation;

			/**
			 *  "When set to 1, this bit selects the graphics mode of operation."
			 */
			bool attributeControllerGraphicsEnabled;

			/**
			 *  "These bits select the border color used in the 80-column alphanumeric modes and in the graphics modes other than modes 4, 5, and D.
			 * (Selects a color from one of the DAC registers.)"
			 */
			u8 overscanPaletteIndex;

			/**
			 *  "Setting a bit to 1, enables the corresponding display-memory color plane."
			 */
			bool planeEnabled[4];

			/**
			 * "These bits select the number of pels that the video data is shifted to the left.
			 * PEL panning is available in both alphanumeric and graphics modes."
			 */
			u8 pixelShiftCount;

			/**
			 * "In modes other than mode 13 hex, these are the two most-significant bits of the 8-bit digital color value to the video DAC.
			 * In mode 13 hex, the 8-bit attribute is the digital color value to the video DAC.
			 * These bits are used to rapidly switch between sets of colors in the video DAC."
			 */
			u8 colorSelect76;

			/**
			 * "These bits can be used in place of the P4 and P5 bits from the Internal Palette registers to form the 8-bit digital color value to the video DAC.
			 * Selecting these bits is done in the Attribute Mode Control  register (index 0x10).
			 * These bits are used to rapidly switch between colors sets within the video DAC."
			 */
			u8 colorSelect54;

			void Read();
			void Write();
		};
	}
}
