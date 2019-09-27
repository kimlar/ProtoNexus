#pragma once

#include <iostream>
#include <string>

#define USE_DEBUG
//#define USE_VERBOSE

#define PRINT(x) std::cout << x << std::endl
#define ERROR(x) std::cout << "ERROR: " << x << std::endl

#ifdef USE_DEBUG
#define DEBUG(x) std::cout << "DEBUG: " << x << std::endl
#else
#define DEBUG(x)
#endif

#ifdef USE_VERBOSE
#define VERBOSE(x) std::cout << x << std::endl
#else
#define VERBOSE(x)
#endif
