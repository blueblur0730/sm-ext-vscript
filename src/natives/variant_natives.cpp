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
#include "core/variant_helpers.h"
#include "mathlib/mathlib.h"
#include <vscript/ivscript.h>

static cell_t Native_ScriptVariant_FromInt(IPluginContext* ctx, const cell_t* params) {
	ScriptVariant_t variant;
	variant = (int)params[1];
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_FromFloat(IPluginContext* ctx, const cell_t* params) {
	ScriptVariant_t variant;
	variant = sp_ctof(params[1]);
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_FromBool(IPluginContext* ctx, const cell_t* params) {
	ScriptVariant_t variant;
	variant = (params[1] != 0);
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_FromString(IPluginContext* ctx, const cell_t* params) {
	char* str;
	ctx->LocalToString(params[1], &str);
	IScriptVM* vm = g_VScriptManager.GetVM();

	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	// Create temporary table to force VM to copy and manage the string
	ScriptVariant_t tableVar;
	vm->CreateTable(tableVar);
	HSCRIPT hTable = tableVar.Get<HSCRIPT>();
	if (tableVar.GetType() != FIELD_HSCRIPT || !hTable || hTable == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to create table HSCRIPT Instance, table: %d, type: %d", hTable, tableVar.GetType());
		return 0;
	}

	// Store string in table (VM copies it using its own allocator)
	ScriptVariant_t tempVar;
	tempVar = ScriptVariant_t(str, false);
	tempVar.SetFlags(0);
	vm->SetValue(hTable, 0, tempVar);

	// Retrieve VM-owned copy
	ScriptVariant_t vmVar;
	vm->GetValue(hTable, 0, &vmVar);
	vm->ReleaseValue(tableVar);

	return CreateVariantHandleFromScriptVariant(ctx, vmVar);
}

static cell_t Native_ScriptVariant_FromVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();

	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	// Create temporary table to force VM to copy and manage the vector
	ScriptVariant_t tableVar;
	vm->CreateTable(tableVar);
	HSCRIPT hTable = tableVar.Get<HSCRIPT>();
	if (tableVar.GetType() != FIELD_HSCRIPT || !hTable || hTable == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to create table HSCRIPT Instance. hTable: %d, type: %d.", hTable, tableVar.GetType());
		return 0;
	}

	// Store vector in table (VM copies it using its own allocator)
	Vector v = ReadVectorParam(ctx, params, 1);
	vm->SetValue(hTable, 0, CreateVectorVariant(v));

	// Retrieve VM-owned copy
	ScriptVariant_t vmVar;
	vm->GetValue(hTable, 0, &vmVar);
	vm->ReleaseValue(tableVar);

	// Create handle with VM-owned variant
	return CreateVariantHandleFromScriptVariant(ctx, vmVar);
}

static cell_t Native_ScriptVariant_Null(IPluginContext* ctx, const cell_t* params) {
	ScriptVariant_t variant;
	variant.SetType(FIELD_VOID);
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_FromTable(IPluginContext* ctx, const cell_t* params) {
	VScriptTableHandle* tableHandle = ReadVScriptHandle<VScriptTableHandle>(ctx, params[1]);
	if (!tableHandle) return 0;

	ScriptVariant_t variant;
	variant = tableHandle->GetHScript();
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_FromArray(IPluginContext* ctx, const cell_t* params) {
	VScriptArrayHandle* arrHandle = ReadVScriptHandle<VScriptArrayHandle>(ctx, params[1]);
	if (!arrHandle) return 0;

	ScriptVariant_t variant;
	variant = arrHandle->GetHScript();
	variant.SetFlags(0);
	return CreateVariantHandleFromScriptVariant(ctx, variant);
}

static cell_t Native_ScriptVariant_Type_get(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;
	return handle->GetVariant().GetType();
}

// ScriptVariant.IsNull.get
static cell_t Native_ScriptVariant_IsNull_get(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 1;
	return (handle->GetVariant().GetType() == FIELD_VOID);
}

static cell_t Native_ScriptVariant_GetInt(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	switch (handle->GetVariant().GetType()) {
		case FIELD_INTEGER: return (cell_t)(handle->GetVariant().Get<int>());
		case FIELD_FLOAT: return (cell_t)(handle->GetVariant().Get<float>());
		case FIELD_BOOLEAN: return (cell_t)(handle->GetVariant().Get<bool>());
		default: return 0;
	}
}

static cell_t Native_ScriptVariant_GetFloat(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	float result = 0.0f;
	switch (handle->GetVariant().GetType()) {
		case FIELD_FLOAT: result = (cell_t)handle->GetVariant().Get<float>(); break;
		case FIELD_INTEGER: result = (cell_t)(handle->GetVariant().Get<int>()); break;
		case FIELD_BOOLEAN: result = (cell_t)handle->GetVariant().Get<bool>() ? 1.0f : 0.0f; break;
		default: break;
	}
	return sp_ftoc(result);
}

static cell_t Native_ScriptVariant_GetBool(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	switch (handle->GetVariant().GetType()) {
		case FIELD_BOOLEAN: return (cell_t)handle->GetVariant().Get<bool>();
		case FIELD_INTEGER: return (handle->GetVariant().Get<int>()) != 0;
		case FIELD_FLOAT: return (handle->GetVariant().Get<float>()) != 0.0f;
		default: return 0;
	}
}

static cell_t Native_ScriptVariant_GetString(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	char *str = nullptr;
	handle->GetVariant().AssignTo(str);
	if (handle->GetVariant().GetType() == FIELD_CSTRING && str) {
		ctx->StringToLocalUTF8(params[2], params[3], str, nullptr);
		return strlen(str);
	}
	return 0;
}

static cell_t Native_ScriptVariant_GetVector(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	Vector vec(0, 0, 0);
	handle->GetVariant().AssignTo(&vec);
	if (handle->GetVariant().GetType() == FIELD_VECTOR &&
	    WriteVectorResult(ctx, params, 2, &vec)) {
		return 1;
	}
	return 0;
}

static cell_t Native_ScriptVariant_GetTable(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	HSCRIPT hScript = handle->GetVariant().Get<HSCRIPT>();
	if (handle->GetVariant().GetType() == FIELD_HSCRIPT &&
	    hScript &&
	    hScript != INVALID_HSCRIPT) {
		if (vm->IsTable(hScript)) {
			VScriptTableHandle* h = new VScriptTableHandle(hScript, false);
			return CreateVScriptHandle(ctx, h);
		}
	}
	return 0;
}

static cell_t Native_ScriptVariant_GetArray(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	HSCRIPT hScript = handle->GetVariant().Get<HSCRIPT>();
	if (handle->GetVariant().GetType() == FIELD_HSCRIPT &&
	    hScript &&
	    hScript != INVALID_HSCRIPT) {
		if (vm->IsArray(hScript)) {
			VScriptArrayHandle* h = new VScriptArrayHandle(hScript, false);
			return CreateVScriptHandle(ctx, h);
		}
	}
	return 0;
}

static cell_t Native_ScriptVariant_GetScope(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	HSCRIPT hScript = handle->GetVariant().Get<HSCRIPT>();
	if (handle->GetVariant().GetType() == FIELD_HSCRIPT &&
	    hScript &&
	    hScript != INVALID_HSCRIPT) {
		VScriptScopeHandle* h = new VScriptScopeHandle(hScript, false);
		return CreateVScriptHandle(ctx, h);
	}
	return 0;
}

static cell_t Native_ScriptVariant_GetFunction(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return 0;

	HSCRIPT hScript = handle->GetVariant().Get<HSCRIPT>();
	if (handle->GetVariant().GetType() == FIELD_HSCRIPT &&
	    hScript &&
	    hScript != INVALID_HSCRIPT) {
		VScriptFunctionHandle* h = new VScriptFunctionHandle(hScript, false, false);
		return CreateVScriptHandle(ctx, h);
	}
	return 0;
}

static cell_t Native_ScriptVariant_FromEntity(IPluginContext* ctx, const cell_t* params) {
	int entity = params[1];

	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) {
		ctx->ThrowNativeError("Failed to get VM Instance.");
		return 0;
	}

	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to get root table.");
		return 0;
	}

	// Use EntIndexToHScript to convert entity index to instance (more general than PlayerInstanceFromIndex)
	char code[128];
	snprintf(code, sizeof(code), "return EntIndexToHScript(%d)", entity);

	HSCRIPT compiled = vm->CompileScript(code, "FromEntity");
	if (!compiled || compiled == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to compile script.");
		return 0;
	}

	ScriptVariant_t returnValue;
	ScriptStatus_t status = vm->ExecuteFunction(compiled, nullptr, 0, &returnValue, root, true);
	vm->ReleaseScript(compiled);

	if (status != SCRIPT_DONE) {
		ctx->ThrowNativeError("Failed to execute compiled script. status: %d, returnResult Type: %d", status, returnValue.GetType());
		return 0;
	}

	return CreateVariantHandleFromScriptVariant(ctx, returnValue);
}

static cell_t Native_ScriptVariant_ToEntity(IPluginContext* ctx, const cell_t* params) {
	VScriptVariantHandle* handle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[1]);
	if (!handle) return -1;

	HSCRIPT hScript = handle->GetVariant().Get<HSCRIPT>();
	if (handle->GetVariant().GetType() != FIELD_HSCRIPT ||
	    !hScript ||
	    hScript == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Invalid HSCRIPT from handle.");
		return -1;
	}

	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return -1;

	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) {
		ctx->ThrowNativeError("Failed to get root table.");
		return 0;
	}

	// Call GetEntityIndex() method on the entity instance
	ScriptVariant_t tableVar;
	HSCRIPT hTable = handle->GetVariant().Get<HSCRIPT>();
	tableVar = hTable;
	vm->SetValue(root, "__sm_temp_entity__", tableVar);

	const char* code = "return __sm_temp_entity__.GetEntityIndex()";
	HSCRIPT compiled = vm->CompileScript(code, "ToEntity");
	if (!compiled || compiled == INVALID_HSCRIPT) {
		vm->ClearValue(root, "__sm_temp_entity__");
		return -1;
	}

	ScriptVariant_t returnValue;
	ScriptStatus_t status = vm->ExecuteFunction(compiled, nullptr, 0, &returnValue, root, true);
	vm->ReleaseScript(compiled);
	vm->ClearValue(root, "__sm_temp_entity__");

	if (status != SCRIPT_DONE || returnValue.GetType() != FIELD_INTEGER) {
		ctx->ThrowNativeError("Failed to execute compiled script. status: %d, returnResult Type: %d", status, returnValue.GetType());
		return -1;
	}

	return (int)returnValue;
}

