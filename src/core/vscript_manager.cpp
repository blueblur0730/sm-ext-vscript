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

#include "vscript_manager.h"
#include <cstdint>
#include <smsdk_ext.h>
#include <extension.h>

VScriptManager g_VScriptManager;

static cell_t GetVScriptVMAddress(IScriptVM* vm) {
	return static_cast<cell_t>(reinterpret_cast<intptr_t>(vm));
}

// Hook for IScriptManager::CreateVM
SH_DECL_HOOK1(IScriptManager, CreateVM, SH_NOATTRIB, 0, IScriptVM*, ScriptLanguage_t);
// Hook for IScriptManager::DestroyVM
SH_DECL_HOOK1_void(IScriptManager, DestroyVM, SH_NOATTRIB, 0, IScriptVM*);
IScriptVM* Hook_CreateVM(ScriptLanguage_t language) {
	IScriptVM* vm = META_RESULT_ORIG_RET(IScriptVM*);

	// Always capture the game's VM (it may be recreated on map change)
	if (vm) {
		g_VScriptManager.SetVM(vm);

		// Fire forward
		if (g_VscriptExt.m_OnVMCreate) {
			g_VscriptExt.m_OnVMCreate->PushCell(GetVScriptVMAddress(vm));
			g_VscriptExt.m_OnVMCreate->Execute(nullptr);
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, vm);
}

void Hook_DestroyVM(IScriptVM* vm) {
	// Close all active handles before VM is destroyed
	if (vm && g_VScriptManager.GetVM() == vm) {
		// Fire forward before cleanup
		if (g_VscriptExt.m_OnVMDestroy) {
			g_VscriptExt.m_OnVMDestroy->PushCell(GetVScriptVMAddress(vm));
			g_VscriptExt.m_OnVMDestroy->Execute(nullptr);
		}

		g_VScriptManager.CloseAllHandles();
		g_VScriptManager.SetVM(nullptr);
	}

	RETURN_META(MRES_IGNORED);
}

bool VScriptManager::Initialize(IScriptManager* scriptMgr) {
	if (!scriptMgr)
		return false;

	m_scriptMgr = scriptMgr;

	// Hook CreateVM to capture the game's VM
	m_createHookId = SH_ADD_HOOK(IScriptManager, CreateVM, scriptMgr, SH_STATIC(Hook_CreateVM), true);

	// Hook DestroyVM to clear the VM pointer (before destruction)
	m_destroyHookId = SH_ADD_HOOK(IScriptManager, DestroyVM, scriptMgr, SH_STATIC(Hook_DestroyVM), false);

	return true;
}

void VScriptManager::Shutdown() {
	if (m_createHookId && m_scriptMgr) {
		SH_REMOVE_HOOK_ID(m_createHookId);
		m_createHookId = 0;
	}

	if (m_destroyHookId && m_scriptMgr) {
		SH_REMOVE_HOOK_ID(m_destroyHookId);
		m_destroyHookId = 0;
	}

	m_vm = nullptr;
	m_scriptMgr = nullptr;
}

void VScriptManager::RegisterHandle(Handle_t handle) {
	m_activeHandles.insert(handle);
}

void VScriptManager::UnregisterHandle(Handle_t handle) {
	m_activeHandles.erase(handle);
}

void VScriptManager::CloseAllHandles() {
	// Copy the set to avoid iterator invalidation during FreeHandle
	auto handles = m_activeHandles;
	m_activeHandles.clear();

	// Use extension identity as both owner and ident to match secondary owner
	HandleSecurity sec(myself->GetIdentity(), myself->GetIdentity());

	for (Handle_t handle : handles) {
		handlesys->FreeHandle(handle, &sec);
	}
}
