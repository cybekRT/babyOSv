#include"VGA_regs_gc.hpp"
#include"VGA_regs.hpp"

void VGA::Registers::GraphicsController::Write()
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

	VGA::Registers::GC_GraphicsModeRegister gc5 = {
		.readMode = readMode,
		.writeMode = writeMode,
		.shiftColor256 = shiftColor256,
		.shiftInterleaved = shiftInterleaved,
		.hostOddEven = hostOddEven,
	};

	VGA::Registers::GC_MiscellaneousGraphicsRegister gc6 = {
		.alphanumericModeDisabled = alphanumericModeDisabled,
		.chainOE = chainOE,
		.memoryMapSelect = memoryMapSelect
	};

	VGA::Registers::GC_ColorDontCareRegister gc7 = {
		.colorDontCare = { colorDontCare[0], colorDontCare[1], colorDontCare[2], colorDontCare[3] }
	};

	VGA::Registers::GC_BitMaskRegister gc8 = {
		.bitMask = bitMask
	};

	gc0.Write();
	gc1.Write();
	gc2.Write();
	gc3.Write();
	gc4.Write();
	gc5.Write();
	gc6.Write();
	gc7.Write();
	gc8.Write();
}
