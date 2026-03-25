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
#include "mathlib/mathlib.h"
#include <vscript/ivscript.h>

static cell_t Native_VScriptTable_Create(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	ScriptVariant_t tableVar;
	vm->CreateTable(tableVar);

	HSCRIPT table = tableVar.Get<HSCRIPT>();
	if (tableVar.GetType() != FIELD_HSCRIPT || !table || table == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to create table HSCRIPT Instance. table: %d, type: %d", table, tableVar.GetType());
		return 0;
	}


	VScriptTableHandle* handle = new VScriptTableHandle(table, false);
	handle->SetVariant(tableVar);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptTable_Length_get(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;
	return vm->GetNumTableEntries(table);
}

static cell_t Native_VScriptTable_SetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->SetValue(table, key, params[3]);
}

static cell_t Native_VScriptTable_SetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->SetValue(table, key, sp_ctof(params[3]));
}

static cell_t Native_VScriptTable_SetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->SetValue(table, key, params[3] != 0);
}

static cell_t Native_VScriptTable_SetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	char* value;
	ctx->LocalToString(params[3], &value);
	return vm->SetValue(table, key, value);
}

static cell_t Native_VScriptTable_SetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	Vector v = ReadVectorParam(ctx, params, 3);
	return vm->SetValue(table, key, CreateVectorVariant(v));
}

static cell_t Native_VScriptTable_SetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	VScriptVariantHandle* varHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[3]);
	if (!varHandle) return 0;

	return vm->SetValue(table, key, varHandle->GetVariant());
}

static cell_t Native_VScriptTable_SetIntAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	variant = (int)params[3];
	return vm->SetValue(table, params[2], variant);
}

static cell_t Native_VScriptTable_SetFloatAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	variant = sp_ctof(params[3]);
	return vm->SetValue(table, params[2], variant);
}

static cell_t Native_VScriptTable_SetValueAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	VScriptVariantHandle* varHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[3]);
	if (!varHandle) return 0;

	return vm->SetValue(table, params[2], varHandle->GetVariant());
}

static cell_t Native_VScriptTable_SetBoolAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	variant = (params[3] != 0);
	return vm->SetValue(table, params[2], variant);
}

static cell_t Native_VScriptTable_SetStringAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* str;
	ctx->LocalToString(params[3], &str);

	ScriptVariant_t variant;
	variant = ScriptVariant_t(str, false);
	return vm->SetValue(table, params[2], variant);
}

static cell_t Native_VScriptTable_SetVectorAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	Vector v = ReadVectorParam(ctx, params, 3);
	return vm->SetValue(table, params[2], CreateVectorVariant(v));
}

static cell_t Native_VScriptTable_GetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_INTEGER) result = (cell_t)variant.Get<int>();
	else if (variant.GetType() == FIELD_FLOAT) result = (cell_t)(variant.Get<float>());

	return result;
}

static cell_t Native_VScriptTable_GetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	if (variant.GetType() == FIELD_FLOAT) result = sp_ftoc(variant.Get<float>());
	else if (variant.GetType() == FIELD_INTEGER) result = sp_ftoc((float)(variant.Get<int>()));

	return result;
}

static cell_t Native_VScriptTable_GetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_BOOLEAN) result = (cell_t)variant.Get<bool>();

	return result;
}

static cell_t Native_VScriptTable_GetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) {
		ctx->ThrowNativeError("Failed to get string from table by key: %s.", key);
		return 0;
	}

	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	char* str = nullptr;
	variant.AssignTo(str);
	if (variant.GetType() == FIELD_CSTRING && str) {
		ctx->StringToLocalUTF8(params[3], params[4], str, nullptr);
		result = strlen(str);
	}

	return result;
}

static cell_t Native_VScriptTable_GetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) {
		ctx->ThrowNativeError("Failed to get vector from table by key: %s.", key);
		return 0;
	}

	AutoReleaseVariant autoRelease(vm, variant);

	Vector vec(0, 0, 0);
	variant.AssignTo(&vec);
	if (variant.GetType() == FIELD_VECTOR && WriteVectorResult(ctx, params, 3, &vec)) {
		return 1;
	}

	return 0;
}

static cell_t Native_VScriptTable_GetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(table, key, &variant)) {
		ctx->ThrowNativeError("Failed to get value from table by key: %s.", key);
		return 0;
	}

	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_VScriptTable_GetIntAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_INTEGER) result = (cell_t)variant.Get<int>();
	else if (variant.GetType() == FIELD_FLOAT) result = (cell_t)(variant.Get<float>());

	return result;
}

static cell_t Native_VScriptTable_GetFloatAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_FLOAT) result = sp_ftoc(variant.Get<float>());
	else if (variant.GetType() == FIELD_INTEGER) result = sp_ftoc((float)(variant.Get<int>()));

	return result;
}

