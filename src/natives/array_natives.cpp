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

#include "extension.h"
#include "core/handle_types.h"
#include "core/vscript_manager.h"
#include "core/generic_operations.h"
#include "core/variant_helpers.h"
#include <vscript/ivscript.h>

static cell_t Native_VScriptArray_Create(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	ScriptVariant_t arrVar;
	vm->CreateArray(arrVar);

	HSCRIPT hScript = arrVar;
	if (arrVar.GetType() != FIELD_HSCRIPT || !hScript || hScript == INVALID_HSCRIPT) return 0;

	VScriptArrayHandle* handle = new VScriptArrayHandle(hScript, false);
	handle->SetVariant(arrVar);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptArray_Length_get(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;
	return vm->GetArrayCount(arr);
}

static cell_t Native_VScriptArray_PushInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = (int)params[2];
	vm->ArrayAddToTail(arr, variant);
	return 1;
}

static cell_t Native_VScriptArray_PushFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = sp_ctof(params[2]);
	vm->ArrayAddToTail(arr, variant);
	return 1;
}

static cell_t Native_VScriptArray_PushBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = (params[2] != 0);
	vm->ArrayAddToTail(arr, variant);
	return 1;
}

static cell_t Native_VScriptArray_PushString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	char* str;
	ctx->LocalToString(params[2], &str);

	ScriptVariant_t variant;
	variant = str;
	vm->ArrayAddToTail(arr, variant);
	return 1;
}

static cell_t Native_VScriptArray_PushVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	Vector v = ReadVectorParam(ctx, params, 2);
	ScriptVariant_t variant = CreateVectorVariant(v);
	vm->ArrayAddToTail(arr, variant);
	return 1;
}

static cell_t Native_VScriptArray_PushValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	VScriptVariantHandle* varHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[2]);
	if (!varHandle) return 0;

	vm->ArrayAddToTail(arr, varHandle->GetVariant());
	return 1;
}

static cell_t Native_VScriptArray_GetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_INTEGER) result = (int)variant;
	else if (variant.GetType() == FIELD_FLOAT) result = (cell_t)((float)variant);

	return result;
}

static cell_t Native_VScriptArray_GetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_FLOAT) result = sp_ftoc((float)variant);
	else if (variant.GetType() == FIELD_INTEGER) result = sp_ftoc((float)((int)variant));

	return result;
}

static cell_t Native_VScriptArray_GetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_BOOLEAN) result = (bool)variant;

	return result;
}

static cell_t Native_VScriptArray_GetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return 0;
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	const char *pszString = variant.m_pszString;
	if (variant.GetType() == FIELD_CSTRING && pszString) {
		ctx->StringToLocalUTF8(params[3], params[4], pszString, nullptr);
		result = strlen(pszString);
	}

	return result;
}

static cell_t Native_VScriptArray_GetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return 0;
	AutoReleaseVariant autoRelease(vm, variant);

	const Vector &pVector = variant;
	if (variant.GetType() == FIELD_VECTOR && WriteVectorResult(ctx, params, 3, pVector)) {
		return 1;
	}

	return 0;
}

static cell_t Native_VScriptArray_GetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(arr, params[2], &variant)) return 0;

	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_VScriptArray_SetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = params[3];
	return vm->SetValue(arr, params[2], variant);
}

static cell_t Native_VScriptArray_SetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = sp_ctof(params[3]);
	return vm->SetValue(arr, params[2], variant);
}

static cell_t Native_VScriptArray_SetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	VScriptVariantHandle* varHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[3]);
	if (!varHandle) return 0;

	return vm->SetValue(arr, params[2], varHandle->GetVariant());
}

static cell_t Native_VScriptArray_SetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	ScriptVariant_t variant;
	variant = (params[3] != 0);
	return vm->SetValue(arr, params[2], variant);
}

static cell_t Native_VScriptArray_SetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	char* str;
	ctx->LocalToString(params[3], &str);

	ScriptVariant_t variant;
	variant = str;
	return vm->SetValue(arr, params[2], variant);
}

static cell_t Native_VScriptArray_SetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	Vector v = ReadVectorParam(ctx, params, 3);
	ScriptVariant_t variant = CreateVectorVariant(v);
	return vm->SetValue(arr, params[2], variant);
}

static cell_t Native_VScriptArray_Remove(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	int index = params[2];
	int length = vm->GetArrayCount(arr);

	if (index < 0 || index >= length) return 0;

	// Use VScript to remove element: array.remove(index)
	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) return 0;

	char code[128];
	snprintf(code, sizeof(code), "__sm_temp_array__.remove(%d)", index);

	// Set array as '__sm_temp_array__' in root temporarily
	ScriptVariant_t arrVar;
	arrVar = arr;
	vm->SetValue(root, "__sm_temp_array__", arrVar);

	HSCRIPT compiled = vm->CompileScript(code, "ArrayRemove");
	if (compiled && compiled != INVALID_HSCRIPT) {
		vm->ExecuteFunction(compiled, nullptr, 0, nullptr, root, true);
		vm->ReleaseScript(compiled);
	}

	// Clean up temporary variable
	vm->ClearValue(root, "__sm_temp_array__");
	return 1;
}

