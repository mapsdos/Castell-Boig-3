#ifndef _GRAPHICS_CONFIG_INCLUDE
#define _GRAPHICS_CONFIG_INCLUDE

// 1. Define GLEW_STATIC if you are linking statically (optional but common)
// #define GLEW_STATIC 

// 2. THE FIX: Include GLEW first
#include <GL/glew.h>

// 3. Include GLFW after
#include <GLFW/glfw3.h>

// 4. If you are on Windows, sometimes you need this to prevent gl.h conflicts
#ifdef _WIN32
#include <windows.h>
#endif

#endif