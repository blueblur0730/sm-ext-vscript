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

#include "smsdk_config.h"
#include "smsdk_ext.h"

class VscriptExt : public SDKExtension {
public:
	virtual bool SDK_OnLoad(char* error, size_t maxlength, bool late) override;
	virtual void SDK_OnUnload() override;
	virtual bool SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late) override;
};

// Native registration
extern const sp_nativeinfo_t g_VariantNatives[];
extern const sp_nativeinfo_t g_ScopeNatives[];
extern const sp_nativeinfo_t g_TableNatives[];
extern const sp_nativeinfo_t g_ArrayNatives[];
extern const sp_nativeinfo_t g_FunctionNatives[];
extern const sp_nativeinfo_t g_VMNatives[];

extern VscriptExt g_VscriptExt;
