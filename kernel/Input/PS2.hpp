#pragma once

namespace PS2
{
	struct StatusRegister
	{
		/* (0 = empty, 1 = full) (must be set before attempting to read data from IO port 0x60) */
		u8 outputBuffer : 1;
		/* (0 = empty, 1 = full) (must be clear before attempting to write data to IO port 0x60 or IO port 0x64)  */
		u8 inputBuffer : 1;
		/* Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST)  */
		u8 systemFlag : 1;
		/* (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command) */
		u8 isData : 1;
		/* May be "keyboard lock" (more likely unused on modern systems)
		   May be "receive time-out" or "second PS/2 port output buffer full" */
		// u8 unused : 2;
		u8 unused : 1;
		u8 auxData : 1;
		/* 0 = no error, 1 = time-out error)  */
		u8 timeoutError : 1;
		/* (0 = no error, 1 = parity error) */
		u8 parityError : 1;
	} __attribute__((packed));

	struct ControllerConfiguration
	{
		/* 0 	First PS/2 port interrupt (1 = enabled, 0 = disabled) */
		u8 keyboardIrq : 1;
		/* 1 	Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported) */
		u8 mouseIrq : 1;
		/* 2 	System Flag (1 = system passed POST, 0 = your OS shouldn't be running) */
		u8 postPassed : 1;
		/* 3 	Should be zero */
		u8 _unused1 : 1;
		/* 4 	First PS/2 port clock (1 = disabled, 0 = enabled) */
		u8 disableKeyboard : 1;
		/* 5 	Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported) */
		u8 disableMouse : 1;
		/* 6 	First PS/2 port translation (1 = enabled, 0 = disabled) */
		u8 keyboardTranslation : 1;
		/* 7 	Must be zero  */
		u8 _unused2 : 1;
	};

	bool Init();
}