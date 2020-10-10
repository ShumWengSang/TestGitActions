#pragma once


// std lib
#include <new>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <stdexcept>
#include <list>
#include <iomanip>

// includes windows.h
#include "Project2Helper.h"
#include <memoryapi.h>
#include <DbgHelp.h>

// Mallocator
#include "mallocator.h"

namespace MyCRT
{
  using String = std::basic_string<char, std::char_traits<char>, Mallocator<char>>;

  using StringStream = std::basic_stringstream<char, std::char_traits<char>, Mallocator<char>>;

  template<typename T>
  using list = std::list<T, Mallocator<T>>;

  inline const int PAGE_SIZE = 4096;
}
