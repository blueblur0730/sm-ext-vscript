/**
 * =============================================================================
 * SourceMod VScript Extension
 * Copyright 2025-2026 ProjectSky
 * =============================================================================
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined _vscript_ext_included
 #endinput
#endif
#define _vscript_ext_included

/**
 * VScript Interface for SourceMod
 *
 * This include provides access to the Source Engine's VScript system,
 * allowing SourcePawn plugins to interact with Squirrel scripts.
 *
 * Key Concepts:
 * - VScriptScope: Represents a Squirrel scope/table (like global scope)
 * - VScriptTable: A Squirrel table (key-value dictionary)
 * - VScriptArray: A Squirrel array (indexed list)
 * - ScriptVariant: A wrapper for any Squirrel value
 * - VScriptFunction: A callable Squirrel function
 *
 * Memory Management:
 * - ALL handles MUST be deleted with delete when done (including GetRoot())
 * - ScriptVariant returned from Execute() or Call() must be deleted
 * - ScriptVariant created with FromXxx() must be deleted
 * - Handles returned from GetValue() must be deleted
 * - Handles become invalid on VM reset and CANNOT be persisted across map changes
 *
 * VM Reset Behavior:
 * - When the VScript VM resets (e.g., on map change), the VM releases all
 *   associated resources, making all related SourceMod handles unusable
 * - The extension automatically frees these handles during VM reset to prevent leaks
 * - Attempting to use a handle after VM reset will result in an invalid handle error
 *
 * Note: GetRoot() returns a handle that wraps the VM's root table.
 * You must still delete the handle to avoid SourceMod handle leaks,
 * but the underlying VScript object won't be released (it's owned by the VM).
 *
 * Example:
 *   VScriptScope root = VScriptScope.GetRoot();
 *   root.Run("printl(\"Hello from VScript!\")");
 *   delete root;  // Must delete to avoid handle leak
 */

// ============================================================================
// Enumerations
// ============================================================================

/**
 * Script field types for ScriptVariant_t
 */
enum ScriptFieldType {
	VSF_VOID = 0,             // No value / null
	VSF_FLOAT = 1,            // Float number
	VSF_VECTOR = 3,           // 3D vector (float[3])
	VSF_INTEGER = 5,          // Integer number
	VSF_BOOLEAN = 6,          // Boolean (true/false)
	VSF_CHARACTER = 8,        // Single byte (char)
	VSF_CSTRING = 31,         // C string (const char*)
	VSF_HSCRIPT = 32,         // HSCRIPT
	VSF_VARIANT = 33,         // Variant type
	VSF_UNSIGNED = 38,        // Unsigned integer
	VSF_QANGLE = 40           // QAngle (rotation)
}

// ============================================================================
// VScript - Base class for all VScript handles
// ============================================================================

/**
 * Base class for all VScript handles.
 * Do not instantiate directly - use derived classes.
 */
methodmap VScript < Handle {
	/**
	 * Check if this handle is valid.
	 *
	 * @return          True if handle is valid, false otherwise
	 */
	property bool IsValid {
		public native get();
	}

	/**
	 * Check if this handle is an array.
	 *
	 * @return          True if this is a VScriptArray, false otherwise
	 */
	property bool IsArray {
		public native get();
	}

	/**
	 * Check if this handle is a table.
	 *
	 * @return          True if this is a VScriptTable, false otherwise
	 */
	property bool IsTable {
		public native get();
	}

	/**
	 * Compile script code and return as a callable function.
	 *
	 * This pre-compiles script code for better performance when executing
	 * the same code multiple times. The compiled function can be called
	 * repeatedly without recompiling.
	 *
	 * IMPORTANT: The returned VScriptFunction handle MUST be deleted when done.
	 *
	 * Example:
	 *   VScriptFunction func = VScript.Compile("return function(x) { return x * 2; }");
	 *
	 *   // Call multiple times without recompiling
	 *   ScriptVariant arg = ScriptVariant.FromInt(10);
	 *   ScriptVariant result = func.CallWithArgs(null, arg);
	 *   PrintToServer("Result: %d", result.Int);  // 20
	 *
	 *   delete result;
	 *   delete arg;
	 *   delete func;  // Must delete!
	 *
	 * @param format    Script code format string
	 * @param ...       Format arguments
	 * @return          VScriptFunction handle (must be deleted), or null if compilation failed
	 */
	public static native VScriptFunction Compile(const char[] format, any ...);

	/**
	 * Run script code without returning a result.
	 * @param format    Script code format string
	 * @param ...       Format arguments
	 * @return          True if successful
	 */
	public static native bool Run(const char[] format, any ...);
}

