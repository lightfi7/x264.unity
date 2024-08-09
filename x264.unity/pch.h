// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <cmath>
#include <iostream>
#include <x264.h>

#endif //PCH_H


#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) int InitializeEncoder(int w, int h);
	__declspec(dllexport) int EncodeFrame(int w, int h, const char* in_buffer, char** out_buffer);
	__declspec(dllexport) void DisposeEncoder();

#ifdef __cplusplus
}
#endif