static cell_t Native_VScriptTable_GetValueAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) {
		ctx->ThrowNativeError("Failed to get value from table by index: %d.", params[2]);
		return 0;
	}

	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_VScriptTable_GetBoolAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return params[3];

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) return params[3];
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = params[3];
	if (variant.GetType() == FIELD_BOOLEAN) result = (cell_t)variant.Get<bool>();

	return result;
}

static cell_t Native_VScriptTable_GetStringAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) {
		ctx->ThrowNativeError("Failed to get string from table by index: %d.", params[2]);
		return 0;
	}

	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	char *str = nullptr;
	variant.AssignTo(str);
	if (variant.GetType() == FIELD_CSTRING && str) {
		ctx->StringToLocalUTF8(params[3], params[4], str, nullptr);
		result = strlen(str);
	}

	return result;
}

static cell_t Native_VScriptTable_GetVectorAt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	ScriptVariant_t variant;
	if (!vm->GetValue(table, params[2], &variant)) {
		ctx->ThrowNativeError("Failed to get vector from table by index: %d.", params[2]);
		return 0;
	}

	AutoReleaseVariant autoRelease(vm, variant);

	Vector vec(0, 0, 0);
	variant.AssignTo(&vec);
	if (variant.GetType() == FIELD_VECTOR && WriteVectorResult(ctx, params, 3, &vec)) {
		return 1;
	}

	return 0;
}

static cell_t Native_VScriptTable_GetKeyValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return -1;

	int iterator = params[2];

	// Loop to skip non-string keys
	while (true) {
		ScriptVariant_t keyVar, valueVar;
		iterator = vm->GetKeyValue(table, iterator, &keyVar, &valueVar);

		// Iteration complete
		if (iterator == -1) return -1;

		// Skip non-string keys (internal metadata)
		char *key = nullptr;
		keyVar.AssignTo(key);
		if (keyVar.GetType() != FIELD_CSTRING || !key) {
			// Release variants before continuing to avoid memory leak
			if (keyVar.GetFlags() & SV_FREE) vm->ReleaseValue(keyVar);
			if (valueVar.GetFlags() & SV_FREE) vm->ReleaseValue(valueVar);
			continue;
		}

		// Copy key to output buffer
		ctx->StringToLocalUTF8(params[3], params[4], key, nullptr);

		// Release key variant after copying string
		if (keyVar.GetFlags() & SV_FREE) vm->ReleaseValue(keyVar);

		// Create handle for value (transfers ownership)
		Handle_t valueHandle = CreateVariantHandleFromScriptVariant(ctx, valueVar);
		cell_t* outValue;
		ctx->LocalToPhysAddr(params[5], &outValue);
		*outValue = valueHandle;

		return iterator;
	}
}

static cell_t Native_VScriptTable_HasKey(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->ValueExists(table, key);
}

static cell_t Native_VScriptTable_ClearKey(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->ClearValue(table, key);
}

static cell_t Native_VScriptTable_LookupFunction(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	char* name;
	ctx->LocalToString(params[2], &name);

	HSCRIPT func = vm->LookupFunction(name, table);
	if (!func || func == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to lookup function by name: %s.", name);
		return 0;
	}

	VScriptFunctionHandle* handle = new VScriptFunctionHandle(func, true, false);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptTable_Clear(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	// Use VScript to clear: table.clear()
	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to get root table.");
		return 0;
	}
	ScriptVariant_t tableVar;
	tableVar = table;
	vm->SetValue(root, "__sm_temp_table__", tableVar);

	HSCRIPT compiled = vm->CompileScript("__sm_temp_table__.clear()", "TableClear");
	if (compiled && compiled != INVALID_HSCRIPT) {
		vm->ExecuteFunction(compiled, nullptr, 0, nullptr, root, true);
		vm->ReleaseScript(compiled);
	}

	vm->ClearValue(root, "__sm_temp_table__");
	return 1;
}