// ============================================================================
// VScriptFunction - Script function
// ============================================================================

/**
 * Represents a callable Squirrel function.
 *
 * Example:
 *   VScriptScope root = VScriptScope.GetRoot();
 *   root.Run("function MyFunc(x, y) { return x + y; }");
 *   VScriptFunction func = root.LookupFunction("MyFunc");
 *   if (func) {
 *       ScriptVariant arg1 = ScriptVariant.FromInt(10);
 *       ScriptVariant arg2 = ScriptVariant.FromInt(20);
 *       ScriptVariant result = func.CallWithArgs(null, arg1, arg2);
 *       PrintToServer("Result: %d", result.Int);  // 30
 *       delete result;
 *       delete arg1;
 *       delete arg2;
 *       delete func;
 *   }
 *   delete root;  // Must delete to avoid handle leak
 */
methodmap VScriptFunction < VScript {
	/**
	 * Call the function with no arguments.
	 *
	 * @param scope     Execution scope (optional, use null for global)
	 * @return          ScriptVariant with return value (must be deleted)
	 * @note            Remember to delete the returned ScriptVariant
	 */
	public native any Call(VScript scope = null);

	/**
	 * Call the function with arguments.
	 *
	 * @param scope     Execution scope (use null for global)
	 * @param ...       ScriptVariant arguments (created with ScriptVariant.FromXxx)
	 * @return          ScriptVariant with return value (must be deleted)
	 * @note            Remember to delete the returned ScriptVariant
	 * @note            Arguments must be ScriptVariant handles
	 */
	public native any CallWithArgs(VScript scope, any ...);
}

// ============================================================================
// VScriptArray - Script array
// ============================================================================

/**
 * Represents a Squirrel array (indexed list).
 *
 * Example:
 *   VScriptArray arr = new VScriptArray();
 *   arr.PushInt(10);
 *   arr.PushInt(20);
 *   arr.PushString("hello");
 *
 *   PrintToServer("Length: %d", arr.Length);  // 3
 *   PrintToServer("arr[0] = %d", arr.GetInt(0));  // 10
 *
 *   delete arr;
 */
