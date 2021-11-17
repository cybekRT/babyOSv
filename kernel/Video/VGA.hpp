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

	struct GraphicsMode
	{
		bool shiftColor256;
		bool shiftInterleaved;

		void Read();
		void Write();
	};

	struct RWLogic
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

		bool memoryPlaneWriteEnable[4];
		bool setResetEnable[4];
		bool setResetValue[4];

		enum LogicalOperation
		{
			LO_Normal	= 0,
			LO_AND		= 1,
			LO_OR		= 2,
			LO_XOR		= 3,
		} logicalOperation;

		u8 rotateCount;
		u8 bitMask;

		void Read();
		void Write();
	};

	bool Init();
}