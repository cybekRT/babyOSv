#include"VGA.hpp"
#include"HAL.hpp"

namespace VGA
{
	u8 Read_3C0(u8 index)
	{
		HAL::In8(0x3DA);
		HAL::Out8(0x3C0, index);
		return HAL::In8(0x3C1);
	}

	void Write_3C0(u8 index, u8 value)
	{
		HAL::In8(0x3DA);
		HAL::Out8(0x3C0, index);
		HAL::Out8(0x3C0, value);
	}

	u8 Read_3C2()
	{
		return HAL::In8(0x3CC);
	}

	void Write_3C2(u8 value)
	{
		HAL::Out8(0x3C2, value);
	}

	u8 Read_3C4(u8 index)
	{
		HAL::Out8(0x3C4, index);
		return HAL::In8(0x3C5);
	}

	void Write_3C4(u8 index, u8 value)
	{
		HAL::Out8(0x3C4, index);
		return HAL::Out8(0x3C5, value);
	}

	// DAC
	void Read_3C8(u8 index, u8* r, u8* g, u8* b)
	{
		HAL::Out8(0x3C8, index);
		*r = HAL::In8(0x3C9);
		*g = HAL::In8(0x3C9);
		*b = HAL::In8(0x3C9);
	}

	void Write_3C8(u8 index, u8 r, u8 g, u8 b)
	{
		HAL::Out8(0x3C8, index);
		HAL::Out8(0x3C9, r);
		HAL::Out8(0x3C9, g);
		HAL::Out8(0x3C9, b);
	}

	u8 Read_3CE(u8 index)
	{
		HAL::Out8(0x3CE, index);
		return HAL::In8(0x3CF);
	}

	void Write_3CE(u8 index, u8 value)
	{
		HAL::Out8(0x3CE, index);
		HAL::Out8(0x3CF, value);
	}

	u8 Read_3D4(u8 index)
	{
		// Enable register
		// u8 v = Read_3C2();
		// v |= 1;
		// Write_3C2(v);

		HAL::Out8(0x3D4, index);
		return HAL::In8(0x3D5);
	}

	void Write_3D4(u8 index, u8 value)
	{
		// Enable register
		// u8 v = Read_3C2();
		// v |= 1;
		// Write_3C2(v);

		HAL::Out8(0x3D4, index);
		HAL::Out8(0x3D5, value);
	}

	/*
	 * Horizontal timing
	 */

	union EndHorizontalBlankingRegister
	{
		u8 value;
		struct
		{
			u8 blankingEnd0_4 : 5;
			u8 displaySkew : 2;
			u8 _unused1 : 1;
		};
	};

	union EndHorizontalRetraceRegister
	{
		u8 value;
		struct
		{
			u8 retraceEnd : 5;
			u8 _unused2 : 2;
			u8 blankingEnd5 : 1;
		};
	};

	void HorizontalTiming::Read()
	{
		EndHorizontalBlankingRegister ehbr;
		EndHorizontalRetraceRegister ehrr;

		total = Read_3D4(0x00);
		displayEnd = Read_3D4(0x01);
		blankingStart = Read_3D4(0x02);
		ehbr.value = Read_3D4(0x03);
		retraceStart = Read_3D4(0x04);
		ehrr.value = Read_3D4(0x05);

		displaySkew = ehbr.displaySkew;
		blankingEnd = (ehbr.blankingEnd0_4 | (ehrr.blankingEnd5 << 5));
		retraceEnd = ehrr.retraceEnd;
	}

	void HorizontalTiming::Write()
	{
		EndHorizontalBlankingRegister ehbr;
		ehbr.displaySkew = displaySkew;
		ehbr.blankingEnd0_4 = (blankingEnd & 0b11111);

		EndHorizontalRetraceRegister ehrr;
		ehrr.blankingEnd5 = (blankingEnd >> 5);
		ehrr.retraceEnd = retraceEnd;

		Write_3D4(0x00, total);
		Write_3D4(0x01, displayEnd);
		Write_3D4(0x02, blankingStart);
		Write_3D4(0x03, ehbr.value);
		Write_3D4(0x04, retraceStart);
		Write_3D4(0x05, ehrr.value);
	}

	/*
	 * Vertical timing
	 */

	union OverflowRegister
	{
		u8 value;
		struct
		{
			u8 total8 : 1;
			u8 displayEnd8 : 1;
			u8 retraceStart8 : 1;
			u8 blankingStart8 : 1;
			u8 _unused1 : 1;
			u8 total9 : 1;
			u8 displayEnd9 : 1;
			u8 retraceStart9 : 1;
		};
	};

