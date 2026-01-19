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

// RAII wrapper for automatic ScriptVariant_t cleanup
class AutoReleaseVariant {
private:
	IScriptVM* vm;
	ScriptVariant_t& variant;

public:
	AutoReleaseVariant(IScriptVM* v, ScriptVariant_t& var) : vm(v), variant(var) {}

	~AutoReleaseVariant() {
		if (variant.m_flags & SV_FREE) {
			vm->ReleaseValue(variant);
		}
	}

	// Prevent copying
	AutoReleaseVariant(const AutoReleaseVariant&) = delete;
	AutoReleaseVariant& operator=(const AutoReleaseVariant&) = delete;
};

// RAII wrapper for formatted strings with dynamic allocation
class FormattedString {
private:
	char buffer[512];
	char* dynamic;
	char* str;

public:
	FormattedString(IPluginContext* ctx, const cell_t* params, int paramIndex) : dynamic(nullptr) {
		size_t result = smutils->FormatString(buffer, sizeof(buffer), ctx, params, paramIndex);
		if (result >= sizeof(buffer)) {
			dynamic = new char[result + 1];
			str = dynamic;
			smutils->FormatString(dynamic, result + 1, ctx, params, paramIndex);
		} else {
			str = buffer;
		}
	}

	~FormattedString() {
		if (dynamic) delete[] dynamic;
	}

	operator const char*() const { return str; }
	const char* c_str() const { return str; }

	// Prevent copying
	FormattedString(const FormattedString&) = delete;
	FormattedString& operator=(const FormattedString&) = delete;
};

// Create a VScriptVariantHandle from a ScriptVariant_t and return its Handle_t
inline Handle_t CreateVariantHandleFromScriptVariant(IPluginContext* ctx, const ScriptVariant_t& variant) {
	VScriptVariantHandle* handle = new VScriptVariantHandle();
	handle->GetVariant() = variant;
	handle->SetOwnsMemory((variant.m_flags & SV_FREE) != 0);
	return CreateScriptVariantHandle(ctx, handle);
}

// Read a Vector parameter from SourcePawn
inline Vector ReadVectorParam(IPluginContext* ctx, const cell_t* params, int paramIndex) {
	cell_t* vec;
	ctx->LocalToPhysAddr(params[paramIndex], &vec);
	return Vector(sp_ctof(vec[0]), sp_ctof(vec[1]), sp_ctof(vec[2]));
}

// Write a Vector result to SourcePawn
inline bool WriteVectorResult(IPluginContext* ctx, const cell_t* params, int paramIndex, const Vector* vec) {
	if (!vec) return false;

	cell_t* out;
	ctx->LocalToPhysAddr(params[paramIndex], &out);
	out[0] = sp_ftoc(vec->x);
	out[1] = sp_ftoc(vec->y);
	out[2] = sp_ftoc(vec->z);
	return true;
}

// Compare two ScriptVariant_t values for equality
inline bool CompareScriptVariants(const ScriptVariant_t& a, const ScriptVariant_t& b) {
	if (a.m_type != b.m_type) return false;

	switch (a.m_type) {
		case FIELD_INTEGER:
			return a.m_int == b.m_int;
		case FIELD_FLOAT:
			return a.m_float == b.m_float;
		case FIELD_BOOLEAN:
			return a.m_bool == b.m_bool;
		case FIELD_CSTRING:
			if (!a.m_pszString || !b.m_pszString) return false;
			return strcmp(a.m_pszString, b.m_pszString) == 0;
		case FIELD_HSCRIPT:
			return a.m_hScript == b.m_hScript;
		case FIELD_VECTOR:
			if (!a.m_pVector || !b.m_pVector) return false;
			return a.m_pVector->x == b.m_pVector->x &&
			       a.m_pVector->y == b.m_pVector->y &&
			       a.m_pVector->z == b.m_pVector->z;
		default:
			return false;
	}
}

// Create a ScriptVariant_t from a Vector (stack-allocated, no ownership transfer)
inline ScriptVariant_t CreateVectorVariant(const Vector& v) {
	ScriptVariant_t variant;
	variant.m_type = FIELD_VECTOR;
	variant.m_pVector = const_cast<Vector*>(&v);
	variant.m_flags = 0;  // No ownership - caller manages lifetime
	return variant;
}
