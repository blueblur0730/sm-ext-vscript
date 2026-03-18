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

static cell_t Native_VScriptScope_Create(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	char* name;
	ctx->LocalToString(params[1], &name);

	HSCRIPT parent = nullptr;
	if (params[2] != 0) {
		VScriptScopeHandle* parentHandle = ReadVScriptHandle<VScriptScopeHandle>(ctx, params[2]);
		if (!parentHandle) return 0;
		parent = parentHandle->GetHScript();
	}

	HSCRIPT scope = vm->CreateScope(name, parent);
	if (!scope || scope == INVALID_HSCRIPT) return 0;

	VScriptScopeHandle* handle = new VScriptScopeHandle(scope, true);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptScope_GetRoot(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	HSCRIPT root = vm->GetRootTable();
	if (!root || root == INVALID_HSCRIPT) return 0;

	// Root table is not owned by us
	VScriptScopeHandle* handle = new VScriptScopeHandle(root, false);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptScope_SetInt(IPluginContext* ctx, const cell_t* params) {
	return GenericSetInt<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_SetFloat(IPluginContext* ctx, const cell_t* params) {
	return GenericSetFloat<VScriptScopeHandle>(ctx, params);
}

// VScriptScope.SetBool
static cell_t Native_VScriptScope_SetBool(IPluginContext* ctx, const cell_t* params) {
	return GenericSetBool<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_SetString(IPluginContext* ctx, const cell_t* params) {
	return GenericSetString<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_SetVector(IPluginContext* ctx, const cell_t* params) {
	return GenericSetVector<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_SetValue(IPluginContext* ctx, const cell_t* params) {
	return GenericSetValue<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_SetTable(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	VScriptTableHandle* tableHandle = ReadVScriptHandle<VScriptTableHandle>(ctx, params[3]);
	if (!tableHandle) return 0;

	ScriptVariant_t variant;
	variant = tableHandle->GetHScript();
	return vm->SetValue(scope, key, variant);
}

static cell_t Native_VScriptScope_SetArray(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	VScriptArrayHandle* arrHandle = ReadVScriptHandle<VScriptArrayHandle>(ctx, params[3]);
	if (!arrHandle) return 0;

	ScriptVariant_t variant;
	variant = arrHandle->GetHScript();
	return vm->SetValue(scope, key, variant);
}

static cell_t Native_VScriptScope_GetInt(IPluginContext* ctx, const cell_t* params) {
	return GenericGetInt<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetFloat(IPluginContext* ctx, const cell_t* params) {
	return GenericGetFloat<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetBool(IPluginContext* ctx, const cell_t* params) {
	return GenericGetBool<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetString(IPluginContext* ctx, const cell_t* params) {
	return GenericGetString<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetVector(IPluginContext* ctx, const cell_t* params) {
	return GenericGetVector<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetValue(IPluginContext* ctx, const cell_t* params) {
	return GenericGetValue<VScriptScopeHandle>(ctx, params);
}

static cell_t Native_VScriptScope_GetTable(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(scope, key, &variant)) return 0;
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	HSCRIPT hTable = variant;
	if (variant.GetType() == FIELD_HSCRIPT && hTable && hTable != INVALID_HSCRIPT) {
		VScriptTableHandle* handle = new VScriptTableHandle(hTable, false);
		result = CreateVScriptHandle(ctx, handle);
	}

	return result;
}

static cell_t Native_VScriptScope_GetArray(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(scope, key, &variant)) return 0;
	AutoReleaseVariant autoRelease(vm, variant);

	cell_t result = 0;
	HSCRIPT hTable = variant;
	if (variant.GetType() == FIELD_HSCRIPT && hTable && hTable != INVALID_HSCRIPT) {
		VScriptArrayHandle* handle = new VScriptArrayHandle(hTable, false);
		result = CreateVScriptHandle(ctx, handle);
	}

	return result;
}

static cell_t Native_VScriptScope_HasKey(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->ValueExists(scope, key);
}

static cell_t Native_VScriptScope_ClearKey(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);
	return vm->ClearValue(scope, key);
}

static cell_t Native_VScriptScope_LookupFunction(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	char* name;
	ctx->LocalToString(params[2], &name);

	HSCRIPT func = vm->LookupFunction(name, scope);
	// Check for both INVALID_HSCRIPT (-1) and nullptr (0)
	if (func == INVALID_HSCRIPT || func == nullptr) return 0;

	VScriptFunctionHandle* handle = new VScriptFunctionHandle(func, true, false);
	return CreateVScriptHandle(ctx, handle);
}

static cell_t Native_VScriptScope_Execute(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT scope;
	if (!GetVMAndHScript<VScriptScopeHandle>(ctx, params[1], vm, scope)) return 0;

	FormattedString code(ctx, params, 2);

	// Compile the script
	HSCRIPT compiled = vm->CompileScript(code, "execute");

	if (!compiled || compiled == INVALID_HSCRIPT) return 0;

	// Execute and get return value
	ScriptVariant_t returnValue;
	ScriptStatus_t status = vm->ExecuteFunction(compiled, nullptr, 0, &returnValue, scope, true);
	vm->ReleaseScript(compiled);

	if (status != SCRIPT_DONE) return 0;

	return CreateVariantHandleFromScriptVariant(ctx, returnValue);
}

const sp_nativeinfo_t g_ScopeNatives[] = {
	// VScriptScope
	{"VScriptScope.VScriptScope",    Native_VScriptScope_Create},
	{"VScriptScope.GetRoot",        Native_VScriptScope_GetRoot},
	{"VScriptScope.SetInt",         Native_VScriptScope_SetInt},
	{"VScriptScope.SetFloat",       Native_VScriptScope_SetFloat},
	{"VScriptScope.SetBool",        Native_VScriptScope_SetBool},
	{"VScriptScope.SetString",      Native_VScriptScope_SetString},
	{"VScriptScope.SetVector",      Native_VScriptScope_SetVector},
	{"VScriptScope.SetValue",       Native_VScriptScope_SetValue},
	{"VScriptScope.SetTable",       Native_VScriptScope_SetTable},
	{"VScriptScope.SetArray",       Native_VScriptScope_SetArray},
	{"VScriptScope.GetInt",         Native_VScriptScope_GetInt},
	{"VScriptScope.GetFloat",       Native_VScriptScope_GetFloat},
	{"VScriptScope.GetBool",        Native_VScriptScope_GetBool},
	{"VScriptScope.GetString",      Native_VScriptScope_GetString},
	{"VScriptScope.GetVector",      Native_VScriptScope_GetVector},
	{"VScriptScope.GetValue",       Native_VScriptScope_GetValue},
	{"VScriptScope.GetTable",       Native_VScriptScope_GetTable},
	{"VScriptScope.GetArray",       Native_VScriptScope_GetArray},
	{"VScriptScope.HasKey",         Native_VScriptScope_HasKey},
	{"VScriptScope.ClearKey",       Native_VScriptScope_ClearKey},
	{"VScriptScope.LookupFunction", Native_VScriptScope_LookupFunction},
	{"VScriptScope.Execute",        Native_VScriptScope_Execute},
	{nullptr,                       nullptr}
};
