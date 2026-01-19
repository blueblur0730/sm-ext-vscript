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

// Helper to read a scope handle that can be either VScriptScope or VScriptTable
static HSCRIPT ReadScopeOrTableHandle(IPluginContext* ctx, Handle_t handle) {
	if (handle == 0) return nullptr;

	HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
	HandleError err;
	void* object;

	// Try VScriptScope first
	err = handlesys->ReadHandle(handle, g_VScriptScopeType, &sec, &object);
	if (err == HandleError_None) {
		VScriptBaseHandle* vhandle = static_cast<VScriptBaseHandle*>(object);
		return vhandle ? vhandle->GetHScript() : nullptr;
	}

	// Try VScriptTable
	err = handlesys->ReadHandle(handle, g_VScriptTableType, &sec, &object);
	if (err == HandleError_None) {
		VScriptBaseHandle* vhandle = static_cast<VScriptBaseHandle*>(object);
		return vhandle ? vhandle->GetHScript() : nullptr;
	}

	return nullptr;
}

static cell_t Native_VScriptFunction_Call(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	VScriptFunctionHandle* funcHandle = ReadVScriptFunctionHandle(ctx, params[1]);
	if (!funcHandle) return 0;

	HSCRIPT scope = ReadScopeOrTableHandle(ctx, params[2]);

	HSCRIPT funcToCall = funcHandle->GetHScript();
	bool needsRelease = false;

	// If this is a compiled script, execute it first to get the function
	if (funcHandle->IsCompiledScript()) {
		ScriptVariant_t scriptResult;
		ScriptStatus_t status = vm->ExecuteFunction(funcHandle->GetHScript(), nullptr, 0, &scriptResult, scope, true);
		if (status != SCRIPT_DONE || scriptResult.m_type != FIELD_HSCRIPT) return 0;
		funcToCall = scriptResult.m_hScript;
		needsRelease = (scriptResult.m_flags & SV_FREE) != 0;
	}

	ScriptVariant_t returnValue;
	ScriptStatus_t status = vm->ExecuteFunction(funcToCall, nullptr, 0, &returnValue, scope, true);

	if (needsRelease) {
		ScriptVariant_t temp;
		temp.m_type = FIELD_HSCRIPT;
		temp.m_hScript = funcToCall;
		temp.m_flags = SV_FREE;
		vm->ReleaseValue(temp);
	}

	if (status != SCRIPT_DONE) return 0;

	return CreateVariantHandleFromScriptVariant(ctx, returnValue);
}

static cell_t Native_VScriptFunction_CallWithArgs(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm = g_VScriptManager.GetVM();
	if (!vm) return 0;

	VScriptFunctionHandle* funcHandle = ReadVScriptFunctionHandle(ctx, params[1]);
	if (!funcHandle) return 0;

	HSCRIPT scope = ReadScopeOrTableHandle(ctx, params[2]);

	// params[0] contains the number of parameters passed
	// params[1] = function handle, params[2] = scope
	// params[3]+ = variadic ScriptVariant handles
	int numArgs = params[0] - 2;  // Subtract function and scope params
	if (numArgs < 0) numArgs = 0;
	if (numArgs > 16) numArgs = 16;

#ifdef _WIN32
	// Windows: VScript expects 16-byte aligned ScriptVariant_t?
	struct AlignedVariant {
		ScriptVariant_t variant;
		int padding;  // Pad to 16 bytes
	};
	AlignedVariant args[16];
	memset(args, 0, sizeof(args));

	for (int i = 0; i < numArgs; i++) {
		cell_t* addr;
		if (ctx->LocalToPhysAddr(params[3 + i], &addr) != SP_ERROR_NONE) {
			args[i].variant.m_type = FIELD_VOID;
			continue;
		}

		cell_t handleId = *addr;
		VScriptVariantHandle* argHandle = ReadScriptVariantHandle(ctx, handleId);
		if (argHandle) {
			args[i].variant = argHandle->GetVariant();
		} else {
			args[i].variant.m_type = FIELD_VOID;
		}
	}
#else
	// Linux: VScript expects 12-byte ScriptVariant_t (no padding?)
	ScriptVariant_t args[16];
	memset(args, 0, sizeof(args));

	for (int i = 0; i < numArgs; i++) {
		cell_t* addr;
		if (ctx->LocalToPhysAddr(params[3 + i], &addr) != SP_ERROR_NONE) {
			args[i].m_type = FIELD_VOID;
			continue;
		}

		cell_t handleId = *addr;
		VScriptVariantHandle* argHandle = ReadScriptVariantHandle(ctx, handleId);
		if (argHandle) {
			args[i] = argHandle->GetVariant();
		} else {
			args[i].m_type = FIELD_VOID;
		}
	}
#endif

	HSCRIPT funcToCall = funcHandle->GetHScript();
	bool needsRelease = false;

	// If this is a compiled script, execute it first to get the function
	if (funcHandle->IsCompiledScript()) {
		ScriptVariant_t scriptResult;
		ScriptStatus_t status = vm->ExecuteFunction(funcHandle->GetHScript(), nullptr, 0, &scriptResult, scope, true);
		if (status != SCRIPT_DONE || scriptResult.m_type != FIELD_HSCRIPT) return 0;
		funcToCall = scriptResult.m_hScript;
		needsRelease = (scriptResult.m_flags & SV_FREE) != 0;
	}

	ScriptVariant_t returnValue;
#ifdef _WIN32
	ScriptStatus_t status = vm->ExecuteFunction(funcToCall, (ScriptVariant_t*)args, numArgs, &returnValue, scope, true);
#else
	ScriptStatus_t status = vm->ExecuteFunction(funcToCall, args, numArgs, &returnValue, scope, true);
#endif

	if (needsRelease) {
		ScriptVariant_t temp;
		temp.m_type = FIELD_HSCRIPT;
		temp.m_hScript = funcToCall;
		temp.m_flags = SV_FREE;
		vm->ReleaseValue(temp);
	}

	if (status != SCRIPT_DONE) return 0;

	return CreateVariantHandleFromScriptVariant(ctx, returnValue);
}

const sp_nativeinfo_t g_FunctionNatives[] = {
	{"VScriptFunction.Call",            Native_VScriptFunction_Call},
	{"VScriptFunction.CallWithArgs",    Native_VScriptFunction_CallWithArgs},
	{nullptr,                           nullptr}
};
