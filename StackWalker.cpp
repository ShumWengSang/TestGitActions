
#include "StackWalker.h"
#include <cstdio>
#include <Dbghelp.h>

#pragma comment( lib, "dbghelp" )

#if defined(_M_AMD64)

static void FillStackFrame(STACKFRAME64& stack_frame, const CONTEXT& context) {
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = context.Rip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = context.Rsp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context.Rbp;
}

#define IMAGE_FILE_MACHINE_CURRENT IMAGE_FILE_MACHINE_AMD64
// x64
#elif defined(_M_IX86)

static void FillStackFrame(STACKFRAME64& stack_frame, const CONTEXT& context) {
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = context.Eip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = context.Esp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context.Ebp;
}

#define IMAGE_FILE_MACHINE_CURRENT IMAGE_FILE_MACHINE_I386
// x86
#else
#error Unsupported Architecture
#endif

// Initialize the debug help libarary
static void InitSym() {
	// Only initialize once
	static bool ImghelpInitilized = false;
	if (ImghelpInitilized) { return; }

	// Initilize IMAGEHLP.DLL
	SymInitialize(GetCurrentProcess(), NULL, true);
	// OPTIONAL: Add support for line # in stack trace using SymGetLineFromAddr()
	SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES);

	ImghelpInitilized = true;
}

static IMAGEHLP_LINE64 GetSymbols(DWORD64 address, bool& success) {
	IMAGEHLP_LINE64 line;
	success = true;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD displacement;
	if (!SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line)) 
	{
		//unsigned long error = GetLastError();
		//std::cout << "SymGetLineFromAddr64() for address " << address << " failed. "
		//	<< "Error: " << error << std::endl;
		success = false;
	}
	return line;
}

// Print the call stack entry for the given address
static MyCRT::String PrintCallStackEntry(DWORD64 addr) {

	static char PrintoutBuffer[2048] = {0};
	MyCRT::String StrResult;
	
	// Initialize the symbol information if not done already
	InitSym();

	// Find the module name
	IMAGEHLP_MODULE64 module = { 0 };
	module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	BOOL result = SymGetModuleInfo64(GetCurrentProcess(), addr, &module);

	// If an associated module was found, print its name
	int size = sprintf_s(PrintoutBuffer, "%-10s!0x%08x",
		(result) ? module.ModuleName : "",
		addr);
	StrResult += PrintoutBuffer;

	// Find the symbol name
	struct 
	{
		SYMBOL_INFO symbol_info;
		char buffer[MAX_PATH];
	} symbol = { 0 };
	symbol.symbol_info.SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol.symbol_info.MaxNameLen = MAX_PATH;
	DWORD64 symbol_offset = 0;
	result = SymFromAddr(GetCurrentProcess(), addr, &symbol_offset, &symbol.symbol_info);

	// If an associated symbol was found, print its name
	if (result) 
	{
		bool success;
		auto symbols = GetSymbols((DWORD64)symbol.symbol_info.Address, success);

		if (success)
		{
			MyCRT::String filename = symbols.FileName;
			// Remove directory if present.
	// Do this before extension removal incase directory has a period character.
			const size_t last_slash_idx = filename.find_last_of("\\/");
			if (std::string::npos != last_slash_idx)
			{
				filename.erase(0, last_slash_idx + 1);
			}

			// Remove extension if present.
			const size_t period_idx = filename.rfind('.');
			if (std::string::npos != period_idx)
			{
				filename.erase(period_idx);
			}

			sprintf_s(PrintoutBuffer, " %-20s (0x%08x+0x%x) File: %s Line: %ld",
				symbol.symbol_info.Name,
				(DWORD)symbol.symbol_info.Address,
				(DWORD)symbol_offset,
				filename.c_str(),
				symbols.LineNumber);
			StrResult += PrintoutBuffer;
		}
		else
		{
			sprintf_s(PrintoutBuffer, " %-20s (0x%08x+0x%x)",
				symbol.symbol_info.Name,
				(DWORD)symbol.symbol_info.Address,
				(DWORD)symbol_offset);
			StrResult += PrintoutBuffer;
		}
		
	}

	sprintf_s(PrintoutBuffer, "\r\n");
	StrResult += PrintoutBuffer;
	return StrResult;
}


#if defined(_M_IX86)
// This only works for x86
// For x64, use RtlCaptureContext()
// See: http://jpassing.wordpress.com/2008/03/12/walking-the-stack-of-the-current-thread/
__declspec(naked) DWORD _stdcall GetEIP() {
	_asm {
		mov eax, dword ptr[esp]
		ret
	};
}

__declspec(naked) DWORD _stdcall GetESP() {
	_asm {
		mov eax, esp
		ret
	};
}

__declspec(naked) DWORD _stdcall GetEBP() {
	_asm {
		mov eax, ebp
		ret
	};
}
#endif

MyCRT::String StackTrace(const CONTEXT* start_context)
{
	MyCRT::String StrResult;
	InitSym();

	// Fill the initial context information
	CONTEXT context = *start_context;

	// Fill the initial stack frame information
	STACKFRAME64 stack_frame = { 0 };
	FillStackFrame(stack_frame, context);

	// Traverse the stack
	while (true) {
		// Find the caller's parent
		BOOL result = StackWalk64(
			IMAGE_FILE_MACHINE_CURRENT, // IMAGE_FILE_MACHINE_I386 or IMAGE_FILE_MACHINE_AMD64
			GetCurrentProcess(), // Process
			GetCurrentThread(), // Thread
			&stack_frame, // Stack Frame Information
			&context, // Thread Context Information
			NULL, // Read Memory Call Back (Not Used)
			SymFunctionTableAccess64, // Function Table Accessor
			SymGetModuleBase64, // Module Base Accessor
			NULL); // Address Translator (Not Used)

		// Finished or untable to trace further
		if (result == FALSE) { break; }

		// Print the call stack entry
		StrResult += "    " + PrintCallStackEntry(stack_frame.AddrPC.Offset);
	}
	return StrResult;
}

void StackInit()
{
	InitSym();
}