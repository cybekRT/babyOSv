#pragma once

#include"VGA_regs.hpp"

namespace VGA
{
	namespace Registers
	{
		struct GraphicsController
		{
			/**
			 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
			 * This field is used by Write Mode 0 and Write Mode 3 (See the Write Mode field.)
			 * In Write Mode 0, if the corresponding bit in the Enable Set/Reset field is set,
			 * and in Write Mode 3 regardless of the Enable Set/Reset field, the value of the
			 * bit in this field is expanded to 8 bits and substituted for the data of the
			 * respective plane and passed to the next stage in the graphics pipeline,
			 * which for Write Mode 0 is the Logical Operation unit and for Write Mode 3 is the Bit Mask unit.
			 */
			bool setResetValue[4];

			/**
			 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
			 * This field is used in Write Mode 0 (See the Write Mode field) to select
			 * whether data for each plane is derived from host data or from expansion of the respective bit in the Set/Reset field.
			 */
			bool setResetEnable[4];

			/**
			 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
			 * This field holds a reference color that is used by Read Mode 1 (See the Read Mode field.)
			 * Read Mode 1 returns the result of the comparison between this value and a location of display memory, modified by the Color Don't Care field.
			 */
			bool memoryPlaneWriteEnable[4];

			/**
			 * This field is used in Write Mode 0 and Write Mode 2 (See the Write Mode field.) The logical operation stage of the graphics pipeline is 32 bits wide (1 byte * 4 planes) and performs the operations on its inputs from the previous stage in the graphics pipeline and the latch register. The latch register remains unchanged and the result is passed on to the next stage in the pipeline. The results based on the value of this field are:
			 *
			 * 00b - Result is input from previous stage unmodified.
			 * 01b - Result is input from previous stage logical ANDed with latch register.
			 * 10b - Result is input from previous stage logical ORed with latch register.
			 * 11b - Result is input from previous stage logical XORed with latch register.
			 *
			 */
			GC_DataRotateRegister::LogicalOperation logicalOperation;

			/**
			 * This field is used in Write Mode 0 and Write Mode 3 (See the Write Mode field.)
			 * In these modes, the host data is rotated to the right by the value specified by the value of this field.
			 * A rotation operation consists of moving bits 7-1 right one position to bits 6-0, simultaneously wrapping bit 0 around to bit 7,
			 * and is repeated the number of times specified by this field.
			 */
			u8 rotateCount;

			/**
			 * This value of this field is used in Read Mode 0 (see the Read Mode field) to specify the display memory plane to transfer data from.
			 * Due to the arrangement of video memory, this field must be modified four times to read one or more pixels values in the planar video modes.
			 */
			u8 mapSelect;

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
			VGA::Registers::GC_GraphicsModeRegister::ReadMode readMode;

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
			VGA::Registers::GC_GraphicsModeRegister::WriteMode writeMode;

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

			/**
			 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
			 * This field selects the planes that are used in the comparisons made by Read Mode 1 (See the Read Mode field.)
			 * Read Mode 1 returns the result of the comparison between the value of the Color Compare field and a location of display memory.
			 * If a bit in this field is set, then the corresponding display plane is considered in the comparison.
			 * If it is not set, then that plane is ignored for the results of the comparison.
			 */
			bool colorDontCare[4];

			/**
			 * This field is used in Write Modes 0, 2, and 3 (See the Write Mode field.)
			 * It it is applied to one byte of data in all four display planes.
			 * If a bit is set, then the value of corresponding bit from the previous stage in the graphics pipeline is selected; otherwise the value of the corresponding bit in the latch register is used instead.
			 * In Write Mode 3, the incoming data byte, after being rotated is logical ANDed with this byte and the resulting value is used in the same way this field would normally be used by itself.
			 */
			u8 bitMask;

			void Read();
			void Write();
		};
	}
}