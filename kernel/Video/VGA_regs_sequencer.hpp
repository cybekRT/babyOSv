#pragma once

namespace VGA
{
	namespace Registers
	{
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
		struct Sequencer
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

			/**
			 * Bits 3-0 of this field correspond to planes 3-0 of the VGA display memory.
			 * If a bit is set, then write operations will modify the respective plane of display memory.
			 * If a bit is not set then write operations will not affect the respective plane of display memory.
			 */
			bool memoryPlaneWriteEnabled[4];

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

			void Read();
			void Write();
		};
	}
}