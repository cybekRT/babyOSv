#pragma once

#include"HAL.hpp"

#define RGB(r, g, b) (( r & 0b11100000 ) | (( g & 0b11000000 ) >> 3) | (( b & 0b11100000 ) >> 5))

namespace VGA
{
	u8 Read_3C0(u8 index);
	void Write_3C0(u8 index, u8 value);

	u8 Read_3C2();
	void Write_3C2(u8 value);

	u8 Read_3C4(u8 index);
	void Write_3C4(u8 index, u8 value);

	void Read_3C8(u8 index, u8* r, u8* g, u8* b);
	void Write_3C8(u8 index, u8 r, u8 g, u8 b);

	u8 Read_3CE(u8 index);
	void Write_3CE(u8 index, u8 value);

	u8 Read_3D4(u8 index);
	void Write_3D4(u8 index, u8 value);

	/**
	 * Misc
	 */

	struct MiscOutput
	{
		/**
		 * Determines the polarity of the vertical sync pulse and can be used (with HSP) to control the vertical size
		 * of the display by utilizing the autosynchronization feature of VGA displays.
		 * = 0 selects a positive vertical retrace sync pulse.
		 */
		bool vSyncPolarity;

		/**
		 * Determines the polarity of the horizontal sync pulse.
  		 * = 0 selects a positive horizontal retrace sync pulse.
		 */
		bool hSyncPolarity;

		/**
		 * Selects the upper/lower 64K page of memory when the system is in an eve/odd mode (modes 0,1,2,3,7).
  		 * = 0 selects the low page
  		 * = 1 selects the high page
		 */
		bool oddEvenPageSelect;

		/**
		 * This field controls the selection of the dot clocks used in driving the display timing.
		 * The standard hardware has 2 clocks available to it, nominally 25 Mhz and 28 Mhz.
		 * It is possible that there may be other "external" clocks that can be selected by programming this register with the undefined values.
		 * The possible valuse of this register are:
		 *
    	 * 00 -- select 25 Mhz clock (used for 320/640 pixel wide modes)
    	 * 01 -- select 28 Mhz clock (used for 360/720 pixel wide modes)
    	 * 10 -- undefined (possible external clock)
    	 * 11 -- undefined (possible external clock)
		*/
		enum ClockSelect
		{
			Clock_25MHz	= 0b00,
			Clock_28MHz	= 0b01,
			Undefined1	= 0b10,
			Undefined2	= 0b11,
		} clockSelect;

		/**
		 * Controls system access to the display buffer.
		 * = 0 disables address decode for the display buffer from the system
		 * = 1 enables address decode for the display buffer from the system
		 */
		bool ramEnabled;

		/**
		 * This bit selects the CRT controller addresses.
		 * When set to 0, this bit sets the CRT controller addresses to 0x03Bx and the address
		 * for the Input Status Register 1 to 0x03BA for compatibility withthe monochrome adapter.
		 * When set to 1, this bit sets CRT controller addresses to 0x03Dx and
		 * the Input Status Register 1 address to 0x03DA for compatibility with the color/graphics adapter.
		 * The Write addresses to the Feature Control register are affected in the same manner.
		 */
		bool inputOutputAddressSelect;

		void Read()
		{
			u8 reg = Read_3C2();
			vSyncPolarity = !!(reg & (1 << 7));
			hSyncPolarity = !!(reg & (1 << 6));
			oddEvenPageSelect = !!(reg & (1 << 5));

			clockSelect = (ClockSelect)((reg & 0b1100) >> 2);

			ramEnabled = !!(reg & (1 << 1));
			inputOutputAddressSelect = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C2();
			reg &= ~(0b11101111);
			reg |= ((int)vSyncPolarity) << 7;
			reg |= ((int)hSyncPolarity) << 6;
			reg |= ((int)oddEvenPageSelect) << 5;
			reg |= ((int)clockSelect) << 2;
			reg |= ((int)ramEnabled) << 1;
			reg |= ((int)inputOutputAddressSelect) << 0;
			Write_3C2(reg);
		}
	};

	struct InputStatus
	{
		/**
		 * When set to 1, this bit indicates that the display is in a vertical retrace interval.
		 * This bit can be programmed, through the Vertical Retrace End register,
		 * to generate an interrupt at the start of the vertical retrace.
		 */
		bool vRetrace;

		/**
		 * When set to 1, this bit indicates a horizontal or vertical retrace interval.
		 * This bit is the real-time status of the inverted 'display enable' signal.
		 * Programs have used this status bit to restrict screen updates to the inactive display
		 * intervals in order to reduce screen flicker.
		 * The video subsystem is designed to eliminate this software requirement;
		 * screen updates may be made at any time without screen degradation.
		 */
		bool displayDisabled;

