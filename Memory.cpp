
#include "Memory.h"
#include "Project2Helper.h"
#include "MemoryDebugger.h"
#include "StackWalker.h"

static void* PageAlignedAllocate(size_t size) 
{
	// Account if size is greater than page size.
	const size_t remainder = (size / MyCRT::PAGE_SIZE) > 0 ? (size / MyCRT::PAGE_SIZE) + 1 : 1;

	// Reserve enough pages so that we can detect underflow
	void* p = VirtualAlloc(0, MyCRT::PAGE_SIZE * (remainder + 1), MEM_RESERVE, PAGE_NOACCESS);
	// Commit the page such that one page is left reserved.
	p = VirtualAlloc(p, MyCRT::PAGE_SIZE * remainder, MEM_COMMIT, PAGE_READWRITE);

	return p;
}

static void PageAlignedDeallocation(void* address)
{
	// Get the size from the address
	size_t size = MemoryDebugger::GetSize(address);
	const size_t remainder = (size / MyCRT::PAGE_SIZE) + 1;

	// We need to find p, where p is the address given to us
	void* p = (unsigned char*)address - (MyCRT::PAGE_SIZE - size) - sizeof(size_t) - sizeof(MemoryDebugger::MEM_TYPE);

	VirtualFree(p, MyCRT::PAGE_SIZE * (remainder + 1), MEM_DECOMMIT);
}

static void* Allocate(size_t size, bool nothrow, MemoryDebugger::MEM_TYPE mem_type)
{
	MemoryAddress addr = PageAlignedAllocate(size + sizeof(size_t) + sizeof(MemoryDebugger::MEM_TYPE));
	
	// Store the context.
	CONTEXT c;
	GET_CONTEXT(c);

	MyCRT::String stackTrace = StackTrace(&c);
	// Error handling
	if (!nothrow)
	{
		if (addr == nullptr)
		{
			throw std::bad_alloc();
		}
	}
	else
	{
		if (addr == nullptr)
		{
			std::cout << "Error with no throw: " << std::endl;
			std::cout << stackTrace << std::endl;
			return addr;
		}
	}

		// Reserve the two bytes before it for memdbg use
	void* returnAdr = MemoryDebugger::ReserveMemory(addr, size + sizeof(size_t) + sizeof(MemoryDebugger::MEM_TYPE), mem_type);

	// Store into the linked list.
	MemoryDebugger::GetInstance().GetAllocatedList().emplace_back(std::pair(stackTrace, returnAdr));


	
	// Give to user
	return returnAdr;
}

static void Deallocate(MemoryAddress addr, MemoryDebugger::MEM_TYPE mem_type)
{
	if (addr == nullptr)
		return;

	// See if its deleted already
	if (MemoryDebugger::GetInstance().isDoubleDeleted(addr))
	{
		// Double delete! Error!
		DEBUG_BREAKPOINT();
		return;
	}	

	bool foundMem = false;
	// Find if address is in list
	auto& MemList = MemoryDebugger::GetInstance().GetAllocatedList();
	for(auto iter = MemList.begin(); iter != MemList.end(); ++iter)
	{
		if((*iter).second == addr)
		{
			// This is the one we want to remove.
			MemList.erase(iter);
			foundMem = true;
			break;
		}
	}

	// Did we find the one we wanted to delete?
	if(!foundMem)
	{
		// Error, the addr passed in is not allocated from us.
		DEBUG_BREAKPOINT();
		return;
	}

	// Check if the paired allo/deallo is correct
	if (!MemoryDebugger::GetInstance().isPairedCorrectly(addr, mem_type))
	{
		// Wrong pair of new/delete! Error!
		DEBUG_BREAKPOINT();
		return;
	}

	// Success
	// Add it into the deleted list.
	MemoryDebugger::GetInstance().GetDeallocatedList().push_back(addr);

	PageAlignedDeallocation(addr);
	
}

void* operator new(size_t size)
{
	return Allocate(size, false, MemoryDebugger::MEM_TYPE::NEW_SINGLE);

}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
	return Allocate(size, true, MemoryDebugger::MEM_TYPE::NEW_SINGLE_NO_THROW);
}

void* operator new[](size_t size)
{
	return Allocate(size, false, MemoryDebugger::MEM_TYPE::NEW_ARRAY);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
	return Allocate(size, true, MemoryDebugger::MEM_TYPE::NEW_ARRAY_NO_THROW);
}

void operator delete(void* address)
{
	DEBUG_BREAKPOINT();
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_SINGLE);
}

void operator delete(void* address, size_t size)
{
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_SINGLE);
}

void operator delete(void* address, const std::nothrow_t&) noexcept
{
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_SINGLE_NO_THROW);
}

void operator delete[](void* address)
{
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_ARRAY);
}

void operator delete[](void* address, size_t size)
{
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_ARRAY);
}

void operator delete[](void* address, const std::nothrow_t&) noexcept
{
	Deallocate(address, MemoryDebugger::MEM_TYPE::NEW_ARRAY_NO_THROW);
}
