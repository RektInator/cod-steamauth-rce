#ifdef _LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack"
#endif

// defines
#define WIN32_LEAN_AND_MEAN

// windows headers
#include <windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <iostream>
#pragma comment (lib, "Ws2_32.lib")

// std includes
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <assert.h>

// add literal support
using namespace std::literals;

#ifdef _LINUX
#pragma clang diagnostic pop
#endif
