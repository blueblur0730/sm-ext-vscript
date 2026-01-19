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

// Base class implementations
bool VScriptBaseHandle::ValidateGeneration(IPluginContext* ctx) const {
	if (vmGeneration != g_VScriptManager.GetVMGeneration()) {
		ctx->ReportError("%s handle is from a different VM (stale after map change)", GetTypeName());
		return false;
	}
	return true;
}

// VScriptVariantHandle implementations
void VScriptVariantHandle::Cleanup(IScriptVM* vm) {
	if (ownsMemory && (variant.m_flags & SV_FREE) && vm) {
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
		IScriptVM* vm = g_VScriptManager.GetVM();
		if (vm && handle->GetVMGeneration() == g_VScriptManager.GetVMGeneration()) {
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
void ShutdownHandleTypes() {
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

// Template implementations
template<typename T>
T* ReadVScriptHandle(IPluginContext* ctx, Handle_t handle) {
	HandleError err;
	HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
	void* object;

	T dummy;  // To get type info
	if ((err = handlesys->ReadHandle(handle, dummy.GetHandleType(), &sec, &object)) != HandleError_None) {
		ctx->ReportError("Invalid %s handle %x (error %d)", dummy.GetTypeName(), handle, err);
		return nullptr;
	}
	return static_cast<T*>(object);
}

// Explicit template instantiations
template VScriptVariantHandle* ReadVScriptHandle<VScriptVariantHandle>(IPluginContext*, Handle_t);
template VScriptScopeHandle* ReadVScriptHandle<VScriptScopeHandle>(IPluginContext*, Handle_t);
template VScriptTableHandle* ReadVScriptHandle<VScriptTableHandle>(IPluginContext*, Handle_t);
template VScriptArrayHandle* ReadVScriptHandle<VScriptArrayHandle>(IPluginContext*, Handle_t);
template VScriptFunctionHandle* ReadVScriptHandle<VScriptFunctionHandle>(IPluginContext*, Handle_t);

// Polymorphic handle reading
VScriptBaseHandle* ReadAnyVScriptHandle(IPluginContext* ctx, Handle_t handle) {
	HandleError err;
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

// Template for VM and HSCRIPT retrieval
template<typename T>
bool GetVMAndHScript(IPluginContext* ctx, cell_t handleParam, IScriptVM*& vm, HSCRIPT& hscript) {
	vm = g_VScriptManager.GetVM();
	if (!vm) return false;

	T* handle = ReadVScriptHandle<T>(ctx, handleParam);
	if (!handle) return false;

	if (!handle->ValidateGeneration(ctx)) return false;

	hscript = handle->GetHScript();
	return true;
}

// Explicit template instantiations
template bool GetVMAndHScript<VScriptScopeHandle>(IPluginContext*, cell_t, IScriptVM*&, HSCRIPT&);
template bool GetVMAndHScript<VScriptTableHandle>(IPluginContext*, cell_t, IScriptVM*&, HSCRIPT&);
template bool GetVMAndHScript<VScriptArrayHandle>(IPluginContext*, cell_t, IScriptVM*&, HSCRIPT&);

// Backward compatibility wrappers
Handle_t CreateScriptVariantHandle(IPluginContext* ctx, VScriptVariantHandle* variant) {
	return CreateVScriptHandle(ctx, variant);
}

Handle_t CreateVScriptScopeHandle(IPluginContext* ctx, VScriptScopeHandle* handle) {
	return CreateVScriptHandle(ctx, handle);
}

Handle_t CreateVScriptTableHandle(IPluginContext* ctx, VScriptTableHandle* handle) {
	return CreateVScriptHandle(ctx, handle);
}

Handle_t CreateVScriptArrayHandle(IPluginContext* ctx, VScriptArrayHandle* handle) {
	return CreateVScriptHandle(ctx, handle);
}

Handle_t CreateVScriptFunctionHandle(IPluginContext* ctx, VScriptFunctionHandle* handle) {
	return CreateVScriptHandle(ctx, handle);
}

VScriptVariantHandle* ReadScriptVariantHandle(IPluginContext* ctx, Handle_t handle) {
	return ReadVScriptHandle<VScriptVariantHandle>(ctx, handle);
}

VScriptScopeHandle* ReadVScriptScopeHandle(IPluginContext* ctx, Handle_t handle) {
	return ReadVScriptHandle<VScriptScopeHandle>(ctx, handle);
}

VScriptTableHandle* ReadVScriptTableHandle(IPluginContext* ctx, Handle_t handle) {
	return ReadVScriptHandle<VScriptTableHandle>(ctx, handle);
}

VScriptArrayHandle* ReadVScriptArrayHandle(IPluginContext* ctx, Handle_t handle) {
	return ReadVScriptHandle<VScriptArrayHandle>(ctx, handle);
}

VScriptFunctionHandle* ReadVScriptFunctionHandle(IPluginContext* ctx, Handle_t handle) {
	return ReadVScriptHandle<VScriptFunctionHandle>(ctx, handle);
}

// VM generation validation
bool ValidateVScriptHandleGeneration(IPluginContext* ctx, VScriptBaseHandle* handle, const char* typeName) {
	if (!handle) return false;
	return handle->ValidateGeneration(ctx);
}

bool ValidateScriptVariantHandleGeneration(IPluginContext* ctx, VScriptVariantHandle* handle) {
	if (!handle) return false;
	return handle->ValidateGeneration(ctx);
}
