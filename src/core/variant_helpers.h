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
#include <memory>

// RAII wrapper for automatic ScriptVariant_t cleanup
class AutoReleaseVariant {
private:
	IScriptVM* vm;
	ScriptVariant_t& variant;

public:
	AutoReleaseVariant(IScriptVM* v, ScriptVariant_t& var) : vm(v), variant(var) {}

	~AutoReleaseVariant() {
		if (variant.GetFlags() & SV_FREE) {
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
	static constexpr size_t BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	std::unique_ptr<char[]> dynamic;
	const char* str;

public:
	FormattedString(IPluginContext* ctx, const cell_t* params, int paramIndex) {
		size_t result = smutils->FormatString(buffer, BUFFER_SIZE, ctx, params, paramIndex);
		if (result >= BUFFER_SIZE) {
			dynamic = std::make_unique<char[]>(result + 1);
			smutils->FormatString(dynamic.get(), result + 1, ctx, params, paramIndex);
			str = dynamic.get();
		} else {
			str = buffer;
		}
	}

	operator const char*() const noexcept { return str; }
	const char* c_str() const noexcept { return str; }

	FormattedString(const FormattedString&) = delete;
	FormattedString& operator=(const FormattedString&) = delete;
};

// Create a VScriptVariantHandle from a ScriptVariant_t and return its Handle_t
[[nodiscard]] inline Handle_t CreateVariantHandleFromScriptVariant(IPluginContext* ctx, const ScriptVariant_t& variant) {
	VScriptVariantHandle* handle = new VScriptVariantHandle();
	handle->SetVariant(variant);
	handle->SetOwnsMemory((variant.GetFlags() & SV_FREE) != 0);
	return CreateVScriptHandle(ctx, handle);
}

// Read a Vector parameter from SourcePawn
inline Vector ReadVectorParam(IPluginContext* ctx, const cell_t* params, int paramIndex) {
	cell_t* vec;
	ctx->LocalToPhysAddr(params[paramIndex], &vec);
	return Vector(sp_ctof(vec[0]), sp_ctof(vec[1]), sp_ctof(vec[2]));
}

// Write a Vector result to SourcePawn
[[nodiscard]] inline bool WriteVectorResult(IPluginContext* ctx, const cell_t* params, int paramIndex, const Vector* vec) {
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
	if (a.GetType() != b.GetType()) return false;

	switch (a.GetType()) {
		case FIELD_INTEGER: {
			int int_a = a.Get<int>(); int int_b = b.Get<int>(); return int_a == int_b;
		}
		case FIELD_FLOAT: {
			float float_a = a.Get<float>(); float float_b = b.Get<float>(); return float_a == float_b;
		}
		case FIELD_BOOLEAN: {
			bool bool_a = a.Get<bool>(); bool bool_b = b.Get<bool>();  return bool_a == bool_b;
		}
		case FIELD_CSTRING: {
			const char* str_a; 
			const char* str_b;
			a.AssignTo(str_a);
			b.AssignTo(str_b);
			if (!str_a || !str_b) return false;
			return strcmp(str_a, str_b) == 0;
		}
		case FIELD_HSCRIPT: {
			HSCRIPT hscript_a = a.Get<HSCRIPT>(); HSCRIPT hscript_b = b.Get<HSCRIPT>(); return hscript_a == hscript_b;
		}
		case FIELD_VECTOR: {
			Vector vec_a(0, 0, 0); Vector vec_b(0, 0, 0);
			a.AssignTo(&vec_a);
			b.AssignTo(&vec_b);
			return vec_a.x == vec_b.x && vec_a.y == vec_b.y && vec_a.z == vec_b.z;
		}
		default:
			return false;
	}
}

// Create a ScriptVariant_t from a Vector (stack-allocated, no ownership transfer)
inline ScriptVariant_t CreateVectorVariant(const Vector& v) {
	ScriptVariant_t variant;
	variant = v;
	variant.ConvertToCopiedData();
	variant.SetFlags(0);  // No ownership - caller manages lifetime
	return variant;
}
