#pragma once

#include "mallocator.h"

class MemoryDebugger
{ 
public:
	enum class MEM_TYPE : bool;
private:
	// Foward declaration
	using StackWalkData = MyCRT::String;
	using MemoryAddress = void*;
	using MemoryList = MyCRT::list<std::pair<StackWalkData, MemoryAddress>>;
	
	friend struct MemoryDebuggerInitializer;

public:
	static MemoryDebugger& GetInstance();
	static void* ReserveMemory(MemoryAddress const addr, size_t size, MEM_TYPE mem_type);
	static MEM_TYPE& GetMemoryType(MemoryAddress const addr);
	static size_t& GetSize(MemoryAddress const addr);
	static bool isPairedCorrectly(MemoryAddress const addr, MEM_TYPE mem_type);

	void DetectLeaks();
	bool isDoubleDeleted(MemoryAddress addr);


	MemoryList& GetAllocatedList();
	MyCRT::list<MemoryAddress>& GetDeallocatedList();


	MemoryDebugger();
	~MemoryDebugger();

private:

	static MemoryDebugger* s_instance;

	MemoryList allocatedList;
	MyCRT::list<MemoryAddress> deallocatedList;

public:
	enum class MEM_TYPE : bool
	{
		NEW_SINGLE,
		NEW_SINGLE_NO_THROW,
		NEW_ARRAY,
		NEW_ARRAY_NO_THROW
	};
};

struct MemoryDebuggerInitializer
{
	MemoryDebuggerInitializer();
	~MemoryDebuggerInitializer();
	static int s_count;
};

static MemoryDebuggerInitializer s_logInit;
