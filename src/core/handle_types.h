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

#pragma once

#include <vscript/ivscript.h>
#include <smsdk_ext.h>
#include "vscript_manager.h"

// Handle type declarations
extern HandleType_t g_ScriptVariantType;
extern HandleType_t g_VScriptScopeType;
extern HandleType_t g_VScriptTableType;
extern HandleType_t g_VScriptArrayType;
extern HandleType_t g_VScriptFunctionType;

// Base class for all VScript handles
class VScriptBaseHandle {
protected:
	int vmGeneration;

public:
	VScriptBaseHandle() : vmGeneration(0) {}
	virtual ~VScriptBaseHandle() = default;

	// Virtual interface
	virtual bool IsValid() const = 0;
	virtual HSCRIPT GetHScript() const { return INVALID_HSCRIPT; }
	virtual HandleType_t GetHandleType() const = 0;
	virtual const char* GetTypeName() const = 0;
	virtual void Cleanup(IScriptVM* vm) = 0;

	// Common functionality
	int GetVMGeneration() const { return vmGeneration; }
	void SetVMGeneration(int gen) { vmGeneration = gen; }
	bool ValidateGeneration(IPluginContext* ctx) const;
};

// ScriptVariant handle (stores any type of value)
class VScriptVariantHandle : public VScriptBaseHandle {
private:
	ScriptVariant_t variant;
	bool ownsMemory;

public:
	VScriptVariantHandle() : ownsMemory(false) {
		variant.m_type = FIELD_VOID;
	}

	bool IsValid() const override { return variant.m_type != FIELD_VOID; }
	HandleType_t GetHandleType() const override { return g_ScriptVariantType; }
	const char* GetTypeName() const override { return "ScriptVariant"; }
	void Cleanup(IScriptVM* vm) override;

	ScriptVariant_t& GetVariant() { return variant; }
	const ScriptVariant_t& GetVariant() const { return variant; }
	void SetOwnsMemory(bool owns) { ownsMemory = owns; }
	bool GetOwnsMemory() const { return ownsMemory; }
};

// Base class for HSCRIPT-based handles
class VScriptHScriptHandle : public VScriptBaseHandle {
protected:
	HSCRIPT hScript;
	bool ownsHandle;
	ScriptVariant_t variant;
	bool hasVariant;

	virtual void CleanupHScript(IScriptVM* vm) = 0;

public:
	VScriptHScriptHandle() : hScript(INVALID_HSCRIPT), ownsHandle(false), hasVariant(false) {}
	VScriptHScriptHandle(HSCRIPT h, bool owns = true) : hScript(h), ownsHandle(owns), hasVariant(false) {}

	bool IsValid() const override { return hScript != INVALID_HSCRIPT; }
	HSCRIPT GetHScript() const override { return hScript; }
	void Cleanup(IScriptVM* vm) override;

	void SetVariant(const ScriptVariant_t& v) {
		variant = v;
		hasVariant = true;
	}
};

// Specific HSCRIPT handle types
class VScriptScopeHandle : public VScriptHScriptHandle {
public:
	using VScriptHScriptHandle::VScriptHScriptHandle;

	HandleType_t GetHandleType() const override { return g_VScriptScopeType; }
	const char* GetTypeName() const override { return "VScriptScope"; }

protected:
	void CleanupHScript(IScriptVM* vm) override;
};

class VScriptTableHandle : public VScriptHScriptHandle {
public:
	using VScriptHScriptHandle::VScriptHScriptHandle;

	HandleType_t GetHandleType() const override { return g_VScriptTableType; }
	const char* GetTypeName() const override { return "VScriptTable"; }

protected:
	void CleanupHScript(IScriptVM* vm) override;
};

class VScriptArrayHandle : public VScriptHScriptHandle {
public:
	using VScriptHScriptHandle::VScriptHScriptHandle;

	HandleType_t GetHandleType() const override { return g_VScriptArrayType; }
	const char* GetTypeName() const override { return "VScriptArray"; }

protected:
	void CleanupHScript(IScriptVM* vm) override;
};

class VScriptFunctionHandle : public VScriptHScriptHandle {
private:
	bool isCompiledScript;

public:
	VScriptFunctionHandle() : VScriptHScriptHandle(), isCompiledScript(false) {}
	VScriptFunctionHandle(HSCRIPT h, bool owns, bool compiled)
		: VScriptHScriptHandle(h, owns), isCompiledScript(compiled) {}

	HandleType_t GetHandleType() const override { return g_VScriptFunctionType; }
	const char* GetTypeName() const override { return "VScriptFunction"; }

	void SetCompiledScript(bool compiled) { isCompiledScript = compiled; }
	bool IsCompiledScript() const { return isCompiledScript; }

protected:
	void CleanupHScript(IScriptVM* vm) override;
};

// Unified handler for all VScript types
class VScriptHandlerUnified : public IHandleTypeDispatch {
public:
	void OnHandleDestroy(HandleType_t type, void* object) override;
};

// Global handler instance
extern VScriptHandlerUnified g_VScriptHandlerUnified;

// Helper functions
bool InitializeHandleTypes();
void ShutdownHandleTypes();

// Template-based handle creation
template<typename T>
Handle_t CreateVScriptHandle(IPluginContext* ctx, T* handle) {
	handle->SetVMGeneration(g_VScriptManager.GetVMGeneration());
	return handlesys->CreateHandle(handle->GetHandleType(), handle,
		ctx->GetIdentity(), myself->GetIdentity(), nullptr);
}

// Template-based handle reading
template<typename T>
T* ReadVScriptHandle(IPluginContext* ctx, Handle_t handle);

// Polymorphic handle reading (for IsValid)
VScriptBaseHandle* ReadAnyVScriptHandle(IPluginContext* ctx, Handle_t handle);

// Template-based VM and HSCRIPT retrieval
template<typename T>
bool GetVMAndHScript(IPluginContext* ctx, cell_t handleParam, IScriptVM*& vm, HSCRIPT& hscript);

// Backward compatibility wrappers (will be removed after migration)
Handle_t CreateScriptVariantHandle(IPluginContext* ctx, VScriptVariantHandle* variant);
Handle_t CreateVScriptScopeHandle(IPluginContext* ctx, VScriptScopeHandle* handle);
Handle_t CreateVScriptTableHandle(IPluginContext* ctx, VScriptTableHandle* handle);
Handle_t CreateVScriptArrayHandle(IPluginContext* ctx, VScriptArrayHandle* handle);
Handle_t CreateVScriptFunctionHandle(IPluginContext* ctx, VScriptFunctionHandle* handle);

VScriptVariantHandle* ReadScriptVariantHandle(IPluginContext* ctx, Handle_t handle);
VScriptScopeHandle* ReadVScriptScopeHandle(IPluginContext* ctx, Handle_t handle);
VScriptTableHandle* ReadVScriptTableHandle(IPluginContext* ctx, Handle_t handle);
VScriptArrayHandle* ReadVScriptArrayHandle(IPluginContext* ctx, Handle_t handle);
VScriptFunctionHandle* ReadVScriptFunctionHandle(IPluginContext* ctx, Handle_t handle);

// VM generation validation
bool ValidateVScriptHandleGeneration(IPluginContext* ctx, VScriptBaseHandle* handle, const char* typeName);
bool ValidateScriptVariantHandleGeneration(IPluginContext* ctx, VScriptVariantHandle* handle);