methodmap VScriptArray < VScript {
	/**
	 * Create a new array.
	 *
	 * @return          VScriptArray handle (must be deleted)
	 * @note            Remember to delete when done
	 */
	public native VScriptArray();

	/**
	 * Get the array length.
	 *
	 * @return          Number of elements in the array
	 */
	property int Length {
		public native get();
	}

	// Push methods - Add elements to the end of the array

	/**
	 * Push an integer value.
	 *
	 * @param value     Integer value to push
	 */
	public native void PushInt(int value);

	/**
	 * Push a float value.
	 *
	 * @param value     Float value to push
	 */
	public native void PushFloat(float value);

	/**
	 * Push a boolean value.
	 *
	 * @param value     Boolean value to push
	 */
	public native void PushBool(bool value);

	/**
	 * Push a string value.
	 *
	 * @param value     String value to push
	 */
	public native void PushString(const char[] value);

	/**
	 * Push a vector value.
	 *
	 * @param vec       Vector value to push (float[3])
	 */
	public native void PushVector(const float vec[3]);

	/**
	 * Push a ScriptVariant value.
	 *
	 * @param value     VScript handle to push
	 */
	public native void PushValue(ScriptVariant value);

	// Get methods - Retrieve elements by index

	/**
	 * Get an integer value at index.
	 *
	 * @param index     Array index (0-based)
	 * @param defValue  Default value if index invalid or wrong type
	 * @return          Integer value
	 */
	public native int GetInt(int index, int defValue = 0);

	/**
	 * Get a float value at index.
	 *
	 * @param index     Array index (0-based)
	 * @param defValue  Default value if index invalid or wrong type
	 * @return          Float value
	 */
	public native float GetFloat(int index, float defValue = 0.0);

	/**
	 * Get a boolean value at index.
	 *
	 * @param index     Array index (0-based)
	 * @param defValue  Default value if index invalid or wrong type
	 * @return          Boolean value
	 */
	public native bool GetBool(int index, bool defValue = false);

	/**
	 * Get a string value at index.
	 *
	 * @param index     Array index (0-based)
	 * @param buffer    Output buffer for string
	 * @param maxlen    Maximum buffer size
	 * @return          Number of characters written
	 */
	public native int GetString(int index, char[] buffer, int maxlen);

	/**
	 * Get a vector value at index.
	 *
	 * @param index     Array index (0-based)
	 * @param vec       Output vector (float[3])
	 * @return          True if successful
	 */
	public native bool GetVector(int index, float vec[3]);
	public native any GetValue(int index);

	// Set methods
	public native bool SetInt(int index, int value);
	public native bool SetFloat(int index, float value);
	public native bool SetBool(int index, bool value);
	public native bool SetString(int index, const char[] value);
	public native bool SetVector(int index, const float vec[3]);
	public native bool SetValue(int index, ScriptVariant value);

	/**
	 * Insert a value at the specified index.
	 * @param index     Index to insert at (0 to Length)
	 * @param value     ScriptVariant to insert
	 * @return          True if successful
	 */
	public native bool Insert(int index, ScriptVariant value);

	/**
	 * Remove the element at the specified index.
	 * @param index     Index to remove
	 * @return          True if successful
	 */
	public native bool Remove(int index);

	/**
	 * Remove and return the last element.
	 * @return          ScriptVariant with the last element (must be deleted), or null if empty
	 */
	public native ScriptVariant Pop();

	/**
	 * Clear all elements from the array.
	 * @return          True if successful
	 */
	public native bool Clear();

	/**
	 * Check if the array contains a value.
	 * @param value     ScriptVariant to search for
	 * @return          True if found
	 */
	public native bool Contains(ScriptVariant value);

	/**
	 * Find the index of a value.
	 * @param value     ScriptVariant to search for
	 * @return          Index of the value, or -1 if not found
	 */
	public native int IndexOf(ScriptVariant value);
}

// ============================================================================
// VScriptTable - Script table/dictionary
// ============================================================================