		void Read()
		{
			u8 reg = HAL::In8(0x3DA);// Read_3DA();
			vRetrace = !!(reg & (1 << 3));
			displayDisabled = !!(reg & (1 << 0));
		}
	};

	/**
	 * Sequencer
	 */

	/**
	 * The Sequencer Registers are accessed via a pair of registers, the Sequencer Address Register and the Sequencer Data Register. See the Accessing the VGA Registers section for more detals. The Address Register is located at port 3C4h and the Data Register is located at port 3C5h.
	 *
     * Index 00h -- Reset Register
     * Index 01h -- Clocking Mode Register
     * Index 02h -- Map Mask Register
     * Index 03h -- Character Map Select Register
     * Index 04h -- Sequencer Memory Mode Register
	 *
	 */

	struct Seq_00 // Reset Register
	{
		/**
		 * When set to 0, this bit commands the sequencer to synchronously clear and halt.
		 * Bits 1 and 0 must be 1 to allow the sequencer to operate.
		 * To prevent the loss of data, bit 1 must be set to 0 during the active display interval
		 * before changing the clock selection.
		 * The clock is changed through the Clocking Mode register or the Miscellaneous Output register.
		 */
		bool syncReset;

		/**
		 * When set to 0, this bit commands the sequencer to asynchronously clear and halt.
		 * Resetting the sequencer with this bit can cause loss of video data
		 */
		bool asyncReset;

