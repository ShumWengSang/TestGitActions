
#include "MemoryDebugger.h"
#include "StackWalker.h"

MemoryDebugger* MemoryDebugger::s_instance;
int MemoryDebuggerInitializer::s_count = 0;


MemoryDebuggerInitializer::MemoryDebuggerInitializer()
{
	if (++s_count == 1)
	{
		MemoryDebugger::s_instance = static_cast<MemoryDebugger*>(malloc(sizeof(MemoryDebugger)));
		MemoryDebugger::s_instance = new (MemoryDebugger::s_instance) MemoryDebugger;
	}
}

MemoryDebuggerInitializer::~MemoryDebuggerInitializer()
{
	if (--s_count == 0)
	{
		MemoryDebugger::s_instance->~MemoryDebugger();
		free(MemoryDebugger::s_instance);
	}
}

MemoryDebugger& MemoryDebugger::GetInstance()
{
	return *s_instance;
}

void MemoryDebugger::DetectLeaks()
{
	// Open to write to file
	std::ofstream leakFile("leak.log");
	MyCRT::StringStream stream;

	stream << "-------------------------- DETECTING LEAKS...----------------------------" << std::endl;

	auto size = allocatedList.size();
	std::cout.clear();
	if (size > 0)
	{
		stream << "Error: " << size << " leak detected! Outputting... " << std::endl;
		int i = 0;
		// Each item in list is a leak
		for (auto& element_ : this->allocatedList)
		{
			// Each one that is still allocated is a leak.
			stream << "  " << i << " : Error: Leak detected at memory " << element_.second << std::endl;
			stream << "  Call stack: " << std::endl;

			stream << element_.first;
			stream << std::endl;

			// Now we cout this and log to file
			std::cout << stream.str();
			leakFile << stream.str();
			stream.str(MyCRT::String());
			++i;
		}
	}
	
	leakFile.close();
}

void* MemoryDebugger::ReserveMemory(MemoryAddress const addr, size_t size, MEM_TYPE mem_type)
{
	unsigned char* ptr = static_cast<unsigned char*>(addr);

	// In case size > PAGE_SIZE
	size = (size % MyCRT::PAGE_SIZE) > 0 ? (size % MyCRT::PAGE_SIZE) : size;

	unsigned char* returnPtr = (unsigned char*)ptr + (MyCRT::PAGE_SIZE - size) + sizeof(size_t) + sizeof(MemoryDebugger::MEM_TYPE);

	// We will be storing the size here...
	GetSize(returnPtr) = size;
	// Store the Memtype
	GetMemoryType(returnPtr) = mem_type;

	// Return the pointer that we allow the user to use
	return returnPtr;
}

MemoryDebugger::MEM_TYPE& MemoryDebugger::GetMemoryType(MemoryAddress const addr)
{
	// TODO: insert return statement here
	return *(MEM_TYPE*)((unsigned char*)addr - sizeof(size_t) - sizeof(MEM_TYPE));

}

size_t& MemoryDebugger::GetSize(MemoryAddress const addr)
{
	return *(size_t*)((unsigned char*)addr - sizeof(size_t));
}

bool MemoryDebugger::isPairedCorrectly(MemoryAddress const addr, MEM_TYPE mem_type)
{
	MEM_TYPE internalType = GetMemoryType(addr);
	
	return mem_type == internalType;
}

bool MemoryDebugger::isDoubleDeleted(MemoryAddress addr)
{
	for(auto& mem = deallocatedList.begin(); mem != deallocatedList.end(); ++mem)
	{
		MemoryAddress memory = *mem;
		if (memory == addr)
			return true;
	}
	return false;
}

MemoryDebugger::MemoryList& MemoryDebugger::GetAllocatedList()
{
	return this->allocatedList;
}

MyCRT::list<MemoryAddress>& MemoryDebugger::GetDeallocatedList()
{
	return this->deallocatedList;
	// TODO: insert return statement here
}

MemoryDebugger::MemoryDebugger()
{
	StackInit();
}

MemoryDebugger::~MemoryDebugger()
{
	DetectLeaks();
}
