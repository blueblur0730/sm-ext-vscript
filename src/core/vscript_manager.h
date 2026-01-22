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
#include <IHandleSys.h>
#include <unordered_set>

class VScriptManager {
public:
	bool Initialize(IScriptManager* scriptMgr);
	void Shutdown();

	IScriptVM* GetVM() const { return m_vm; }
	void SetVM(IScriptVM* vm) {
		if (vm != m_vm) {
			m_vm = vm;
		}
	}

	// Handle tracking
	void RegisterHandle(SourceMod::Handle_t handle);
	void UnregisterHandle(SourceMod::Handle_t handle);
	void CloseAllHandles();

private:
	IScriptManager* m_scriptMgr = nullptr;
	IScriptVM* m_vm = nullptr;
	int m_createHookId = 0;
	int m_destroyHookId = 0;

	// Active handle tracking
	std::unordered_set<SourceMod::Handle_t> m_activeHandles;
};

extern VScriptManager g_VScriptManager;
