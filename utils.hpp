#pragma once

#include <format>
#include <d3dx9.h>

#define SAFE_RELEASE(x) { if(x) x->Release(); x = nullptr; }

template<class... Args>
inline void OutputDebugFormat(std::string str, Args... args)
{
    OutputDebugString(std::format(str, args...).c_str());
}

#define OUTPUT_DEBUG_FUNCNAME OutputDebugFormat("\n{} called.\n", __func__)