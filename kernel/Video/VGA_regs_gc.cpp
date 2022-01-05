#include"VGA_regs_gc.hpp"
#include"VGA_regs.hpp"

void VGA::Registers::GC::Write()
{
	VGA::Registers::GC_SetResetRegister gc0 = {
		.setResetValue = { setResetValue[0], setResetValue[1], setResetValue[2], setResetValue[3] }
	};

	VGA::Registers::GC_EnableSetResetRegister gc1 = {
		.setResetEnable = { setResetEnable[0], setResetEnable[1], setResetEnable[2], setResetEnable[3] }
	};

	VGA::Registers::GC_ColorCompareRegister gc2 = {
		.memoryPlaneWriteEnable = { memoryPlaneWriteEnable[0], memoryPlaneWriteEnable[1], memoryPlaneWriteEnable[2], memoryPlaneWriteEnable[3] }
	};

	VGA::Registers::GC_DataRotateRegister gc3 = {
		.logicalOperation = logicalOperation,
		.rotateCount = rotateCount
	};

	VGA::Registers::GC_ReadMapSelectRegister gc4 = {
		.mapSelect = mapSelect
	};


}
		struct GC_GraphicsModeRegister
			/**
			 * This field selects between two read modes, simply known as Read Mode 0, and Read Mode 1, based upon the value of this field:
			 *
			 * 0b - Read Mode 0: In this mode, a byte from one of the four planes is returned on read operations.
			 * 		The plane from which the data is returned is determined by the value of the Read Map Select field.
			 * 1b - Read Mode 1: In this mode, a comparison is made between display memory and a reference color defined by the Color Compare field.
			 * 		Bit planes not set in the Color Don't Care field then the corresponding color plane is not considered in the comparison.
			 * 		Each bit in the returned result represents one comparison between the reference color, with the bit being set if the comparison is true.
			 *
			 */
			enum ReadMode
			{
				RM_0 = 0,
			} readMode;

			/**
			 * This field selects between four write modes, simply known as Write Modes 0-3, based upon the value of this field:
			 *
			 * 00b - Write Mode 0: In this mode, the host data is first rotated as per the Rotate Count field,
			 * 		then the Enable Set/Reset mechanism selects data from this or the Set/Reset field.
			 * 		Then the selected Logical Operation is performed on the resulting data and the data in the latch register.
			 * 		Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register.
			 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
			 * 01b - Write Mode 1: In this mode, data is transferred directly from the 32 bit latch register to display memory,
			 * 		affected only by the Memory Plane Write Enable field. The host data is not used in this mode.
			 * 10b - Write Mode 2: In this mode, the bits 3-0 of the host data are replicated across all 8 bits of their respective planes.
			 * 		Then the selected Logical Operation is performed on the resulting data and the data in the latch register.
			 * 		Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register.
			 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
			 * 11b - Write Mode 3: In this mode, the data in the Set/Reset field is used as if the Enable Set/Reset field were set to 1111b.
			 * 		Then the host data is first rotated as per the Rotate Count field, then logical ANDed with the value of the Bit Mask field.
			 * 		The resulting value is used on the data obtained from the Set/Reset field in the same way that the Bit Mask field would ordinarily be used.
			 * 		to select which bits come from the expansion of the Set/Reset field and which come from the latch register.
			 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
			 *
			 */
			enum WriteMode
			{
				WM_0 = 0,
				WM_3 = 3,
			} writeMode;

			/**
			 * When set to 0, this bit allows bit 5 to control the loading of the shift registers.
			 * When set to 1, this bit causes the shift registers to be loaded in a manner that supports the 256-color mode.
			 */
			bool shiftColor256;

			/**
			 * When set to 1, this bit directs the shift registers in the graphics controller to format the serial data stream
			 * with even-numbered bits from both maps on even-numbered maps, and odd-numbered bits from both maps on the odd-numbered maps.
			 * This bit is used for modes 4 and 5.
			 */
			bool shiftInterleaved;

			/**
			 * When set to 1, this bit selects the odd/even addressing mode used by the IBM Color/Graphics Monitor Adapter.
			 * Normally, the value here follows the value of Memory Mode register bit 2 in the sequencer.
			 */
			bool hostOddEven;

		struct GC_MiscellaneousGraphicsRegister
			/**
			 * This bit controls alphanumeric mode addressing.
			 * When set to 1, this bit selects graphics modes, which also disables the character generator latches.
			 */
			bool alphanumericModeDisabled;

			/**
			 * Chain O/E -- Chain Odd/Even Enable
			 * "When set to 1, this bit directs the system address bit, A0, to be replaced by a higher-order bit.
			 * The odd map is then selected when A0 is 1, and the even map when A0 is 0."
			 */
			bool chainOE;

			/**
			 * This field specifies the range of host memory addresses that is decoded by the VGA hardware and mapped into display memory accesses.
			 * The values of this field and their corresponding host memory ranges are:
			 *
			 * 00b -- A0000h-BFFFFh (128K region)
			 * 01b -- A0000h-AFFFFh (64K region)
			 * 10b -- B0000h-B7FFFh (32K region)
			 * 11b -- B8000h-BFFFFh (32K region)
			 *
			 */
			u8 memoryMapSelect;

		struct GC_ColorDontCareRegister
			/**
			 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
			 * This field selects the planes that are used in the comparisons made by Read Mode 1 (See the Read Mode field.)
			 * Read Mode 1 returns the result of the comparison between the value of the Color Compare field and a location of display memory.
			 * If a bit in this field is set, then the corresponding display plane is considered in the comparison.
			 * If it is not set, then that plane is ignored for the results of the comparison.
			 */
			bool colorDontCare[4];

		struct GC_BitMaskRegister
			/**
			 * This field is used in Write Modes 0, 2, and 3 (See the Write Mode field.)
			 * It it is applied to one byte of data in all four display planes.
			 * If a bit is set, then the value of corresponding bit from the previous stage in the graphics pipeline is selected; otherwise the value of the corresponding bit in the latch register is used instead.
			 * In Write Mode 3, the incoming data byte, after being rotated is logical ANDed with this byte and the resulting value is used in the same way this field would normally be used by itself.
			 */
			u8 bitMask;