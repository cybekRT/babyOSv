#pragma once

enum class Status
{
	Success = 0,
	Timeout,

	// IO
	IOError,
	EndOfFile,

	Undefined = 0xFE
};