#pragma once

#if defined(_M_AMD64)
// x64
#define GET_CONTEXT(c) \
	do { \
		RtlCaptureContext(&c); \
	} while(0)
#elif defined(_M_IX86)
// This only works for x86
// For x64, use RtlCaptureContext()
// See: http://jpassing.wordpress.com/2008/03/12/walking-the-stack-of-the-current-thread/
DWORD _stdcall GetEIP();

DWORD _stdcall GetESP();

DWORD _stdcall GetEBP();

// Capture the context at the current location for the current thread
// This is a macro because we want the CURRENT function - not a sub-function
#define GET_CONTEXT(c) \
	do { \
		ZeroMemory(&c, sizeof(c)); \
		c.ContextFlags = CONTEXT_CONTROL; \
		c.Eip = GetEIP(); \
		c.Esp = GetESP(); \
		c.Ebp = GetEBP(); \
	} while(0)

#endif

// Given a context, returns a string that is the stack trace of the given context
MyCRT::String StackTrace(const CONTEXT* start_context);
void StackInit();
