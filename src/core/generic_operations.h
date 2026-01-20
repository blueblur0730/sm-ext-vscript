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
	if (variant.m_type == FIELD_INTEGER) result = variant.m_int;
	else if (variant.m_type == FIELD_FLOAT) result = (cell_t)variant.m_float;

	if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
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
	if (variant.m_type == FIELD_FLOAT) result = sp_ftoc(variant.m_float);
	else if (variant.m_type == FIELD_INTEGER) result = sp_ftoc((float)variant.m_int);

	if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
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
	if (variant.m_type == FIELD_BOOLEAN) result = variant.m_bool;
	else if (variant.m_type == FIELD_INTEGER) result = variant.m_int;

	if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
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
	if (variant.m_type == FIELD_CSTRING && variant.m_pszString) {
		ctx->StringToLocalUTF8(params[3], params[4], variant.m_pszString, &written);
	}

	if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
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

	if (variant.m_type == FIELD_VECTOR && variant.m_pVector) {
		cell_t* vec;
		ctx->LocalToPhysAddr(params[3], &vec);
		vec[0] = sp_ftoc(variant.m_pVector->x);
		vec[1] = sp_ftoc(variant.m_pVector->y);
		vec[2] = sp_ftoc(variant.m_pVector->z);

		if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
		return 1;
	}

	if (variant.m_flags & SV_FREE) vm->ReleaseValue(variant);
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
	handle->GetVariant() = variant;
	handle->SetOwnsMemory((variant.m_flags & SV_FREE) != 0);
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
	variant.m_type = FIELD_INTEGER;
	variant.m_int = params[3];
	variant.m_flags = 0;

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetFloat(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	variant.m_type = FIELD_FLOAT;
	variant.m_float = sp_ctof(params[3]);
	variant.m_flags = 0;

	return vm->SetValue(hscript, key, variant);
}

template<typename HandleType>
cell_t GenericSetBool(IPluginContext* ctx, const cell_t* params) {
	IScriptVM* vm; HSCRIPT hscript;
	if (!GetVMAndHScript<HandleType>(ctx, params[1], vm, hscript)) return 0;

	char* key;
	ctx->LocalToString(params[2], &key);

	ScriptVariant_t variant;
	variant.m_type = FIELD_BOOLEAN;
	variant.m_bool = params[3] ? true : false;
	variant.m_flags = 0;

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
	variant.m_type = FIELD_CSTRING;
	variant.m_pszString = value;
	variant.m_flags = 0;

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
	variant.m_type = FIELD_VECTOR;
	variant.m_pVector = &v;
	variant.m_flags = 0;

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
