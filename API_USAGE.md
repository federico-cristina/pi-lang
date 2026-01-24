# PI DLL API Usage Guide

This guide explains how to use the `PI_Api(T)` and `PI_CCONV` macros for creating cross-platform shared libraries (DLLs).

## Overview

The API macros in [api.h](api.h) provide a simple way to mark functions for export/import in shared libraries across Windows, Linux, macOS, and other Unix-like systems.

## Build Configurations

### Building as a Shared Library (DLL)

When building PI as a shared library, define `PI_BUILD_DLL`:

```cmake
# CMakeLists.txt
add_library(pi-common SHARED ${SOURCES})
target_compile_definitions(pi-common PRIVATE PI_BUILD_DLL)
```

Or with compiler flags:

```bash
# GCC/Clang
gcc -DPI_BUILD_DLL -shared -o libpi.so *.c

# MSVC
cl /DPI_BUILD_DLL /LD *.c
```

### Using a Shared Library (DLL)

When linking against PI as a shared library on Windows, define `PI_USE_DLL`:

```cmake
# CMakeLists.txt
add_executable(myapp main.c)
target_link_libraries(myapp pi-common)
target_compile_definitions(myapp PRIVATE PI_USE_DLL)
```

Or with compiler flags:

```bash
# MSVC
cl /DPI_USE_DLL main.c pi-common.lib

# MinGW
gcc -DPI_USE_DLL main.c -lpi-common
```

### Building as a Static Library

When building as a static library (default), don't define any special macros:

```cmake
# CMakeLists.txt
add_library(pi-common STATIC ${SOURCES})
# No special definitions needed
```

## Usage Examples

### Declaring Functions

Use `PI_Api(T)` where `T` is the return type:

```c
// In header file (e.g., pi/common/common.h)
#include "pi/common/api.h"

// Simple void function
PI_Api(void) pi_InitSystem(void);

// Function returning a pointer
PI_Api(void *) pi_malloc(size_t size);

// Function returning int
PI_Api(int) pi_GetErrorCode(void);

// Function returning const char*
PI_Api(const char *) pi_GetVersion(void);
```

### Implementing Functions

In the implementation file, use the same `PI_Api(T)` macro:

```c
// In source file (e.g., common.c)
#include "pi/common/common.h"

PI_Api(void) pi_InitSystem(void) {
    // Implementation
}

PI_Api(void *) pi_malloc(size_t size) {
    // Implementation
    return result;
}
```

### Advanced: Using PI_API and PI_CCONV Separately

For complex declarations, you can use the underlying macros:

```c
// Function pointer type
typedef PI_API void (PI_CCONV *PiCallback)(int code);

// Array of function pointers
PI_API extern void (PI_CCONV *callbacks[])(void);

// Complex return type
PI_API struct PiData* PI_CCONV pi_GetData(void);
```

## Compiler-Specific Expansion

### On Windows (MSVC)

```c
// Building DLL:
PI_Api(void) myFunc(void);
// Expands to:
__declspec(dllexport) void __cdecl myFunc(void);

// Using DLL:
PI_Api(void) myFunc(void);
// Expands to:
__declspec(dllimport) void __cdecl myFunc(void);

// Static library:
PI_Api(void) myFunc(void);
// Expands to:
void myFunc(void);
```

### On Windows (GCC/MinGW)

```c
// Building DLL:
PI_Api(void) myFunc(void);
// Expands to:
__declspec(dllexport) void __cdecl myFunc(void);

// Using DLL:
PI_Api(void) myFunc(void);
// Expands to:
__declspec(dllimport) void __cdecl myFunc(void);
```

### On Linux/Unix (GCC/Clang)

```c
// Building shared library:
PI_Api(void) myFunc(void);
// Expands to:
__attribute__((visibility("default"))) void myFunc(void);

// Using shared library:
PI_Api(void) myFunc(void);
// Expands to:
void myFunc(void);

// Note: Compile with -fvisibility=hidden for best results
gcc -fvisibility=hidden -shared -o libpi.so *.c
```

## Best Practices

1. **Always use `PI_Api(T)` for public API functions** - Any function that should be accessible from outside the library should use this macro.

2. **Don't use for internal/static functions** - Functions marked `static` or meant to be internal should not use `PI_Api(T)`.

3. **Consistent usage** - Use the macro in both declaration (header) and definition (source file).

4. **Visibility flags on Unix** - When building shared libraries on Linux/Unix, use `-fvisibility=hidden` to hide symbols by default:

   ```bash
   gcc -fvisibility=hidden -shared -fPIC -o libpi.so *.c
   ```

5. **CMake integration** - In CMakeLists.txt:

   ```cmake
   if(BUILD_SHARED_LIBS)
       target_compile_definitions(pi-common PRIVATE PI_BUILD_DLL)
       if(UNIX)
           target_compile_options(pi-common PRIVATE -fvisibility=hidden)
       endif()
   endif()
   ```

## Troubleshooting

### Linker Errors on Windows

If you get "unresolved external symbol" errors:

- Make sure `PI_USE_DLL` is defined when linking against the DLL
- Verify the DLL was built with `PI_BUILD_DLL` defined

### Symbol Not Found on Linux

If you get symbol errors on Linux:

- Check that `-fvisibility=hidden` was used when building the shared library
- Verify the symbol is marked with `PI_Api(T)`
- Use `nm -D libpi.so` to inspect exported symbols

### Wrong Calling Convention

If you get crashes or stack corruption:

- Ensure all declarations and definitions use `PI_Api(T)` consistently
- Don't mix decorated and undecorated declarations

## Reference

- **PI_Api(T)** - Main macro for declaring exported functions with return type T
- **PI_API** - Lower-level macro for export/import attribute only
- **PI_CCONV** - Calling convention macro (__cdecl on Windows, empty on Unix)
- **PI_BUILD_DLL** - Define when building the shared library
- **PI_USE_DLL** - Define when using the shared library (Windows only)
- **PI_PLATFORM_WINDOWS** - Internal platform detection macro
