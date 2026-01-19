# SourceMod VScript Extension

## Overview
A comprehensive [SourceMod](http://www.sourcemod.net/) extension that provides full VScript integration. Access and manipulate VScript from SourcePawn with support for scopes, tables, arrays, functions, and complete API wrapping.

## Supported Games

**Currently Supported:**
- Left 4 Dead 2 (fully tested)

**Other Games:**
- Other Source Engine games with VScript support have not been tested and may require additional work

## Features

### Core VScript Integration
* **Scope Management**: Access root scope, create custom scopes, and manage scope lifecycle
* **Table Operations**: Create, read, modify VScript tables with full key-value support
* **Array Support**: Create and manipulate VScript arrays with element access
* **Function Execution**: Call VScript functions with variable arguments and return values
* **Type System**: Complete ScriptVariant support for all VScript types (int, float, bool, string, vector, table, array, function)

### VScript API Access
* **Function Lookup**: Look up any VScript function by name via `LookupFunction()`
* **Wrapper Library**: Auto-generated wrappers for 400+ VScript functions
* **API Tables**: Organized access to VScript APIs:
	- `GlobalAPI` - Global Squirrel functions (format, print, type, etc.)
	- `CDirectorAPI` - Director functions (game mode, difficulty, etc.)
	- `ConvarsAPI` - Console variable access
	- `CEntitiesAPI` - Entity enumeration and search
	- `CNetPropManagerAPI` - Network property access
	- `CBaseEntityAPI` - Entity methods (Kill, SetOrigin, etc.)
	- And more...

### Advanced Features
* **Variadic Functions**: Full support for functions with variable arguments
* **Memory Management**: Automatic handle cleanup with proper ownership tracking
* **Type Conversion**: Seamless conversion between SourcePawn and VScript types
* **Error Handling**: Safe execution with proper error reporting

### Build Instructions
```sh
# Clone the repository
git clone https://github.com/ProjectSky/sm-ext-vscript.git
cd sm-ext-vscript

# Configure and build
mkdir build && cd build
python ../configure.py --enable-optimize --sm-path=YOUR_SOURCEMOD_PATH --hl2sdk-root=YOUR_SDK_PATH --mms-path=YOUR_MMS_PATH --sdks=l4d2
ambuild
```

## Usage

### Basic Example
```cpp
// Get root scope
VScriptScope root = VScriptScope.GetRoot();

// Execute VScript code directly
ScriptVariant result = root.Execute("return Convars.GetFloat(\"sv_gravity\")");

PrintToServer("sv_gravity: %f", result.Float);

// Clean up
delete result;
delete root;
```

### NetProps Example
```cpp
// Get player health using NetProps
VScriptScope root = VScriptScope.GetRoot();

// Execute VScript code with entity instance
ScriptVariant result = root.Execute("return NetProps.GetPropInt(GetPlayerFromUserID(%d), \"m_iHealth\")", GetClientUserId(client));

PrintToServer("Player health: %d", result.Int);

// Clean up
delete result;
delete root;
```

## Wrapper System

The extension includes an auto-generated wrapper library (`vscript_wrappers.nut`) that wraps all VScript functions to make them accessible via `LookupFunction()`.

### Why Wrappers?
C++ bound functions (userdata) cannot be found by `LookupFunction()` - only Squirrel-defined functions can be looked up by name. The wrapper library solves this by wrapping all VScript functions in Squirrel functions.

### Generating Wrappers
```sh
# Edit vscript_function_info.txt to add/modify function signatures
# Then regenerate wrappers
cd tools
python3 generate_wrappers.py
```

## Documentation

### API Reference
See [vscript.inc](scripting/include/vscript.inc) for complete API documentation.

### Function Reference
See [vscript_function_info.txt](tools/vscript_function_info.txt) for the complete list of wrapped VScript functions.

## Technical Notes

### Cross-Platform ScriptVariant_t Alignment Issue

During development, discovered a critical cross-platform compatibility issue with `ScriptVariant_t` structure alignment:

**Problem:**
- Windows expects 16-byte aligned `ScriptVariant_t`
- Linux expects 12-byte `ScriptVariant_t`
- Both platforms use the same header file definition from hl2sdk-l4d2

Through reverse engineering of the VScript DLLs:

1. **Linux (`vscript_srv.so`)** - Disassembly of `CSquirrelVM::ExecuteFunction`:

	Decompiled C code:
	```c
	// Parameter array iteration
	v23 = a3;  // args array pointer
	do {
		CSquirrelVM::PushVariant(a1, v23, 1);
		v23 += 12;  // ← 12-byte stride
	} while (v22 != a4);
	```

	assembly:
	```asm
	mov     dword ptr [esp+8], 1
	mov     [esp+4], esi
	mov     [esp], edi
	call    _ZN11CSquirrelVM11PushVariantERK12CVariantBaseI24CVariantDefaultAllocatorEb
	add     ebx, 1
	add     esi, 0Ch        ; ← 12-byte stride
	cmp     ebx, [ebp+arg_C]
	jnz     short loc_540D0
	```

2. **Windows (`vscript.dll`)** - Disassembly of `CSquirrelVM::ExecuteFunction`:

	Decompiled C code:
	```c
	// Parameter array iteration
	v21 = a3;  // args array pointer
	do {
		sub_1000FEE0(v7);  // PushVariant
		v21 += 16;  // ← 16-byte stride
	} while (v9);
	```

	assembly:
	```asm
	push    edi
	mov     eax, esi
	call    sub_1000FEE0
	add     esi, 10h        ; ← 16-byte stride
	dec     ebx
	jnz     short loc_1000C040
	```

**Suspected Root Cause:**

Based on disassembly analysis, the difference appears to be related to compiler-specific structure alignment, though the exact cause has not been fully confirmed:
- **GCC/Clang (Linux)**: Uses natural alignment, resulting in 12-byte structure
- **MSVC (Windows)**: Appears to use 8-byte alignment, padding to 16 bytes

Observed structure layout:
```cpp
struct ScriptVariant_t {
	union { ... };      // 4 bytes
	int m_extra;        // 4 bytes (L4D2-specific field, discovered via decompiled)
	int16 m_type;       // 2 bytes
	int16 m_flags;      // 2 bytes
	// Windows have 4 bytes padding here?
};
```

**Solution:**

Implemented platform-specific handling in `CallWithArgs`:
```cpp
#ifdef _WIN32
	// Windows: 16-byte aligned array
	struct AlignedVariant {
		ScriptVariant_t variant;
		int padding;
	};
	AlignedVariant args[16];
#else
	// Linux: 12-byte array
	ScriptVariant_t args[16];
#endif
```

## TODO
- [x] Basic VScript integration (scopes, tables, arrays, functions)
- [x] ScriptVariant type system
- [x] Function lookup and execution
- [x] Wrapper generator for VScript functions
- [x] Variadic function support
- [x] Memory management and handle cleanup
- [x] Entity instance conversion helpers
- [ ] Investigate ScriptVariant_t structure definition and cross-platform alignment differences (Win 16-byte, Linux 12-byte?)
- [ ] More usage examples and documentation
- [ ] Support for additional Source Engine games