methodmap VScriptTable < VScript {
	/**
	 * Create a new table.
	 * @return          VScriptTable handle
	 */
	public native VScriptTable();

	/**
	 * Get the number of entries in the table.
	 */
	property int Length {
		public native get();
	}

	// Set by key
	public native bool SetInt(const char[] key, int value);
	public native bool SetFloat(const char[] key, float value);
	public native bool SetBool(const char[] key, bool value);
	public native bool SetString(const char[] key, const char[] value);
	public native bool SetVector(const char[] key, const float vec[3]);
	public native bool SetValue(const char[] key, ScriptVariant value);

	// Set by index
	public native bool SetIntAt(int index, int value);
	public native bool SetFloatAt(int index, float value);
	public native bool SetBoolAt(int index, bool value);
	public native bool SetStringAt(int index, const char[] value);
	public native bool SetVectorAt(int index, const float vec[3]);
	public native bool SetValueAt(int index, ScriptVariant value);

	// Get by key
	public native int GetInt(const char[] key, int defValue = 0);
	public native float GetFloat(const char[] key, float defValue = 0.0);
	public native bool GetBool(const char[] key, bool defValue = false);
	public native int GetString(const char[] key, char[] buffer, int maxlen);
	public native bool GetVector(const char[] key, float vec[3]);
	public native any GetValue(const char[] key);

	// Get by index
	public native int GetIntAt(int index, int defValue = 0);
	public native float GetFloatAt(int index, float defValue = 0.0);
	public native bool GetBoolAt(int index, bool defValue = false);
	public native int GetStringAt(int index, char[] buffer, int maxlen);
	public native bool GetVectorAt(int index, float vec[3]);
	public native any GetValueAt(int index);

	/**
	 * Iterate over key-value pairs.
	 * @param iterator      Iterator value (start at 0)
	 * @param keyBuffer     Output buffer for key
	 * @param keyMaxlen     Key buffer size
	 * @param value         Output ScriptVariant for value
	 * @return              Next iterator value, or -1 if iteration complete
	 */
	public native int GetKeyValue(int iterator, char[] keyBuffer, int keyMaxlen, any &value);

	public native bool HasKey(const char[] key);
	public native bool ClearKey(const char[] key);

	/**
	 * Clear all entries from the table.
	 * @return          True if successful
	 */
	public native bool Clear();

	/**
	 * Get all keys in the table as an array.
	 * @return          VScriptArray containing all keys (must be deleted)
	 */
	public native VScriptArray GetKeys();

	/**
	 * Create a shallow copy of the table.
	 * @return          VScriptTable with copied entries (must be deleted)
	 */
	public native VScriptTable Clone();

	/**
	 * Look up a function in this table.
	 * @param name      Function name
	 * @return          VScriptFunction handle, or null if not found
	 */
	public native VScriptFunction LookupFunction(const char[] name);
}

// ============================================================================
// VScriptScope - Script scope/namespace
// ============================================================================

methodmap VScriptScope < VScript {
	/**
	 * Create a new scope.
	 * @param name      Scope name
	 * @param parent    Parent scope (optional)
	 * @return          VScriptScope handle
	 */
	public native VScriptScope(const char[] name, VScriptScope parent = null);

	/**
	 * Get the root table (global scope).
	 * @return          VScriptScope handle
	 */
	public static native VScriptScope GetRoot();

	// Set value methods
	public native bool SetInt(const char[] key, int value);
	public native bool SetFloat(const char[] key, float value);
	public native bool SetBool(const char[] key, bool value);
	public native bool SetString(const char[] key, const char[] value);
	public native bool SetVector(const char[] key, const float vec[3]);
	public native bool SetValue(const char[] key, ScriptVariant value);
	public native bool SetTable(const char[] key, VScriptTable table);
	public native bool SetArray(const char[] key, VScriptArray arr);

	// Get value methods
	public native int GetInt(const char[] key, int defValue = 0);
	public native float GetFloat(const char[] key, float defValue = 0.0);
	public native bool GetBool(const char[] key, bool defValue = false);
	public native int GetString(const char[] key, char[] buffer, int maxlen);
	public native bool GetVector(const char[] key, float vec[3]);
	public native any GetValue(const char[] key);
	public native VScriptTable GetTable(const char[] key);
	public native VScriptArray GetArray(const char[] key);

	/**
	 * Check if a key exists.
	 * @param key       Key name
	 * @return          True if key exists
	 */
	public native bool HasKey(const char[] key);

	/**
	 * Clear/delete a key.
	 * @param key       Key name
	 * @return          True if successful
	 */
	public native bool ClearKey(const char[] key);

	/**
	 * Look up a Squirrel-defined function by name.
	 *
	 * IMPORTANT: This only works for functions defined in Squirrel scripts,
	 * NOT for C++ bound functions (like RandomInt, Director methods, etc).
	 * C++ bound functions can only be called via Execute() or Run().
	 *
	 * Example:
	 *   root.Run("function MyFunc(x) { return x * 2 }");
	 *   VScriptFunction func = root.LookupFunction("MyFunc");  // Works
	 *
	 *   VScriptFunction rand = root.LookupFunction("RandomInt");  // Returns null
	 *
	 * @param name      Function name
	 * @return          VScriptFunction handle, or null if not found
	 */
	public native VScriptFunction LookupFunction(const char[] name);

	/**
	 * Execute script code and return the result.
	 * @param format    Script code format string
	 * @param ...       Format arguments
	 * @return          ScriptVariant with result
	 */
	public native any Execute(const char[] format, any ...);
}

