# Project 2 - Memory Debugger

## Compilers  
- Cover which compilers your application targets
    - [ ] gcc [version]  
    - [ ] clang [version]  
    - [x] msvc/Visual Studio 2019 Windows SDK 10.0
    

## Integration  
- Add #include "MyCRT.h" to the file that is using the debugger. Recommended to place in stdafx.h, and set the precompiled header to be forced included in every file of the project.
- The memory debugger is set such that it is only activated when _DEBUG is defined.

## Requirements  
- Required to be on Windows platform.

## Output  
- In the event of a memory leak
  - At the end of the program leaks are outputted to the console as well as saved to a "leaks.log" file.
  - A total of x leaks will be reported, with each leak's call stack shown.
  - The format is
    - Memory
	- Call Stack
	  - <module> <module address> <function name> <(function address)>File: <filename> Line: <line number>
  
  - Example:
	  0 : Error: Leak detected at memory 01060FFC
	Call stack:
		project2  !0x00f78bf7 Allocate             (0x00f78b80+0x77) File: Memory Line: 29
		
		project2  !0x00f77d7b operator new         (0x00f77d50+0x2b) File: Memory Line: 119
		
		project2  !0x00f886cb project2_leaks       (0x00f88660+0x6b) File: tests Line: 37
		
		project2  !0x00f73ff8 main                 (0x00f73f30+0xc8) File: main Line: 25
		
		project2  !0x00f8a713 invoke_main          (0x00f8a6e0+0x33) File: exe_common Line: 77
		
		project2  !0x00f8a597 __scrt_common_main_seh (0x00f8a440+0x157) File: exe_common Line: 236
		
		project2  !0x00f8a42d __scrt_common_main   (0x00f8a420+0xd) File: exe_common Line: 324
		
		project2  !0x00f8a798 mainCRTStartup       (0x00f8a790+0x8) File: exe_main Line: 15
		
		KERNEL32  !0x757e6359 BaseThreadInitThunk  (0x757e6340+0x19)
		
		ntdll     !0x77d07b74 RtlGetAppContainerNamedObjectPath (0x77d07a90+0xe4)
		
		ntdll     !0x77d07b44 RtlGetAppContainerNamedObjectPath (0x77d07a90+0xb4)
	
	
  - "leak.log" will be created at the root directory. If this is run through Visual Studio, it will be in the project folder.
    - The number of leaks will be recorded, as well as their call stack.

## Additional Information
### Areas of difficulty:
  Understanding VirtualAlloc and how it solves buffer overflow but not underflow.
### Design Decisions:
  Used the Nifty Counter implementation of a singleton for the Memory Debugger Singleton. This was chosen because it was important
  that the Memory Debugger can be used in static initialization and static deinitialization. Thus, it is possible to have static 
  instantiations of classes that own memory, and the memory debugger still works.
  
  Another decision was not to use macros. The Memory.h overloads 4 news (array and no throw combinations), and 6 deletes (array and 
  no throw combinations). It uses Windows Symbols to retrieve modules and line information for memory leaks and debugging.
  
  It is possible to check in runtime the number of memory that has not been deallocated yet, by calling MemoryDebugger::GetInstance().DetectLeaks();
  
  This implementation was also checked against std::vector, and appears to be working properly. So it is not required to typename std data structures
  to apply the memory debugger.
  
  I chose to implement detection for buffer overflow because its far more common for people to have buffer overflows then underflows.
  
  I used a link list to track both allocations and deallocations. This was simply for ease of use. 
  
  I also implemented a small header block in front of any allocation that contains information about the allocation. Currently it only contains 
  the size of the requested allocation + header size, as well as the pairing of the allocation (new, new[], delete, delete[]...).
  
  Further improvements can be made if more time is dedicated, including
    - Splitting up the header block into its own class, so that it is more modular then what it currently is.
	- Allowing more allocations per page. Currently it is one allocation per page, for ease of writing.
	  - This would require a header block for the entire allocation, that contains how much memory in that block is alive.
	  - At that point, it would be necessary to turn it into a linked list memory manager (much more like Mead's CS180 C memory manager, then his CS280 one).
	    - Take Mead's CS180 (which allows for objects of different size in one memory manager) and add the dead zones and marking from his CS280 and I'd imagine
		that is a full scale Memory Manager.

  In summary: This is not a very efficient usage of the Memory Allocator. The use of VirtualAlloc for buffer underflow/overflow is very 
  inefficient, but works well. To use VirtualAlloc and make it more efficient would a lot more work.
### How many hours the project took you:
  15 hours.
