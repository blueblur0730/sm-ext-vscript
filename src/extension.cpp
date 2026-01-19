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
#include "core/vscript_manager.h"
#include "core/handle_types.h"

VscriptExt g_VscriptExt;
SMEXT_LINK(&g_VscriptExt);

IScriptManager* g_pScriptManager = nullptr;

bool VscriptExt::SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
	GET_V_IFACE_ANY(GetEngineFactory, g_pScriptManager, IScriptManager, VSCRIPT_INTERFACE_VERSION);
	if (!g_pScriptManager) {
		snprintf(error, maxlen, "Couldn't get IScriptManager!");
		return false;
	}

	if (!g_VScriptManager.Initialize(g_pScriptManager)) {
		snprintf(error, maxlen, "Failed to initialize VScriptManager!");
		return false;
	}

	return true;
}

bool VscriptExt::SDK_OnLoad(char* error, size_t maxlength, bool late) {
	// Initialize handle types
	if (!InitializeHandleTypes()) {
		snprintf(error, maxlength, "Failed to initialize handle types!");
		return false;
	}

	// Register all natives
	sharesys->AddNatives(myself, g_VariantNatives);
	sharesys->AddNatives(myself, g_ScopeNatives);
	sharesys->AddNatives(myself, g_TableNatives);
	sharesys->AddNatives(myself, g_ArrayNatives);
	sharesys->AddNatives(myself, g_FunctionNatives);
	sharesys->AddNatives(myself, g_VMNatives);

	// Register library
	sharesys->RegisterLibrary(myself, "vscript");

	return true;
}

void VscriptExt::SDK_OnUnload() {
	ShutdownHandleTypes();
	g_VScriptManager.Shutdown();
}
