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

#include "handle_types.h"
#include "vscript_manager.h"
#include <smsdk_ext.h>

// Handle type globals
HandleType_t g_ScriptVariantType = 0;
HandleType_t g_VScriptScopeType = 0;
HandleType_t g_VScriptTableType = 0;
HandleType_t g_VScriptArrayType = 0;
HandleType_t g_VScriptFunctionType = 0;

// Unified handler instance
VScriptHandlerUnified g_VScriptHandlerUnified;

// VScriptVariantHandle implementations
void VScriptVariantHandle::Cleanup(IScriptVM* vm) {
	if (ownsMemory && (variant.GetFlags() & SV_FREE) && vm) {
		vm->ReleaseValue(variant);
	}
}

// VScriptHScriptHandle implementations
void VScriptHScriptHandle::Cleanup(IScriptVM* vm) {
	if (!vm) return;

	if (hasVariant) {
		vm->ReleaseValue(variant);
	}

	if (ownsHandle && hScript != INVALID_HSCRIPT) {
		CleanupHScript(vm);
	}
}

// Specific handle type cleanup implementations
void VScriptScopeHandle::CleanupHScript(IScriptVM* vm) {
	vm->ReleaseScope(hScript);
}

void VScriptTableHandle::CleanupHScript(IScriptVM* vm) {
	// Tables are cleaned via variant
}

void VScriptArrayHandle::CleanupHScript(IScriptVM* vm) {
	// Arrays are cleaned via variant
}

void VScriptFunctionHandle::CleanupHScript(IScriptVM* vm) {
	if (isCompiledScript) {
		vm->ReleaseScript(hScript);
	} else {
		vm->ReleaseFunction(hScript);
	}
}

// Unified handler implementation
void VScriptHandlerUnified::OnHandleDestroy(HandleType_t type, void* object) {
	VScriptBaseHandle* handle = static_cast<VScriptBaseHandle*>(object);
	if (handle) {
		// Unregister from tracking
		g_VScriptManager.UnregisterHandle(handle->GetSourceModHandle());

		IScriptVM* vm = g_VScriptManager.GetVM();
		if (vm) {
			handle->Cleanup(vm);
		}
		delete handle;
	}
}

// Initialize all handle types
bool InitializeHandleTypes() {
	HandleError err;

	g_ScriptVariantType = handlesys->CreateType("ScriptVariant", &g_VScriptHandlerUnified,
		0, nullptr, nullptr, myself->GetIdentity(), &err);
	if (!g_ScriptVariantType) return false;

	g_VScriptScopeType = handlesys->CreateType("VScriptScope", &g_VScriptHandlerUnified,
		0, nullptr, nullptr, myself->GetIdentity(), &err);
	if (!g_VScriptScopeType) return false;

	g_VScriptTableType = handlesys->CreateType("VScriptTable", &g_VScriptHandlerUnified,
		0, nullptr, nullptr, myself->GetIdentity(), &err);
	if (!g_VScriptTableType) return false;

	g_VScriptArrayType = handlesys->CreateType("VScriptArray", &g_VScriptHandlerUnified,
		0, nullptr, nullptr, myself->GetIdentity(), &err);
	if (!g_VScriptArrayType) return false;

	g_VScriptFunctionType = handlesys->CreateType("VScriptFunction", &g_VScriptHandlerUnified,
		0, nullptr, nullptr, myself->GetIdentity(), &err);
	if (!g_VScriptFunctionType) return false;

	return true;
}

// Shutdown all handle types
void RemoveHandleTypes() {
	if (g_VScriptFunctionType) {
		handlesys->RemoveType(g_VScriptFunctionType, myself->GetIdentity());
		g_VScriptFunctionType = 0;
	}
	if (g_VScriptArrayType) {
		handlesys->RemoveType(g_VScriptArrayType, myself->GetIdentity());
		g_VScriptArrayType = 0;
	}
	if (g_VScriptTableType) {
		handlesys->RemoveType(g_VScriptTableType, myself->GetIdentity());
		g_VScriptTableType = 0;
	}
	if (g_VScriptScopeType) {
		handlesys->RemoveType(g_VScriptScopeType, myself->GetIdentity());
		g_VScriptScopeType = 0;
	}
	if (g_ScriptVariantType) {
		handlesys->RemoveType(g_ScriptVariantType, myself->GetIdentity());
		g_ScriptVariantType = 0;
	}
}

// Polymorphic handle reading
VScriptBaseHandle* ReadAnyVScriptHandle(IPluginContext* ctx, Handle_t handle) {
	HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
	void* object;

	static HandleType_t types[] = {
		g_VScriptScopeType, g_VScriptTableType, g_VScriptArrayType,
		g_VScriptFunctionType, g_ScriptVariantType
	};

	for (auto type : types) {
		if (handlesys->ReadHandle(handle, type, &sec, &object) == HandleError_None) {
			return static_cast<VScriptBaseHandle*>(object);
		}
	}
	return nullptr;
}