	union MaximumScanLineRegister
	{
		u8 value;
		struct
		{
			u8 _unused1 : 5;
			u8 blankingStart9 : 1;
			u8 _unused2 : 2;
		};
	};

	union VerticalRetraceEndRegister
	{
		u8 value;
		struct
		{
			u8 retraceEnd : 4;
			u8 _unused1 : 4;
		};
	};

	void VerticalTiming::Read()
	{
		OverflowRegister oreg;
		MaximumScanLineRegister mslr;
		VerticalRetraceEndRegister vrer;
		u16 total0_7;
		u16 retraceStart0_7;
		u16 displayEnd0_7;
		u16 blankingStart0_7;

		total0_7 = Read_3D4(0x06);
		oreg.value = Read_3D4(0x07);
		mslr.value = Read_3D4(0x09);
		retraceStart0_7 = Read_3D4(0x06);
		vrer.value = Read_3D4(0x11);
		displayEnd0_7 = Read_3D4(0x12);
		blankingStart0_7 = Read_3D4(0x15);
		blankingEnd = Read_3D4(0x16);

		total = (total0_7) | (oreg.total8 << 8) | (oreg.total9 << 9);
		displayEnd = (displayEnd0_7) | (oreg.displayEnd8 << 8) | (oreg.displayEnd9 << 9);
		blankingStart = (blankingStart0_7) | (oreg.blankingStart8 << 8);
		blankingEnd;
		retraceStart = (retraceStart0_7) | (oreg.retraceStart8 << 8) | (oreg.retraceStart9 << 9);
		retraceEnd;
	}

	void VerticalTiming::Write()
	{
		OverflowRegister oreg;
		MaximumScanLineRegister mslr;
		VerticalRetraceEndRegister vrer;

		oreg.value = Read_3D4(0x07);
		mslr.value = Read_3D4(0x09);
		vrer.value = Read_3D4(0x16);

		oreg.retraceStart8 = !!(retraceStart & 0x100);
		oreg.retraceStart9 = !!(retraceStart & 0x200);
		oreg.displayEnd8 = !!(displayEnd & 0x100);
		oreg.displayEnd9 = !!(displayEnd & 0x200);
		oreg.total8 = !!(total & 0x100);
		oreg.total9 = !!(total & 0x200);
		oreg.blankingStart8 = !!(blankingStart & 0x100);

		mslr.blankingStart9 = !!(blankingStart & 0x200);

		vrer.retraceEnd = retraceEnd;

		Write_3D4(0x06, total & 0xff);
		Write_3D4(0x07, oreg.value);
		Write_3D4(0x09, mslr.value);
		Write_3D4(0x10, retraceStart & 0xff);
		Write_3D4(0x11, vrer.value);
		Write_3D4(0x12, displayEnd & 0xff);
		Write_3D4(0x15, blankingStart);
		Write_3D4(0x16, blankingEnd & 0x7f);
	}

	/*
	 * Sequencer
	 */

	void GraphicsMode::Read()
	{
		u8 reg = Read_3CE(0x05);

		shiftColor256 = !!(reg & (1 << 6));
		shiftInterleaved = !!(reg & (1 << 5));
	}

	void GraphicsMode::Write()
	{
		u8 reg = Read_3CE(0x05);
		reg &= ~( (1 << 5) | (1 << 6) );
		if(shiftColor256)
			reg |= (1 << 6);
		if(shiftInterleaved)
			reg |= (1 << 5);

		Write_3CE(0x05, reg);
	}

	/*
	 * RW logic
	 */

	void RWLogic::Read()
	{
		u8 graphicsModeReg = Read_3CE(0x05);
		readMode = (ReadMode)((graphicsModeReg >> 3) & 0b1);
		writeMode = (WriteMode)((graphicsModeReg >> 0) & 0b11);

		u8 mapMaskRegister = Read_3C4(0x02);
		memoryPlaneWriteEnable[0] = !!(mapMaskRegister & 0b0001);
		memoryPlaneWriteEnable[1] = !!(mapMaskRegister & 0b0010);
		memoryPlaneWriteEnable[2] = !!(mapMaskRegister & 0b0100);
		memoryPlaneWriteEnable[3] = !!(mapMaskRegister & 0b1000);

		u8 enableSetResetReg = Read_3CE(0x01);
		setResetEnable[0] = !!(enableSetResetReg & 0b0001);
		setResetEnable[1] = !!(enableSetResetReg & 0b0010);
		setResetEnable[2] = !!(enableSetResetReg & 0b0100);
		setResetEnable[3] = !!(enableSetResetReg & 0b1000);

		u8 setResetReg = Read_3CE(0x00);
		setResetValue[0] = !!(setResetReg & 0b0001);
		setResetValue[1] = !!(setResetReg & 0b0010);
		setResetValue[2] = !!(setResetReg & 0b0100);
		setResetValue[3] = !!(setResetReg & 0b1000);

		u8 dataRotateReg = Read_3CE(0x03);
		logicalOperation = (LogicalOperation)((dataRotateReg >> 3) & 0b11);
		rotateCount = (dataRotateReg >> 0) & 0b111;

		bitMask = Read_3CE(0x08);
	}