static cell_t Native_VScriptArray_Pop(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	int length = vm->GetArrayCount(arr);
	if (length == 0) return 0;

	// Get last element
	ScriptVariant_t value;
	if (!vm->GetValue(arr, length - 1, &value)) return 0;

	// Remove last element
	HSCRIPT root = vm->GetRootTable();
	if (root && root != INVALID_HSCRIPT) {
		char code[128];
		snprintf(code, sizeof(code), "__sm_temp_array__.remove(%d)", length - 1);

		ScriptVariant_t arrVar;
		arrVar = arr;
		vm->SetValue(root, "__sm_temp_array__", arrVar);

		HSCRIPT compiled = vm->CompileScript(code, "ArrayPop");
		if (compiled && compiled != INVALID_HSCRIPT) {
			vm->ExecuteFunction(compiled, nullptr, 0, nullptr, root, true);
			vm->ReleaseScript(compiled);
		}

		vm->ClearValue(root, "__sm_temp_array__");
	}

	return CreateVariantHandleFromScriptVariant(ctx, value);
}

static cell_t Native_VScriptArray_Clear(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	// Use VScript to clear: array.clear()
	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) return 0;

	ScriptVariant_t arrVar;
	arrVar = arr;
	vm->SetValue(root, "__sm_temp_array__", arrVar);

	HSCRIPT compiled = vm->CompileScript("__sm_temp_array__.clear()", "ArrayClear");
	if (compiled && compiled != INVALID_HSCRIPT) {
		vm->ExecuteFunction(compiled, nullptr, 0, nullptr, root, true);
		vm->ReleaseScript(compiled);
	}

	vm->ClearValue(root, "__sm_temp_array__");
	return 1;
}

static cell_t Native_VScriptArray_Contains(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	VScriptVariantHandle* searchHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[2]);
	if (!searchHandle) return 0;

	int length = vm->GetArrayCount(arr);
	for (int i = 0; i < length; i++) {
		ScriptVariant_t value;
		if (!vm->GetValue(arr, i, &value)) continue;
		AutoReleaseVariant autoRelease(vm, value);

		if (CompareScriptVariants(value, searchHandle->GetVariant())) {
			return 1;
		}
	}

	return 0;
}

static cell_t Native_VScriptArray_IndexOf(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return -1;

	VScriptVariantHandle* searchHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[2]);
	if (!searchHandle) return -1;

	int length = vm->GetArrayCount(arr);
	for (int i = 0; i < length; i++) {
		ScriptVariant_t value;
		if (!vm->GetValue(arr, i, &value)) continue;
		AutoReleaseVariant autoRelease(vm, value);

		if (CompareScriptVariants(value, searchHandle->GetVariant())) {
			return i;
		}
	}

	return -1;
}

static cell_t Native_VScriptArray_Insert(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT arr;
	if (!GetVMAndHScript<VScriptArrayHandle>(ctx, params[1], vm, arr)) return 0;

	int index = params[2];
	int length = vm->GetArrayCount(arr);

	if (index < 0 || index > length) return 0;

	VScriptVariantHandle* valueHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[3]);
	if (!valueHandle) return 0;

	// Use VScript to insert: array.insert(index, value)
	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) return 0;

	// Set array and value in root temporarily
	ScriptVariant_t arrVar;
	arrVar = arr;
	vm->SetValue(root, "__sm_temp_array__", arrVar);
	vm->SetValue(root, "__sm_temp_value__", valueHandle->GetVariant());

	char code[128];
	snprintf(code, sizeof(code), "__sm_temp_array__.insert(%d, __sm_temp_value__)", index);

	HSCRIPT compiled = vm->CompileScript(code, "ArrayInsert");
	if (compiled && compiled != INVALID_HSCRIPT) {
		vm->ExecuteFunction(compiled, nullptr, 0, nullptr, root, true);
		vm->ReleaseScript(compiled);
	}

	// Clean up temporary variables
	vm->ClearValue(root, "__sm_temp_array__");
	vm->ClearValue(root, "__sm_temp_value__");
	return 1;
}

const sp_nativeinfo_t g_ArrayNatives[] = {
	{"VScriptArray.VScriptArray",    Native_VScriptArray_Create},
	{"VScriptArray.Length.get",     Native_VScriptArray_Length_get},
	{"VScriptArray.PushInt",        Native_VScriptArray_PushInt},
	{"VScriptArray.PushFloat",      Native_VScriptArray_PushFloat},
	{"VScriptArray.PushBool",       Native_VScriptArray_PushBool},
	{"VScriptArray.PushString",     Native_VScriptArray_PushString},
	{"VScriptArray.PushVector",     Native_VScriptArray_PushVector},
	{"VScriptArray.PushValue",      Native_VScriptArray_PushValue},
	{"VScriptArray.GetInt",         Native_VScriptArray_GetInt},
	{"VScriptArray.GetFloat",       Native_VScriptArray_GetFloat},
	{"VScriptArray.GetBool",        Native_VScriptArray_GetBool},
	{"VScriptArray.GetString",      Native_VScriptArray_GetString},
	{"VScriptArray.GetVector",      Native_VScriptArray_GetVector},
	{"VScriptArray.GetValue",       Native_VScriptArray_GetValue},
	{"VScriptArray.SetInt",         Native_VScriptArray_SetInt},
	{"VScriptArray.SetFloat",       Native_VScriptArray_SetFloat},
	{"VScriptArray.SetBool",        Native_VScriptArray_SetBool},
	{"VScriptArray.SetString",      Native_VScriptArray_SetString},
	{"VScriptArray.SetVector",      Native_VScriptArray_SetVector},
	{"VScriptArray.SetValue",       Native_VScriptArray_SetValue},
	{"VScriptArray.Insert",         Native_VScriptArray_Insert},
	{"VScriptArray.Remove",         Native_VScriptArray_Remove},
	{"VScriptArray.Pop",            Native_VScriptArray_Pop},
	{"VScriptArray.Clear",          Native_VScriptArray_Clear},
	{"VScriptArray.Contains",       Native_VScriptArray_Contains},
	{"VScriptArray.IndexOf",        Native_VScriptArray_IndexOf},
	{nullptr,                       nullptr}
};
