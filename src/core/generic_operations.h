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

#include "handle_types.h"
#include <vscript/ivscript.h>
#include <smsdk_ext.h>

// Generic Get operations for HSCRIPT-based handles

template<typename HandleType>
cell_t GenericGetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return params[3];

	cell_t result = params[3];
	if (variant.GetType() == FIELD_INTEGER) result = (cell_t)variant.Get<int>();
	else if (variant.GetType() == FIELD_FLOAT) result = (cell_t)(variant.Get<float>());

	if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
	return result;
}

template<typename HandleType>
cell_t GenericGetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return params[3];

	cell_t result = params[3];
	if (variant.GetType() == FIELD_FLOAT) result = sp_ftoc(variant.Get<float>());
	else if (variant.GetType() == FIELD_INTEGER) result = sp_ftoc((float)(variant.Get<int>()));

	if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
	return result;
}

template<typename HandleType>
cell_t GenericGetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return params[3];

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return params[3];

	cell_t result = params[3];
	if (variant.GetType() == FIELD_BOOLEAN) result = (cell_t)(variant.Get<bool>());
	else if (variant.GetType() == FIELD_INTEGER) result = (cell_t)variant.Get<int>();

	if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
	return result;
}

template<typename HandleType>
cell_t GenericGetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return 0;

	size_t written = 0;
	char *pStr = nullptr;
	variant.AssignTo(pStr);
	if (variant.GetType() == FIELD_CSTRING && pStr) {
		ctx->StringToLocalUTF8(params[3], params[4], pStr, &written);
	}

	if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
	return written;
}

template<typename HandleType>
cell_t GenericGetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return 0;

	if (variant.GetType() == FIELD_VECTOR) {
		cell_t* vec;
		Vector pVector(0, 0, 0);
		variant.AssignTo(&pVector);
		ctx->LocalToPhysAddr(params[3], &vec);
		vec[0] = sp_ftoc(pVector.x);
		vec[1] = sp_ftoc(pVector.y);
		vec[2] = sp_ftoc(pVector.z);

		if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
		return 1;
	}

	if (variant.GetFlags() & SV_FREE) vm->ReleaseValue(variant);
	return 0;
}

template<typename HandleType>
cell_t GenericGetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	if (!vm->GetValue(hscript, key, &variant)) return 0;

	VScriptVariantHandle* handle = new VScriptVariantHandle();
	handle->SetVariant(variant);
	handle->SetOwnsMemory((variant.GetFlags() & SV_FREE) != 0);
	return CreateVScriptHandle(ctx, handle);
}

// Generic Set operations for HSCRIPT-based handles

template<typename HandleType>
cell_t GenericSetInt(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	variant = params[3];
	variant.SetFlags(0);

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	variant = sp_ctof(params[3]);
	variant.SetFlags(0);

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	variant = params[3] ? true : false;
	variant.SetFlags(0);

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetString(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	char* value;
	ctx->LocalToString(params[3], &value);

	ScriptVariant_t variant;
	variant = ScriptVariant_t(value, false);
	variant.SetFlags(0);

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetVector(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	cell_t* vec;
	ctx->LocalToPhysAddr(params[3], &vec);

	Vector v(sp_ctof(vec[0]), sp_ctof(vec[1]), sp_ctof(vec[2]));

	ScriptVariant_t variant;
	variant = v;
	variant.ConvertToCopiedData();
	variant.SetFlags(0);

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetValue(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	VScriptVariantHandle* varHandle = ReadVScriptHandle<VScriptVariantHandle>(ctx, params[3]);
	if (!varHandle) return 0;

	return vm->SetValue(hscript, key, varHandle->GetVariant());
}
