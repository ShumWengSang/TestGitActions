#pragma once
#include "MemoryDebugger.h"

// Global overload 
void* operator new(size_t size);
void* operator new(size_t size, const std::nothrow_t&) noexcept;
void* operator new[](size_t size);
void* operator new[](size_t size, const std::nothrow_t&) noexcept;
void operator delete(void* address);
void operator delete(void* address, size_t size); 
void operator delete(void* address, const std::nothrow_t&) noexcept;
void operator delete[](void* address);
void operator delete[](void* address, size_t size);
void operator delete[](void* address, const std::nothrow_t&) noexcept;
