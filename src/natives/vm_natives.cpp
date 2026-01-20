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
#include <vscript/ivscript.h>

static cell_t Native_VScript_IsValid_get(IPluginContext* ctx, const cell_t* params) {
	VScriptBaseHandle* handle = ReadAnyVScriptHandle(ctx, params[1]);
	return handle && handle->IsValid();
}

static cell_t Native_VScript_IsArray_get(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
	void* object;

	if (handlesys->ReadHandle(params[1], g_VScriptArrayType, &sec, &object) == HandleError_None) {
		VScriptArrayHandle* h = static_cast<VScriptArrayHandle*>(object);
		if (h && h->GetHScript() != INVALID_HSCRIPT) {
			return vm->IsArray(h->GetHScript());
		}
	}

	return 0;
}

static cell_t Native_VScript_IsTable_get(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
	void* object;

	if (handlesys->ReadHandle(params[1], g_VScriptTableType, &sec, &object) == HandleError_None) {
		VScriptTableHandle* h = static_cast<VScriptTableHandle*>(object);
		if (h && h->GetHScript() != INVALID_HSCRIPT) {
			return vm->IsTable(h->GetHScript());
		}
	}

	return 0;
}

static cell_t Native_VScript_Compile(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	FormattedString code(ctx, params, 1);

	// Compile the script
	HSCRIPT compiled = vm->CompileScript(code, "Compiled");

	if (compiled == INVALID_HSCRIPT) return 0;

	// Return as VScriptFunction handle with isCompiledScript=true
	VScriptFunctionHandle* handle = new VScriptFunctionHandle(compiled, true, true);
	return CreateVScriptHandle(ctx, handle);
}

// VScript_Run
static cell_t Native_VScript_Run(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	FormattedString code(ctx, params, 1);

	ScriptStatus_t status = vm->Run(code);

	return (status == SCRIPT_DONE);
}

const sp_nativeinfo_t g_VMNatives[] = {
	{"VScript.IsValid.get",         Native_VScript_IsValid_get},
	{"VScript.IsArray.get",         Native_VScript_IsArray_get},
	{"VScript.IsTable.get",         Native_VScript_IsTable_get},
	{"VScript.Compile",             Native_VScript_Compile},
	{"VScript.Run",                 Native_VScript_Run},
	{nullptr,                       nullptr}
};