// ============================================================================
// ScriptVariant - Script variable value wrapper
// ============================================================================

methodmap ScriptVariant < VScript {
	/**
	 * Create a ScriptVariant from an integer value.
	 * @param value     Integer value
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromInt(int value);

	/**
	 * Create a ScriptVariant from a float value.
	 * @param value     Float value
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromFloat(float value);

	/**
	 * Create a ScriptVariant from a boolean value.
	 * @param value     Boolean value
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromBool(bool value);

	/**
	 * Create a ScriptVariant from a string value.
	 * @param value     String value
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromString(const char[] value);

	/**
	 * Create a ScriptVariant from a vector value.
	 * @param vec       Vector value
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromVector(const float vec[3]);

	/**
	 * Create a ScriptVariant from a table.
	 * @param table     VScriptTable handle
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromTable(VScriptTable table);

	/**
	 * Create a ScriptVariant from an array.
	 * @param arr       VScriptArray handle
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant FromArray(VScriptArray arr);

	/**
	 * Create a ScriptVariant from an entity index.
	 * Converts an entity index to its VScript instance representation.
	 * @param entity    Entity index
	 * @return          ScriptVariant handle containing the entity instance, or null if failed
	 */
	public static native ScriptVariant FromEntity(int entity);

	/**
	 * Create a null ScriptVariant.
	 * @return          ScriptVariant handle
	 */
	public static native ScriptVariant Null();

	/**
	 * Get the type of this variant.
	 */
	property ScriptFieldType Type {
		public native get();
	}

	/**
	 * Check if this variant is null.
	 */
	property bool IsNull {
		public native get();
	}

	/**
	 * Get the integer value.
	 */
	property int Int {
		public native get();
	}

	/**
	 * Get the float value.
	 */
	property float Float {
		public native get();
	}

	/**
	 * Get the boolean value.
	 */
	property bool Bool {
		public native get();
	}

	/**
	 * Get the string value.
	 * @param buffer    Output buffer
	 * @param maxlen    Buffer size
	 * @return          Number of characters written
	 */
	public native int GetString(char[] buffer, int maxlen);

	/**
	 * Get the vector value.
	 * @param vec       Output vector
	 * @return          True if successful
	 */
	public native bool GetVector(float vec[3]);

	/**
	 * Get as VScriptTable (if type is FIELD_HSCRIPT and value is a table).
	 */
	property VScriptTable Table {
		public native get();
	}

	/**
	 * Get as VScriptArray (if type is FIELD_HSCRIPT and value is an array).
	 */
	property VScriptArray Array {
		public native get();
	}

	/**
	 * Get as VScriptScope (if type is FIELD_HSCRIPT and value is a scope).
	 */
	property VScriptScope Scope {
		public native get();
	}

	/**
	 * Get as VScriptFunction (if type is FIELD_HSCRIPT and value is a function).
	 */
	property VScriptFunction Function {
		public native get();
	}

	/**
	 * Convert this variant to an entity index.
	 * Only works if this variant contains an entity instance (HSCRIPT type).
	 * @return          Entity index, or -1 if conversion failed
	 */
	public native int ToEntity();
}

// ============================================================================
// Forwards
// ============================================================================

/**
 * Called when the VScript VM is created.
 * This occurs on map load or when the VM is initialized.
 * @param vm    IScriptVM pointer
 */
forward void OnVScriptVMCreate(Address vm);