static cell_t Native_VScriptTable_GetKeys(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	// Create new array to hold keys
	ScriptVariant_t arrVar;
	vm->CreateArray(arrVar);
	HSCRIPT arr = arrVar.Get<HSCRIPT>();
	if (arrVar.GetType() != FIELD_HSCRIPT || !arr || arr == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to create array HSCRIPT Instance. arr: %d, type: %d", arr, arrVar.GetType());
		return 0;
	}

	// Iterate through table and collect keys
	int iterator = 0;  // SDK uses 0 as start
	while (true) {
		ScriptVariant_t keyVar, valueVar;
		iterator = vm->GetKeyValue(table, iterator, &keyVar, &valueVar);

		if (iterator == -1) break;

		// Skip non-string keys (internal metadata)
		char *key = nullptr;
		keyVar.AssignTo(key);
		if (keyVar.GetType() == FIELD_CSTRING && key) {
			// Add key to array
			vm->ArrayAddToTail(arr, keyVar);
		}

		// Release variants
		if (keyVar.GetFlags() & SV_FREE) vm->ReleaseValue(keyVar);
		if (valueVar.GetFlags() & SV_FREE) vm->ReleaseValue(valueVar);
	}

	// Create handle for the array
	VScriptArrayHandle* handle = new VScriptArrayHandle(arr, false);
	handle->SetVariant(arrVar);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptTable_Clone(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT table;
	if (!GetVMAndHScript<VScriptTableHandle>(ctx, params[1], vm, table)) return 0;

	// Create new table
	ScriptVariant_t newTableVar;
	vm->CreateTable(newTableVar);
	HSCRIPT newTable = newTableVar.Get<HSCRIPT>();
	if (newTableVar.GetType() != FIELD_HSCRIPT || !newTable || newTable == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to create new table HSCRIPT Instance. newTable: %d, type: %d", newTable, newTableVar.GetType());
		return 0;
	}

	// Iterate through source table and copy all key-value pairs
	int iterator = 0;  // SDK uses 0 as start
	while (true) {
		ScriptVariant_t keyVar, valueVar;
		iterator = vm->GetKeyValue(table, iterator, &keyVar, &valueVar);

		if (iterator == -1) break;

		// Copy key-value pair to new table
		char *key = nullptr;
		keyVar.AssignTo(key);
		if (keyVar.GetType() == FIELD_CSTRING && key) {
			vm->SetValue(newTable, key, valueVar);
		} else if (keyVar.GetType() == FIELD_INTEGER) {
			vm->SetValue(newTable, (int)keyVar, valueVar);
		}

		// Release variants
		if (keyVar.GetFlags() & SV_FREE) vm->ReleaseValue(keyVar);
		if (valueVar.GetFlags() & SV_FREE) vm->ReleaseValue(valueVar);
	}

	// Create handle for the new table
	VScriptTableHandle* handle = new VScriptTableHandle(newTable, false);
	handle->SetVariant(newTableVar);
	return CreateVScriptHandle(ctx, handle);
}

const sp_nativeinfo_t g_TableNatives[] = {
	{"VScriptTable.VScriptTable",    Native_VScriptTable_Create},
	{"VScriptTable.Length.get",     Native_VScriptTable_Length_get},
	{"VScriptTable.SetInt",         Native_VScriptTable_SetInt},
	{"VScriptTable.SetFloat",       Native_VScriptTable_SetFloat},
	{"VScriptTable.SetBool",        Native_VScriptTable_SetBool},
	{"VScriptTable.SetString",      Native_VScriptTable_SetString},
	{"VScriptTable.SetVector",      Native_VScriptTable_SetVector},
	{"VScriptTable.SetValue",       Native_VScriptTable_SetValue},
	{"VScriptTable.SetIntAt",       Native_VScriptTable_SetIntAt},
	{"VScriptTable.SetFloatAt",     Native_VScriptTable_SetFloatAt},
	{"VScriptTable.SetBoolAt",      Native_VScriptTable_SetBoolAt},
	{"VScriptTable.SetStringAt",    Native_VScriptTable_SetStringAt},
	{"VScriptTable.SetVectorAt",    Native_VScriptTable_SetVectorAt},
	{"VScriptTable.SetValueAt",     Native_VScriptTable_SetValueAt},
	{"VScriptTable.GetInt",         Native_VScriptTable_GetInt},
	{"VScriptTable.GetFloat",       Native_VScriptTable_GetFloat},
	{"VScriptTable.GetBool",        Native_VScriptTable_GetBool},
	{"VScriptTable.GetString",      Native_VScriptTable_GetString},
	{"VScriptTable.GetVector",      Native_VScriptTable_GetVector},
	{"VScriptTable.GetValue",       Native_VScriptTable_GetValue},
	{"VScriptTable.GetIntAt",       Native_VScriptTable_GetIntAt},
	{"VScriptTable.GetFloatAt",     Native_VScriptTable_GetFloatAt},
	{"VScriptTable.GetBoolAt",      Native_VScriptTable_GetBoolAt},
	{"VScriptTable.GetStringAt",    Native_VScriptTable_GetStringAt},
	{"VScriptTable.GetVectorAt",    Native_VScriptTable_GetVectorAt},
	{"VScriptTable.GetValueAt",     Native_VScriptTable_GetValueAt},
	{"VScriptTable.GetKeyValue",    Native_VScriptTable_GetKeyValue},
	{"VScriptTable.HasKey",         Native_VScriptTable_HasKey},
	{"VScriptTable.ClearKey",       Native_VScriptTable_ClearKey},
	{"VScriptTable.LookupFunction", Native_VScriptTable_LookupFunction},
	{"VScriptTable.Clear",          Native_VScriptTable_Clear},
	{"VScriptTable.GetKeys",        Native_VScriptTable_GetKeys},
	{"VScriptTable.Clone",          Native_VScriptTable_Clone},
	{nullptr,                       nullptr}
};