const sp_nativeinfo_t g_VariantNatives[] = {
	{"ScriptVariant.FromInt",       Native_ScriptVariant_FromInt},
	{"ScriptVariant.FromFloat",     Native_ScriptVariant_FromFloat},
	{"ScriptVariant.FromBool",      Native_ScriptVariant_FromBool},
	{"ScriptVariant.FromString",    Native_ScriptVariant_FromString},
	{"ScriptVariant.FromVector",    Native_ScriptVariant_FromVector},
	{"ScriptVariant.FromTable",     Native_ScriptVariant_FromTable},
	{"ScriptVariant.FromArray",     Native_ScriptVariant_FromArray},
	{"ScriptVariant.FromEntity",    Native_ScriptVariant_FromEntity},
	{"ScriptVariant.Null",          Native_ScriptVariant_Null},
	{"ScriptVariant.Type.get",      Native_ScriptVariant_Type_get},
	{"ScriptVariant.IsNull.get",    Native_ScriptVariant_IsNull_get},
	{"ScriptVariant.Int.get",       Native_ScriptVariant_GetInt},
	{"ScriptVariant.Float.get",     Native_ScriptVariant_GetFloat},
	{"ScriptVariant.Bool.get",      Native_ScriptVariant_GetBool},
	{"ScriptVariant.GetString",     Native_ScriptVariant_GetString},
	{"ScriptVariant.GetVector",     Native_ScriptVariant_GetVector},
	{"ScriptVariant.Table.get",     Native_ScriptVariant_GetTable},
	{"ScriptVariant.Array.get",     Native_ScriptVariant_GetArray},
	{"ScriptVariant.Scope.get",     Native_ScriptVariant_GetScope},
	{"ScriptVariant.Function.get",  Native_ScriptVariant_GetFunction},
	{"ScriptVariant.ToEntity",      Native_ScriptVariant_ToEntity},
	{nullptr,                       nullptr}
};
