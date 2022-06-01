#pragma once

namespace Serial
{
	typedef void (*Handler)(u8 port, u8 byte);

	bool Init();
	void Register(u8 port, Handler handler);
	void Unregister(u8 port, Handler handler);
};