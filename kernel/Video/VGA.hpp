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

	/*
	 * CRTC
	 */

	struct HorizontalTiming
	{
		u8 total;
		u8 displayEnd;
		u8 blankingStart;
		u8 blankingEnd;
		u8 displaySkew;
		u8 retraceStart;
		u8 retraceEnd;

		void Read();
		void Write();
	};

	struct VerticalTiming
	{
		u16 total;
		u16 displayEnd;
		u16 blankingStart;
		u16 blankingEnd;
		u16 retraceStart;
		u8 retraceEnd;

		void Read();
		void Write();
	};

	/*
	 * Sequencer
	 */

	struct GC_05 // Graphics Mode Register
	{
		bool shiftColor256;
		bool shiftInterleaved;

		void Read()
		{
			u8 reg = Read_3CE(0x05);

			shiftColor256 = !!(reg & (1 << 6));
			shiftInterleaved = !!(reg & (1 << 5));
		}

		void Write()
		{
			u8 reg = Read_3CE(0x05);
			reg &= ~( (1 << 5) | (1 << 6) );
			if(shiftColor256)
				reg |= (1 << 6);
			if(shiftInterleaved)
				reg |= (1 << 5);

			Write_3CE(0x05, reg);
		}
	};

	struct GC_00 // SetResetRegister
	{
		bool setResetValue[4];

		void Read()
		{
			u8 setResetReg = Read_3CE(0x00);
			setResetValue[0] = !!(setResetReg & 0b0001);
			setResetValue[1] = !!(setResetReg & 0b0010);
			setResetValue[2] = !!(setResetReg & 0b0100);
			setResetValue[3] = !!(setResetReg & 0b1000);
		}

		void Write()
		{
			u8 setResetReg = Read_3CE(0x00);
			setResetReg &= ~(0b1111);
			setResetReg |= ((int)setResetValue[0]) << 0;
			setResetReg |= ((int)setResetValue[1]) << 1;
			setResetReg |= ((int)setResetValue[2]) << 2;
			setResetReg |= ((int)setResetValue[3]) << 3;
			Write_3CE(0x00, setResetReg);
		}
	};

	struct GC_01 // EnableSetResetRegister
	{
		bool setResetEnable[4];

		void Read()
		{
			u8 enableSetResetReg = Read_3CE(0x01);
			setResetEnable[0] = !!(enableSetResetReg & 0b0001);
			setResetEnable[1] = !!(enableSetResetReg & 0b0010);
			setResetEnable[2] = !!(enableSetResetReg & 0b0100);
			setResetEnable[3] = !!(enableSetResetReg & 0b1000);
		}

		void Write()
		{
			u8 enSetResetReg = Read_3CE(0x01);
			enSetResetReg &= ~(0b1111);
			enSetResetReg |= ((int)setResetEnable[0]) << 0;
			enSetResetReg |= ((int)setResetEnable[1]) << 1;
			enSetResetReg |= ((int)setResetEnable[2]) << 2;
			enSetResetReg |= ((int)setResetEnable[3]) << 3;
			Write_3CE(0x01, enSetResetReg);
		}
	};

	struct GC_02 // ColorCompareRegister
	{
		bool memoryPlaneWriteEnable[4];

		void Read()
		{
			u8 mapMaskRegister = Read_3C4(0x02);
			memoryPlaneWriteEnable[0] = !!(mapMaskRegister & 0b0001);
			memoryPlaneWriteEnable[1] = !!(mapMaskRegister & 0b0010);
			memoryPlaneWriteEnable[2] = !!(mapMaskRegister & 0b0100);
			memoryPlaneWriteEnable[3] = !!(mapMaskRegister & 0b1000);
		}

		void Write()
		{
			u8 mapMaskReg = Read_3C4(0x02);
			mapMaskReg &= ~(0b1111);
			mapMaskReg |= ((int)memoryPlaneWriteEnable[0]) << 0;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[1]) << 1;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[2]) << 2;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[3]) << 3;
			Write_3C4(0x02, mapMaskReg);
		}
	};

	struct GC_03 // DataRotateRegister
	{
		enum LogicalOperation
		{
			LO_Normal	= 0,
			LO_AND		= 1,
			LO_OR		= 2,
			LO_XOR		= 3,
		} logicalOperation;

		u8 rotateCount;

		void Read()
		{
			u8 dataRotateReg = Read_3CE(0x03);
			logicalOperation = (LogicalOperation)((dataRotateReg >> 3) & 0b11);
			rotateCount = (dataRotateReg >> 0) & 0b111;
		}

		void Write()
		{
			u8 dataRotateReg = Read_3CE(0x03);
			dataRotateReg &= ~(0b11111);
			dataRotateReg |= (logicalOperation << 3) & 0b11000;
			dataRotateReg |= (rotateCount << 0) & 0b111;
			Write_3CE(0x03, dataRotateReg);
		}
	};

	struct GC_04 // ReadMapSelectRegister
	{
		u8 mapSelect;

		void Read()
		{
			u8 v = Read_3CE(0x04);
			mapSelect = v & 0b11;
		}

		void Write()
		{
			u8 v = Read_3CE(0x04);
			v &= ~(0b11);
			v |= (mapSelect << 0) & 0b11;
			Write_3CE(0x04, v);
		}
	};

	struct GC_05 // GraphicsModeRegister
	{
		enum ReadMode
		{
			RM_0 = 0,
		} readMode;

		enum WriteMode
		{
			WM_0 = 0,
			WM_3 = 3,
		} writeMode;

		void Read()
		{
			u8 graphicsModeReg = Read_3CE(0x05);
			readMode = (ReadMode)((graphicsModeReg >> 3) & 0b1);
			writeMode = (WriteMode)((graphicsModeReg >> 0) & 0b11);
		}

		void Write()
		{
			u8 graphicsModeReg = Read_3CE(0x05);
			graphicsModeReg &= ~(0b1011);
			graphicsModeReg |= ((int)writeMode) << 0;
			graphicsModeReg |= ((int)readMode) << 3;
			Write_3CE(0x05, graphicsModeReg);
		}
	};

	struct GC_06 // Miscellaneous Graphics Register
	{
		/**
		 * Alpha Dis. -- Alphanumeric Mode Disable
		 * "This bit controls alphanumeric mode addressing. When set to 1, this bit selects graphics modes, which also disables the character generator latches."
		 */
		bool alphaDis;

		/**
		 * Chain O/E -- Chain Odd/Even Enable
		 * "When set to 1, this bit directs the system address bit, A0, to be replaced by a higher-order bit. The odd map is then selected when A0 is 1, and the even map when A0 is 0."
		 */
		bool chainOE;

		/**
		 *  Memory Map Select
		 * This field specifies the range of host memory addresses that is decoded by the VGA hardware and mapped into display memory accesses.  The values of this field and their corresponding host memory ranges are:
		 *
		 * 00b -- A0000h-BFFFFh (128K region)
		 * 01b -- A0000h-AFFFFh (64K region)
		 * 10b -- B0000h-B7FFFh (32K region)
		 * 11b -- B8000h-BFFFFh (32K region)
		 *
		 */
		u8 memoryMapSelect;

		void Read()
		{
			u8 v = Read_3CE(0x06);
			alphaDis = !!(v & 0b1);
			chainOE = !!(v & 0b10);
			memoryMapSelect = (v >> 2) & 0b11;
		}

		void Write()
		{
			u8 v = Read_3CE(0x06);
			v &= ~(0b1111);
			v |= (memoryMapSelect << 2) & 0b1100;
			v |= (chainOE << 1) & 0b10;
			v |= (alphaDis << 0) & 0b1;
			Write_3CE(0x06, v);
		}
	};

	struct GC_07 // ColorDontCareRegister
	{
		/**
		 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
		 * This field selects the planes that are used in the comparisons made by Read Mode 1 (See the Read Mode field.)
		 * Read Mode 1 returns the result of the comparison between the value of the Color Compare field and a location of display memory.
		 * If a bit in this field is set, then the corresponding display plane is considered in the comparison.
		 * If it is not set, then that plane is ignored for the results of the comparison.
		 */
		bool colorDontCare[4];

		void Read()
		{
			u8 v = Read_3CE(0x07);
			colorDontCare[0] = !!(v & 0b0001);
			colorDontCare[1] = !!(v & 0b0010);
			colorDontCare[2] = !!(v & 0b0100);
			colorDontCare[3] = !!(v & 0b1000);
		}

		void Write()
		{
			u8 v = Read_3C4(0x07);
			v &= ~(0b1111);
			v |= ((int)colorDontCare[0]) << 0;
			v |= ((int)colorDontCare[1]) << 1;
			v |= ((int)colorDontCare[2]) << 2;
			v |= ((int)colorDontCare[3]) << 3;
			Write_3C4(0x07, v);
		}
	};

	struct GC_08 // BitMaskRegister
	{
		/**
		 * This field is used in Write Modes 0, 2, and 3 (See the Write Mode field.)
		 * It it is applied to one byte of data in all four display planes.
		 * If a bit is set, then the value of corresponding bit from the previous stage in the graphics pipeline is selected; otherwise the value of the corresponding bit in the latch register is used instead.
		 * In Write Mode 3, the incoming data byte, after being rotated is logical ANDed with this byte and the resulting value is used in the same way this field would normally be used by itself.
		 */
		u8 bitMask;

		void Read()
		{
			bitMask = Read_3CE(0x08);
		}

		void Write()
		{
			Write_3CE(0x08, bitMask);
		}
	};

	bool Init();
}