/**
 * Called when the VScript VM is about to be destroyed.
 * This occurs on map change or when the VM is shut down.
 * All VScript handles will be automatically freed after this forward.
 * @param vm    IScriptVM pointer
 */
forward void OnVScriptVMDestroy(Address vm);

// ============================================================================
// Optional Native Markers
// ============================================================================

public void __ext_vscriptfun_SetNTVOptional() {
	// VScript
	MarkNativeAsOptional("VScript.IsValid.get");
	MarkNativeAsOptional("VScript.IsArray.get");
	MarkNativeAsOptional("VScript.IsTable.get");
	MarkNativeAsOptional("VScript.Compile");
	MarkNativeAsOptional("VScript.Run");

	// VScriptFunction
	MarkNativeAsOptional("VScriptFunction.Call");
	MarkNativeAsOptional("VScriptFunction.CallWithArgs");

	// VScriptArray
	MarkNativeAsOptional("VScriptArray.VScriptArray");
	MarkNativeAsOptional("VScriptArray.Length.get");
	MarkNativeAsOptional("VScriptArray.PushInt");
	MarkNativeAsOptional("VScriptArray.PushFloat");
	MarkNativeAsOptional("VScriptArray.PushBool");
	MarkNativeAsOptional("VScriptArray.PushString");
	MarkNativeAsOptional("VScriptArray.PushVector");
	MarkNativeAsOptional("VScriptArray.PushValue");
	MarkNativeAsOptional("VScriptArray.GetInt");
	MarkNativeAsOptional("VScriptArray.GetFloat");
	MarkNativeAsOptional("VScriptArray.GetBool");
	MarkNativeAsOptional("VScriptArray.GetString");
	MarkNativeAsOptional("VScriptArray.GetVector");
	MarkNativeAsOptional("VScriptArray.GetValue");
	MarkNativeAsOptional("VScriptArray.SetInt");
	MarkNativeAsOptional("VScriptArray.SetFloat");
	MarkNativeAsOptional("VScriptArray.SetBool");
	MarkNativeAsOptional("VScriptArray.SetString");
	MarkNativeAsOptional("VScriptArray.SetVector");
	MarkNativeAsOptional("VScriptArray.SetValue");
	MarkNativeAsOptional("VScriptArray.Insert");
	MarkNativeAsOptional("VScriptArray.Remove");
	MarkNativeAsOptional("VScriptArray.Pop");
	MarkNativeAsOptional("VScriptArray.Clear");
	MarkNativeAsOptional("VScriptArray.Contains");
	MarkNativeAsOptional("VScriptArray.IndexOf");

	// VScriptTable
	MarkNativeAsOptional("VScriptTable.VScriptTable");
	MarkNativeAsOptional("VScriptTable.Length.get");
	MarkNativeAsOptional("VScriptTable.SetInt");
	MarkNativeAsOptional("VScriptTable.SetFloat");
	MarkNativeAsOptional("VScriptTable.SetBool");
	MarkNativeAsOptional("VScriptTable.SetString");
	MarkNativeAsOptional("VScriptTable.SetVector");
	MarkNativeAsOptional("VScriptTable.SetValue");
	MarkNativeAsOptional("VScriptTable.SetIntAt");
	MarkNativeAsOptional("VScriptTable.SetFloatAt");
	MarkNativeAsOptional("VScriptTable.SetBoolAt");
	MarkNativeAsOptional("VScriptTable.SetStringAt");
	MarkNativeAsOptional("VScriptTable.SetVectorAt");
	MarkNativeAsOptional("VScriptTable.SetValueAt");
	MarkNativeAsOptional("VScriptTable.GetInt");
	MarkNativeAsOptional("VScriptTable.GetFloat");
	MarkNativeAsOptional("VScriptTable.GetBool");
	MarkNativeAsOptional("VScriptTable.GetString");
	MarkNativeAsOptional("VScriptTable.GetVector");
	MarkNativeAsOptional("VScriptTable.GetValue");
	MarkNativeAsOptional("VScriptTable.GetIntAt");
	MarkNativeAsOptional("VScriptTable.GetFloatAt");
	MarkNativeAsOptional("VScriptTable.GetBoolAt");
	MarkNativeAsOptional("VScriptTable.GetStringAt");
	MarkNativeAsOptional("VScriptTable.GetVectorAt");
	MarkNativeAsOptional("VScriptTable.GetValueAt");
	MarkNativeAsOptional("VScriptTable.GetKeyValue");
	MarkNativeAsOptional("VScriptTable.HasKey");
	MarkNativeAsOptional("VScriptTable.ClearKey");
	MarkNativeAsOptional("VScriptTable.LookupFunction");
	MarkNativeAsOptional("VScriptTable.Clear");
	MarkNativeAsOptional("VScriptTable.GetKeys");
	MarkNativeAsOptional("VScriptTable.Clone");

	// VScriptScope
	MarkNativeAsOptional("VScriptScope.VScriptScope");
	MarkNativeAsOptional("VScriptScope.GetRoot");
	MarkNativeAsOptional("VScriptScope.SetInt");
	MarkNativeAsOptional("VScriptScope.SetFloat");
	MarkNativeAsOptional("VScriptScope.SetBool");
	MarkNativeAsOptional("VScriptScope.SetString");
	MarkNativeAsOptional("VScriptScope.SetVector");
	MarkNativeAsOptional("VScriptScope.SetValue");
	MarkNativeAsOptional("VScriptScope.SetTable");
	MarkNativeAsOptional("VScriptScope.SetArray");
	MarkNativeAsOptional("VScriptScope.GetInt");
	MarkNativeAsOptional("VScriptScope.GetFloat");
	MarkNativeAsOptional("VScriptScope.GetBool");
	MarkNativeAsOptional("VScriptScope.GetString");
	MarkNativeAsOptional("VScriptScope.GetVector");
	MarkNativeAsOptional("VScriptScope.GetValue");
	MarkNativeAsOptional("VScriptScope.GetTable");
	MarkNativeAsOptional("VScriptScope.GetArray");
	MarkNativeAsOptional("VScriptScope.HasKey");
	MarkNativeAsOptional("VScriptScope.ClearKey");
	MarkNativeAsOptional("VScriptScope.LookupFunction");
	MarkNativeAsOptional("VScriptScope.Execute");

	// ScriptVariant
	MarkNativeAsOptional("ScriptVariant.FromInt");
	MarkNativeAsOptional("ScriptVariant.FromFloat");
	MarkNativeAsOptional("ScriptVariant.FromBool");
	MarkNativeAsOptional("ScriptVariant.FromString");
	MarkNativeAsOptional("ScriptVariant.FromVector");
	MarkNativeAsOptional("ScriptVariant.FromTable");
	MarkNativeAsOptional("ScriptVariant.FromArray");
	MarkNativeAsOptional("ScriptVariant.FromEntity");
	MarkNativeAsOptional("ScriptVariant.Null");
	MarkNativeAsOptional("ScriptVariant.Type.get");
	MarkNativeAsOptional("ScriptVariant.IsNull.get");
	MarkNativeAsOptional("ScriptVariant.Int.get");
	MarkNativeAsOptional("ScriptVariant.Float.get");
	MarkNativeAsOptional("ScriptVariant.Bool.get");
	MarkNativeAsOptional("ScriptVariant.GetString");
	MarkNativeAsOptional("ScriptVariant.GetVector");
	MarkNativeAsOptional("ScriptVariant.Table.get");
	MarkNativeAsOptional("ScriptVariant.Array.get");
	MarkNativeAsOptional("ScriptVariant.Scope.get");
	MarkNativeAsOptional("ScriptVariant.Function.get");
	MarkNativeAsOptional("ScriptVariant.ToEntity");
}

public Extension __ext_vscript = {
	name = "VScript",
	file = "vscript.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};