	void RWLogic::Write()
	{
		u8 graphicsModeReg = Read_3CE(0x05);
		graphicsModeReg &= ~(0b1011);
		graphicsModeReg |= ((int)writeMode) << 0;
		graphicsModeReg |= ((int)readMode) << 3;
		Write_3CE(0x05, graphicsModeReg);

		u8 mapMaskReg = Read_3C4(0x02);
		mapMaskReg &= ~(0b1111);
		mapMaskReg |= ((int)memoryPlaneWriteEnable[0]) << 0;
		mapMaskReg |= ((int)memoryPlaneWriteEnable[1]) << 1;
		mapMaskReg |= ((int)memoryPlaneWriteEnable[2]) << 2;
		mapMaskReg |= ((int)memoryPlaneWriteEnable[3]) << 3;
		Write_3C4(0x02, mapMaskReg);

		u8 enSetResetReg = Read_3CE(0x01);
		enSetResetReg &= ~(0b1111);
		enSetResetReg |= ((int)setResetEnable[0]) << 0;
		enSetResetReg |= ((int)setResetEnable[1]) << 1;
		enSetResetReg |= ((int)setResetEnable[2]) << 2;
		enSetResetReg |= ((int)setResetEnable[3]) << 3;
		Write_3CE(0x01, enSetResetReg);

		u8 setResetReg = Read_3CE(0x00);
		setResetReg &= ~(0b1111);
		setResetReg |= ((int)setResetValue[0]) << 0;
		setResetReg |= ((int)setResetValue[1]) << 1;
		setResetReg |= ((int)setResetValue[2]) << 2;
		setResetReg |= ((int)setResetValue[3]) << 3;
		Write_3CE(0x00, setResetReg);

		u8 dataRotateReg = Read_3CE(0x03);
		dataRotateReg &= ~(0b11111);
		dataRotateReg |= (logicalOperation << 3) & 0b11000;
		dataRotateReg |= (rotateCount << 0) & 0b111;
		Write_3CE(0x03, dataRotateReg);

		Write_3CE(0x08, bitMask);
	}

	/*
	 * Init
	 */

	bool Init()
	{
		// Enable reg 3D4
		u8 v = Read_3C2();
		v |= 1;
		Write_3C2(v);

		// Disable 3D4 write protect
		v = Read_3D4(0x11);
		v &= ~0x80;
		Write_3D4(0x11, v);





Write_3C0(0x10, 0x41);
Write_3C0(0x11, 0x00);
Write_3C0(0x12, 0x0F);
Write_3C0(0x13, 0x00);
Write_3C0(0x14, 0x00);
Write_3C2(0x63);
Write_3C4(0x01, 0x01);
Write_3C4(0x03, 0x00);
Write_3C4(0x04, 0x0E);
Write_3CE(0x05, 0x40);
Write_3CE(0x06, 0x05);
Write_3D4(0x00, 0x5F);
Write_3D4(0x01, 0x4F);
Write_3D4(0x02, 0x50);
Write_3D4(0x03, 0x82);
Write_3D4(0x04, 0x54);
Write_3D4(0x05, 0x80);
Write_3D4(0x06, 0xBF);
Write_3D4(0x07, 0x1F);
Write_3D4(0x08, 0x00);
Write_3D4(0x09, 0x41);
Write_3D4(0x10, 0x9C);
Write_3D4(0x11, 0x8E);
Write_3D4(0x12, 0x8F);
Write_3D4(0x13, 0x28);
Write_3D4(0x14, 0x40);
Write_3D4(0x15, 0x96);
Write_3D4(0x16, 0xB9);
Write_3D4(0x17, 0xA3);






		return true;
	}
}
