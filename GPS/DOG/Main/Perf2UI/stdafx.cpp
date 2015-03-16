// stdafx.cpp : source file that includes just the standard includes
// Perf2.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


void DebugTraceA(const char* fmt, ...)
{
	va_list va;
	char szBuffer[256] = "";

	va_start(va, fmt);
	vsnprintf_s(szBuffer, sizeof(szBuffer), sizeof(szBuffer)-1, fmt, va);
	va_end(va);

	szBuffer[sizeof(szBuffer)-1]=0;
	OutputDebugStringA(szBuffer);
}

