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

class CSquirrelVM : public IScriptVM
{
public:
	virtual bool Init() { return true; }
	virtual void Shutdown() { }

	virtual bool ConnectDebugger() { return true; }
	virtual void DisconnectDebugger() { }

	virtual ScriptLanguage_t GetLanguage() { return SL_NONE; }
	virtual const char *GetLanguageName() { return NULL; }

	virtual void *GetInternalVM() { return NULL; }	// SQVM*		m_pVM; this + 4

	virtual void AddSearchPath( const char *pszSearchPath ) { }

	//--------------------------------------------------------

	virtual bool ForwardConsoleCommand(CCommandContext const & commandContext, CCommand const &command) { return true; }
	virtual bool Frame( float simTime ) { return true; }

	//--------------------------------------------------------
	// Simple script usage
	//--------------------------------------------------------
	virtual ScriptStatus_t Run(const char *pszScript, bool bWait = true) { return SCRIPT_ERROR; }

	//--------------------------------------------------------
	// Compilation
	//--------------------------------------------------------
	virtual HSCRIPT CompileScript(const char *pszScript, const char *pszId = NULL) { return INVALID_HSCRIPT; }
	virtual void ReleaseScript( HSCRIPT hScript ) { }

	//--------------------------------------------------------
	// Execution of compiled
	//--------------------------------------------------------
	virtual ScriptStatus_t Run( HSCRIPT hScript, HSCRIPT hScope = NULL, bool bWait = true ) { return SCRIPT_ERROR; }
	virtual ScriptStatus_t Run( HSCRIPT hScript, bool bWait ) { return SCRIPT_ERROR; }

	//--------------------------------------------------------
	// Scope
	//--------------------------------------------------------
	virtual HSCRIPT CreateScope( const char *pszScope, HSCRIPT hParent = NULL ) { return INVALID_HSCRIPT; }
	virtual HSCRIPT ReferenceScope( HSCRIPT hScript ) { return INVALID_HSCRIPT; }
	virtual void ReleaseScope( HSCRIPT hScript ) { }

	//--------------------------------------------------------
	// Script functions
	//--------------------------------------------------------
	virtual HSCRIPT LookupFunction( const char *pszFunction, HSCRIPT hScope = NULL, bool bNoDelegation = false ) { return INVALID_HSCRIPT; }
	virtual void ReleaseFunction( HSCRIPT hScript ) { }

	//--------------------------------------------------------
	// Script functions (raw, use Call())
	//--------------------------------------------------------
	virtual ScriptStatus_t ExecuteFunction( HSCRIPT hFunction, ScriptVariant_t *pArgs, int nArgs, ScriptVariant_t *pReturn, HSCRIPT hScope, bool bWait ) { return SCRIPT_ERROR; }

	virtual void RegisterFunction( ScriptFunctionBinding_t *pScriptFunction ){ }
	virtual bool RegisterClass( ScriptClassDesc_t *pClassDesc ) { return true; }

	virtual HSCRIPT RegisterInstance( ScriptClassDesc_t *pDesc, void *pInstance ) { return INVALID_HSCRIPT; }
	virtual void SetInstanceUniqeId( HSCRIPT hInstance, const char *pszId ) { }
	virtual void RemoveInstance( HSCRIPT hInstance ) { }
	virtual void *GetInstanceValue( HSCRIPT hInstance, ScriptClassDesc_t *pExpectedType = NULL ) { return NULL; }

	virtual bool GenerateUniqueKey( const char *pszRoot, char *pBuf, int nBufSize ) { return true; }

	virtual bool ValueExists( HSCRIPT hScope, const char *pszKey ) { return true; }
	virtual bool SetValue( HSCRIPT hScope, const char *pszKey, const char *pszValue ) { return true; }
	virtual bool SetValue( HSCRIPT hScope, const char *pszKey, const ScriptVariant_t &value ) { return true; }
	virtual bool SetValue( HSCRIPT hScope, int nIndex, const ScriptVariant_t &value ) { return true; }

	virtual void CreateTable( ScriptVariant_t &Table ) { }
	virtual bool IsTable( HSCRIPT hScope ) { return true; }
	virtual int	GetNumTableEntries( HSCRIPT hScope ) { return 0; }
	virtual int GetKeyValue( HSCRIPT hScope, int nIterator, ScriptVariant_t *pKey, ScriptVariant_t *pValue ) { return 0; }

	virtual bool GetValue( HSCRIPT hScope, const char *pszKey, ScriptVariant_t *pValue ) { return true; }
	virtual bool GetValue( HSCRIPT hScope, int nIndex, ScriptVariant_t *pValue ) { return true; }
	virtual bool GetScalarValue( HSCRIPT hScope, ScriptVariant_t *pValue ) { return true; }
	virtual void ReleaseValue( ScriptVariant_t &value ) { }

	virtual bool ClearValue( HSCRIPT hScope, const char *pszKey ) { return true; }

	virtual void CreateArray( ScriptVariant_t &pArray ) { }
	virtual bool IsArray( HSCRIPT hScope ) { return true; }
	virtual int GetArrayCount( HSCRIPT hScope ) { return 0; }
	virtual int ArrayAddToTail( HSCRIPT hScope, const ScriptVariant_t &value ) { return 0; }

	virtual void WriteState( CUtlBuffer *pBuffer ) { }
	virtual void ReadState( CUtlBuffer *pBuffer ) { }
	virtual void CollectGarbage(char const* psz, bool b) { }
	//virtual void RemoveOrphanInstances() = 0;

	virtual void DumpState() { }

	virtual void SetOutputCallback( ScriptOutputFunc_t pFunc ) { }
	virtual void SetErrorCallback( ScriptErrorFunc_t pFunc ) { }

	virtual bool RaiseException( const char *pszExceptionText ) { return true; }

	virtual HSCRIPT GetRootTable() { return INVALID_HSCRIPT; }
	virtual HSCRIPT CopyHandle( HSCRIPT hScope ) { return INVALID_HSCRIPT; }
	virtual void *GetIdentity( HSCRIPT hScope ) { return NULL; }	// SQObjectValue

	protected:
	/// interface hiding for the squirrel metamethod machinery -- don't call these directly,
	/// but instead try to use the constructor and dtor of the CSquirrelNamedSlotToGetMethodDelegate
	/// instead. That'll automatically clean up after itself when it goes out of scope.
	virtual CSquirrelMetamethodDelegateImpl *MakeSquirrelMetamethod_Get(
		HSCRIPT &hParentObject,  ///< the instance or class in which you want to register this metamethod
		const char *pszSlotName, ///< the name of the slot in the instance which will delegate to this metamethod. (eg, if you use "foo" here, then any call to instance.foo.x will result in a Get() call to your delegate with key 'x').
		ISquirrelMetamethodDelegate *pDelegate, ///< your implementation of the delegate class, which has a Get() providing dictionary semantics
		bool bDeleteDelegateWhenIAmDeleted ///< if true, DestroySquirrelMetamethod_Get will also call delete on the pDelegate stored here.
		) { return NULL; }
	/// calls delete on the given pointer, and does other important cleanup work as well.
	virtual void DestroySquirrelMetamethod_Get( CSquirrelMetamethodDelegateImpl * pMetaMethodImpl ) { }

public:
	virtual int GetKeyValue2( HSCRIPT hScope, int nIterator, ScriptVariant_t *pKey, ScriptVariant_t *pValue ) { return 0; }
};