#pragma once

namespace Serial
{
	enum class WordLength
	{
		Bits_5 = 0b00,
		Bits_6 = 0b01,
		Bits_7 = 0b10,
		Bits_8 = 0b11,
	};

	enum class StopBits
	{
		Bits_1 = 0b0,
		Bits_2 = 0b1,
	};

	enum class Parity
	{
		None	= 0b000,
		Odd		= 0b001,
		Even	= 0b011,
		Mark	= 0b101,
		Space	= 0b111,
	};

	typedef void (*Handler)(u8 port, u8 byte);

	bool Init();
	void Register(u8 port, Handler handler);
	void Unregister(u8 port, Handler handler);

	bool Configure(u8 port, u32 baudRate, WordLength wordLength, StopBits stopBits, Parity parity, bool breakLine = false);
	bool SetReady(u8 port, bool rts, bool dtr);
	void ClearBuffers(u8 port);

	void Test(u8 port);

	bool ReadByte(u8 port, u8* value, u32 timeout);
	bool WriteByte(u8 port, u8 value, u32 timeout);
};