		void Read()
		{
			u8 reg = Read_3C4(0x00);
			syncReset = !!(reg & (1 << 1));
			asyncReset = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x00);
			reg &= ~(0b11);
			reg |= ((int)syncReset) << 1;
			reg |= ((int)asyncReset) << 0;
			Write_3C4(0x00, reg);
		}
	};

	struct Seq_01 // Clocking Mode Register
	{
		/**
		 * When set to 1, this bit turns off the display and assigns maximum memory bandwidth to the system.
		 * Although the display is blanked, the synchronization pulses are maintained.
		 * This bit can be used for rapid full-screen updates.
		 */
		bool screenDisabled;

		/**
		 * When the Shift 4 field and the Shift Load Field are set to 0, the video serializers are loaded
		 * every character clock.
		 * When the Shift 4 field is set to 1, the video serializers are loaded every forth character clock,
		 * which is useful when 32 bits are fetched per cycle and chained together in the shift registers.
		 */
		bool shift4Enabled;

		/**
		 * When set to 0, this bit selects the normal dot clocks derived from the sequencer master clock input.
		 * When this bit is set to 1, the master clock will be divided by 2 to generate the dot clock.
		 * All other timings are affected because they are derived from the dot clock.
		 * The dot clock divided by 2 is used for 320 and 360 horizontal PEL modes.
		 */
		bool dotClockRate;

		/**
		 * When this bit and bit 4 are set to 0, the video serializers are loaded every character clock.
		 * When this bit is set to 1, the video serializers are loaded every other character clock,
		 * which is useful when 16 bits are fetched per cycle and chained together in the shift registers.
		 * The Type 2 video behaves as if this bit is set to 0; therefore, programs should set it to 0.
		 */
		bool shiftLoadRate;

		/**
		 * This field is used to select whether a character is 8 or 9 dots wide.
		 * This can be used to select between 720 and 640 pixel modes (or 360 and 320) and also is used to
		 * provide 9 bit wide character fonts in text mode.
		 * The possible values for this field are:
		 *
    	 * 0 - Selects 9 dots per character.
    	 * 1 - Selects 8 dots per character.
		 *
		 */
		bool dot8Mode;

		void Read()
		{
			u8 reg = Read_3C4(0x01);
			screenDisabled = !!(reg & (1 << 5));
			shift4Enabled = !!(reg & (1 << 4));
			dotClockRate = !!(reg & (1 << 3));
			shiftLoadRate = !!(reg & (1 << 2));
			dot8Mode = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x01);
			reg &= ~(0b00111101);
			reg |= ((int)screenDisabled) << 5;
			reg |= ((int)shift4Enabled) << 4;
			reg |= ((int)dotClockRate) << 3;
			reg |= ((int)shiftLoadRate) << 2;
			reg |= ((int)dot8Mode) << 0;
			Write_3C4(0x01, reg);
		}
	};

	struct Seq_02 // MapMaskRegister
	{
		/**
		 * Bits 3-0 of this field correspond to planes 3-0 of the VGA display memory.
		 * If a bit is set, then write operations will modify the respective plane of display memory.
		 * If a bit is not set then write operations will not affect the respective plane of display memory.
		 */
		bool memoryPlaneWriteEnabled[4];

		void Read()
		{
			u8 reg = Read_3C4(0x02);
			memoryPlaneWriteEnabled[0] = !!(reg & (1 << 0));
			memoryPlaneWriteEnabled[1] = !!(reg & (1 << 1));
			memoryPlaneWriteEnabled[2] = !!(reg & (1 << 2));
			memoryPlaneWriteEnabled[3] = !!(reg & (1 << 3));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x02);
			reg &= ~(0b00001111);
			reg |= ((int)memoryPlaneWriteEnabled[0]) << 0;
			reg |= ((int)memoryPlaneWriteEnabled[1]) << 1;
			reg |= ((int)memoryPlaneWriteEnabled[2]) << 2;
			reg |= ((int)memoryPlaneWriteEnabled[3]) << 3;
			Write_3C4(0x02, reg);
		}
	};

	struct Seq_03 // CharacterMapSelect
	{
		/**
		 * This field is used to select the font that is used in text mode when bit 3 of the attribute byte for a character is set to 1. Note that this field is not contiguous in order to provide EGA compatibility. The font selected resides in plane 2 of display memory at the address specified by this field, as follows:
		 *
    	 * 000b -- Select font residing at 0000h - 1FFFh
    	 * 001b -- Select font residing at 4000h - 5FFFh
    	 * 010b -- Select font residing at 8000h - 9FFFh
    	 * 011b -- Select font residing at C000h - DFFFh
    	 * 100b -- Select font residing at 2000h - 3FFFh
    	 * 101b -- Select font residing at 6000h - 7FFFh
    	 * 110b -- Select font residing at A000h - BFFFh
    	 * 111b -- Select font residing at E000h - FFFFh
		 *
		 */
		u8 characterSetASelect;
		/**
		 * This field is used to select the font that is used in text mode when bit 3 of the attribute byte for a character is set to 0. Note that this field is not contiguous in order to provide EGA compatibility. The font selected resides in plane 2 of display memory at the address specified by this field, identical to the mapping used by Character Set A Select above.
		 */
		u8 characterSetBSelect;

		void Read()
		{
			u8 reg = Read_3C4(0x03);
			characterSetASelect = ((reg & 0b00100000) >> 3) | ((reg & 0b1100) >> 2);
			characterSetBSelect = ((reg & 0b00010000) >> 2) | ((reg & 0b0011) >> 0);
		}

		void Write()
		{
			u8 reg = Read_3C4(0x03);
			reg &= ~(0b00111111);
			reg |= (characterSetASelect & 0b100) << 3;
			reg |= (characterSetASelect & 0b011) << 2;
			reg |= (characterSetBSelect & 0b100) << 2;
			reg |= (characterSetBSelect & 0b011) << 0;
			Write_3C4(0x03, reg);
		}
	};

	struct Seq_04 // SequencerMemoryMode
	{
		/**
		 * This bit controls the map selected during system read operations.
		 * When set to 0, this bit enables system addresses to sequentially access data within a bit
		 * map by using the Map Mask register.
		 * When set to 1, this bit causes the two low-order bits to select the map accessed as shown below.
		 * Address Bits
		 * A0 A1            Map Selected
		 * 0   0              0
		 * 0   1              1
		 * 1   0              2
		 * 1   1              3
		 */
		bool chain4Enabled;

		/**
		 * When this bit is set to 0, even system addresses access maps 0 and 2, while odd system addresses access
		 * maps 1 and 3.
		 * When this bit is set to 1, system addresses sequentially access data within a bit map,
		 * and the maps are accessed according to the value in the Map Mask register (index 0x02).
		 */
		bool oddEvenHostMemoryWriteAddressingDisabled;

		/**
		 * When set to 1, this bit enables the video memory from 64KB to 256KB.
		 * This bit must be set to 1 to enable the character map selection described for the previous register.
		 */
		bool extendedMemory;

		void Read()
		{
			u8 reg = Read_3C4(0x04);
			chain4Enabled = !!(reg & 0b1000);
			oddEvenHostMemoryWriteAddressingDisabled = !!(reg & 0b0100);
			extendedMemory = !!(reg & 0b0010);
		}

		void Write()
		{
			u8 reg = Read_3C4(0x04);
			reg &= ~(0b1110);
			reg |= ((int)chain4Enabled) << 3;
			reg |= ((int)oddEvenHostMemoryWriteAddressingDisabled) << 2;
			reg |= ((int)extendedMemory) << 1;
			Write_3C4(0x04, reg);
		}
	};

	/**
	 * Attribute controller -> http://www.osdever.net/FreeVGA/vga/attrreg.htm
	 */

	struct ATTR_PaletteRegister
	{
		/**
		 * "These 6-bit registers allow a dynamic mapping between the text attribute or graphic color input value and the display color on the CRT screen.
		 * When set to 1, this bit selects the appropriate color.
		 * The Internal Palette registers should be modified only during the vertical retrace interval to avoid problems with the displayed image.
		 * These internal palette values are sent off-chip to the video DAC, where they serve as addresses into the DAC registers."
		 */
		u8 paletteIndex[16];

		void Read()
		{
			for(unsigned a = 0; a < 16; a++)
				paletteIndex[a] = Read_3C0(a);
		}

		void Write()
		{
			for(unsigned a = 0; a < 16; a++)
				Write_3C0(a, paletteIndex[a]);

			//HAL::Out8(0x3C0, 0x20);
		}
	};

	struct ATTR_ModeControlRegister
	{
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

		void Read()
		{
			u8 reg = Read_3C0(0x10);
			paletteBits54Select =					!!(reg & 0b10000000);
			color8BitEnabled =						!!(reg & 0b01000000);
			pixelPanningMode =						!!(reg & 0b00100000);
			blinkEnabled =							!!(reg & 0b00001000);
			lineGraphicsEnabled =					!!(reg & 0b00000100);
			monochromeEmulation =					!!(reg & 0b00000010);
			attributeControllerGraphicsEnabled =	!!(reg & 0b00000001);
		}

		void Write()
		{
			u8 reg = Read_3C0(0x10);
			reg &= ~(0b11101111);
			reg |= ((int)paletteBits54Select) << 7;
			reg |= ((int)color8BitEnabled) << 6;
			reg |= ((int)pixelPanningMode) << 5;
			reg |= ((int)blinkEnabled) << 3;
			reg |= ((int)lineGraphicsEnabled) << 2;
			reg |= ((int)monochromeEmulation) << 1;
			reg |= ((int)attributeControllerGraphicsEnabled) << 0;
			Write_3C0(0x10, reg);
		}
	};

	struct ATTR_OverscanColorRegister
	{
		/**
		 *  "These bits select the border color used in the 80-column alphanumeric modes and in the graphics modes other than modes 4, 5, and D.
		 * (Selects a color from one of the DAC registers.)"
		 */
		u8 overscanPaletteIndex;

		void Read()
		{
			overscanPaletteIndex = Read_3C0(0x11);
		}

		void Write()
		{
			Write_3C0(0x11, overscanPaletteIndex);
		}
	};

	struct ATTR_ColorPlaneEnableRegister
	{
		/**
		 *  "Setting a bit to 1, enables the corresponding display-memory color plane."
		 */
		bool planeEnabled[4];

		void Read()
		{
			u8 reg = Read_3C0(0x12);
			planeEnabled[0] = !!(reg & 0b0001);
			planeEnabled[1] = !!(reg & 0b0010);
			planeEnabled[2] = !!(reg & 0b0100);
			planeEnabled[3] = !!(reg & 0b1000);
		}

		void Write()
		{
			u8 reg = Read_3C0(0x12);
			reg &= ~(0b00001111);
			reg |= ((int)planeEnabled[3]) << 3;
			reg |= ((int)planeEnabled[2]) << 2;
			reg |= ((int)planeEnabled[1]) << 1;
			reg |= ((int)planeEnabled[0]) << 0;
			Write_3C0(0x12, reg);
		}
	};

	struct ATTR_HorizontalPixelPanningRegister
	{
		/**
		 * "These bits select the number of pels that the video data is shifted to the left.
		 * PEL panning is available in both alphanumeric and graphics modes."
		 */
		u8 pixelShiftCount;

		void Read()
		{
			u8 reg = Read_3C0(0x13);
			pixelShiftCount = (reg & 0b00001111);
		}

		void Write()
		{
			u8 reg = Read_3C0(0x13);
			reg &= ~(0b00001111);
			reg |= (pixelShiftCount & 0b00001111);
			Write_3C0(0x13, reg);
		}
	};

	struct ATTR_ColorSelectRegister
	{
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

		void Read()
		{
			u8 reg = Read_3C0(0x14);
			colorSelect76 = (reg & 0b00001100) >> 2;
			colorSelect54 = (reg & 0b00000011) >> 0;
		}

		void Write()
		{
			u8 reg = Read_3C0(0x14);
			reg &= ~(0b00001111);
			reg |= (colorSelect76 & 0b11) << 2;
			reg |= (colorSelect54 & 0b11) << 0;
			Write_3C0(0x13, reg);
		}
	};

	bool Init();

	void SetCursor(bool enabled);
}