#include <sourcemod>
#include <vscript>

public Plugin myinfo = {
	name = "VScript Test",
	author = "ProjectSky",
	description = "test suite for all VScript API features",
	version = "1.0.0",
	url = "https://github.com/ProjectSky/sm-ext-vscript"
};

// Global test counters
int g_TotalTests = 0;
int g_PassedTests = 0;
int g_FailedTests = 0;

public void OnPluginStart() {
	RegAdminCmd("test_vscript", Command_RunTests, ADMFLAG_ROOT, "Run all VScript tests");
}

// Helper function to record test result
void TestResult(const char[] testName, bool passed) {
	g_TotalTests++;
	if (passed) {
		g_PassedTests++;
		PrintToServer("%s: PASS", testName);
	} else {
		g_FailedTests++;
		PrintToServer("%s: FAIL", testName);
	}
}

// Helper function with value display
void TestResultWithValue(const char[] testName, bool passed, const char[] format, any ...) {
	g_TotalTests++;
	char buffer[256];
	VFormat(buffer, sizeof(buffer), format, 4);

	if (passed) {
		g_PassedTests++;
		PrintToServer("%s: PASS %s", testName, buffer);
	} else {
		g_FailedTests++;
		PrintToServer("%s: FAIL %s", testName, buffer);
	}
}

// ============================================================================
// Main Test Runner
// ============================================================================

Action Command_RunTests(int client, int args) {
	PrintToServer("========================================");
	PrintToServer("Vscript Test Suite");
	PrintToServer("========================================");

	// Reset counters
	g_TotalTests = 0;
	g_PassedTests = 0;
	g_FailedTests = 0;

	Test_Base();
	Test_Scope();
	Test_Table();
	Test_Array();
	Test_Function();
	Test_Variant();

	// Print results
	PrintToServer("========================================");
	PrintToServer("Vscript Test Suite Results");
	PrintToServer("========================================");
	PrintToServer("Total Tests: %d", g_TotalTests);
	PrintToServer("Passed: %d", g_PassedTests);
	PrintToServer("Failed: %d", g_FailedTests);

	if (g_TotalTests > 0)
	{
		float successRate = (float(g_PassedTests) / float(g_TotalTests)) * 100.0;
		PrintToServer("Success Rate: %.2f%%", successRate);
	}

	PrintToServer("========================================");

	return Plugin_Handled;
}

// ============================================================================
// VScript Base Class Tests
// ============================================================================

void Test_Base() {
	PrintToServer("\n[VScript Base] Testing base class methods...");

	// Test IsValid
	VScriptScope root = VScriptScope.GetRoot();
	TestResult("[VScript Base] IsValid", root.IsValid);

	// Test IsArray
	VScriptArray arr = new VScriptArray();
	TestResult("[VScript Base] IsArray on array", arr.IsArray);
	TestResult("[VScript Base] IsArray on scope", !root.IsArray);

	// Test IsTable
	VScriptTable tbl = new VScriptTable();
	TestResult("[VScript Base] IsTable on table", tbl.IsTable);
	TestResult("[VScript Base] IsTable on scope", !root.IsTable);

	// Test Compile
	VScriptFunction func = VScript.Compile("return function(x) { return x * 2; }");
	if (func) {
		ScriptVariant arg = ScriptVariant.FromInt(10);
		ScriptVariant result = func.CallWithArgs(null, arg);
		TestResultWithValue("[VScript Base] Compile", result.IsValid && result.Int == 20, "(result=%d)", result.Int);
		delete result;
		delete arg;
		delete func;
	} else {
		TestResult("[VScript Base] Compile", false);
	}

	// Test Run
	bool runSuccess = VScript.Run("x <- 42");
	TestResult("[VScript Base] Run", runSuccess);

	// Cleanup global variables
	root.ClearKey("x");

	delete tbl;
	delete arr;
	delete root;
}

// ============================================================================
// VScriptScope Tests
// ============================================================================

void Test_Scope() {
	PrintToServer("\n[VScriptScope] Testing scope methods...");

	VScriptScope root = VScriptScope.GetRoot();

	// Test constructor with parent
	VScriptScope childScope = new VScriptScope("ChildScope", root);
	childScope.SetInt("child_value", 42);
	TestResult("[VScriptScope] Constructor with parent", childScope.GetInt("child_value") == 42);
	delete childScope;

	// Test Set/Get methods
	root.SetInt("test_int", 123);
	TestResult("[VScriptScope] SetInt/GetInt", root.GetInt("test_int") == 123);

	root.SetFloat("test_float", 3.14);
	TestResult("[VScriptScope] SetFloat/GetFloat", root.GetFloat("test_float") == 3.14);

	root.SetBool("test_bool", true);
	TestResult("[VScriptScope] SetBool/GetBool", root.GetBool("test_bool"));

	// Test SetValue/GetValue
	ScriptVariant setValue = ScriptVariant.FromInt(888);
	root.SetValue("test_value", setValue);
	ScriptVariant getValue = root.GetValue("test_value");
	TestResult("[VScriptScope] SetValue/GetValue", getValue.IsValid && getValue.Int == 888);
	delete getValue;
	delete setValue;

	root.SetString("test_string", "hello");
	char buffer[64];
	root.GetString("test_string", buffer, sizeof(buffer));
	TestResultWithValue("[VScriptScope] SetString/GetString", StrEqual(buffer, "hello"), "(%s)", buffer);

	float vec[3] = {1.0, 2.0, 3.0};
	root.SetVector("test_vector", vec);
	float vecOut[3];
	root.GetVector("test_vector", vecOut);
	TestResult("[VScriptScope] SetVector/GetVector", (vecOut[0] == 1.0 && vecOut[1] == 2.0 && vecOut[2] == 3.0));

	// Test SetTable/GetTable
	VScriptTable tbl = new VScriptTable();
	tbl.SetInt("key", 999);
	root.SetTable("test_table", tbl);
	VScriptTable tblOut = root.GetTable("test_table");
	TestResult("[VScriptScope] SetTable/GetTable", tblOut && tblOut.GetInt("key") == 999);
	delete tblOut;
	delete tbl;

	// Test SetArray/GetArray
	VScriptArray arr = new VScriptArray();
	arr.PushInt(777);
	root.SetArray("test_array", arr);
	VScriptArray arrOut = root.GetArray("test_array");
	TestResult("[VScriptScope] SetArray/GetArray", arrOut && arrOut.GetInt(0) == 777);
	delete arrOut;
	delete arr;

	// Test GetBool (ensure it works)
	root.SetBool("test_bool_get", false);
	TestResult("[VScriptScope] GetBool", !root.GetBool("test_bool_get"));

	// Test HasKey/ClearKey
	TestResult("[VScriptScope] HasKey", root.HasKey("test_int"));
	root.ClearKey("test_int");
	TestResult("[VScriptScope] ClearKey", !root.HasKey("test_int"));

	// Test Execute
	VScript.Run("function TestFunc(x) { return x + 100; }");
	ScriptVariant execResult = root.Execute("return TestFunc(50)");
	TestResultWithValue("[VScriptScope] Execute", execResult.IsValid && execResult.Int == 150, "(result=%d)", execResult.Int);
	delete execResult;

	// Test LookupFunction
	VScriptFunction testFunc = root.LookupFunction("TestFunc");
	if (testFunc) {
		ScriptVariant arg = ScriptVariant.FromInt(25);
		ScriptVariant result = testFunc.CallWithArgs(null, arg);
		TestResultWithValue("[VScriptScope] LookupFunction", result.IsValid && result.Int == 125, "(result=%d)", result.Int);
		delete result;
		delete arg;
		delete testFunc;
	} else {
		TestResult("[VScriptScope] LookupFunction", false);
	}

	// Cleanup global variables
	root.ClearKey("TestFunc");

	delete root;
}

// ============================================================================
// VScriptTable Tests
// ============================================================================

void Test_Table() {
	PrintToServer("\n[VScriptTable] Testing table methods...");

	VScriptTable tbl = new VScriptTable();

	// Test Set/Get by key
	tbl.SetInt("int_key", 456);
	TestResult("[VScriptTable] SetInt/GetInt", tbl.GetInt("int_key") == 456);

	tbl.SetFloat("float_key", 2.71);
	TestResult("[VScriptTable] SetFloat/GetFloat", tbl.GetFloat("float_key") == 2.71);

	tbl.SetBool("bool_key", false);
	TestResult("[VScriptTable] SetBool/GetBool", !tbl.GetBool("bool_key"));

	tbl.SetString("string_key", "world");
	char buffer[64];
	tbl.GetString("string_key", buffer, sizeof(buffer));
	TestResultWithValue("[VScriptTable] SetString/GetString", StrEqual(buffer, "world"), "(%s)", buffer);

	// Test Set/Get by index (all types)
	tbl.SetIntAt(0, 111);
	TestResult("[VScriptTable] SetIntAt/GetIntAt", tbl.GetIntAt(0) == 111);

	tbl.SetFloatAt(1, 2.22);
	TestResult("[VScriptTable] SetFloatAt/GetFloatAt", tbl.GetFloatAt(1) == 2.22);

	tbl.SetBoolAt(2, true);
	TestResult("[VScriptTable] SetBoolAt/GetBoolAt", tbl.GetBoolAt(2));

	tbl.SetStringAt(3, "indexed");
	tbl.GetStringAt(3, buffer, sizeof(buffer));
	TestResult("[VScriptTable] SetStringAt/GetStringAt", StrEqual(buffer, "indexed"));

	float vec[3] = {4.0, 5.0, 6.0};
	tbl.SetVectorAt(4, vec);
	float vecOut[3];
	tbl.GetVectorAt(4, vecOut);
	TestResult("[VScriptTable] SetVectorAt/GetVectorAt", (vecOut[0] == 4.0 && vecOut[1] == 5.0 && vecOut[2] == 6.0));

	// Test SetValue/GetValue by key
	ScriptVariant setValue = ScriptVariant.FromInt(555);
	tbl.SetValue("value_key", setValue);
	ScriptVariant getValue = tbl.GetValue("value_key");
	TestResult("[VScriptTable] SetValue/GetValue", getValue.IsValid && getValue.Int == 555);
	delete getValue;
	delete setValue;

	// Test SetValueAt/GetValueAt by index
	ScriptVariant setValueAt = ScriptVariant.FromInt(666);
	tbl.SetValueAt(5, setValueAt);
	ScriptVariant getValueAt = tbl.GetValueAt(5);
	TestResult("[VScriptTable] SetValueAt/GetValueAt", getValueAt.IsValid && getValueAt.Int == 666);
	delete getValueAt;
	delete setValueAt;

	// Test Length
	int tableLen = tbl.Length;
	TestResultWithValue("[VScriptTable] Length", tableLen >= 0, "(length=%d)", tableLen);

	// Test HasKey/ClearKey
	TestResult("[VScriptTable] HasKey", tbl.HasKey("int_key"));
	tbl.ClearKey("int_key");
	TestResult("[VScriptTable] ClearKey", !tbl.HasKey("int_key"));

	// Test GetKeys
	tbl.SetInt("a", 1);
	tbl.SetInt("b", 2);
	tbl.SetInt("c", 3);
	VScriptArray keys = tbl.GetKeys();
	TestResultWithValue("[VScriptTable] GetKeys", keys && keys.Length >= 3, "(length=%d)", keys ? keys.Length : 0);
	delete keys;

	// Test GetKeyValue (iterator)
	int iterator = 0;
	int keyCount = 0;
	char keyBuffer[64];
	ScriptVariant valueVar;
	while ((iterator = tbl.GetKeyValue(iterator, keyBuffer, sizeof(keyBuffer), valueVar)) != -1) {
		keyCount++;
		if (valueVar) delete valueVar;
	}
	TestResultWithValue("[VScriptTable] GetKeyValue iterator", keyCount >= 3, "(found %d keys)", keyCount);

	// Test Clone
	VScriptTable clone = tbl.Clone();
	TestResult("[VScriptTable] Clone", clone && clone.GetInt("a") == 1);
	delete clone;

	// Test Clear
	tbl.Clear();
	TestResult("[VScriptTable] Clear", tbl.Length == 0);

	// Test LookupFunction
	VScriptScope root = VScriptScope.GetRoot();
	VScript.Run("TestTable <- { MyFunc = function(x) { return x * 3; } }");
	VScriptTable testTbl = root.GetTable("TestTable");
	if (testTbl) {
		VScriptFunction func = testTbl.LookupFunction("MyFunc");
		if (func) {
			ScriptVariant arg = ScriptVariant.FromInt(7);
			ScriptVariant result = func.CallWithArgs(null, arg);
			TestResultWithValue("[VScriptTable] LookupFunction", result.IsValid && result.Int == 21, "(result=%d)", result.Int);
			delete result;
			delete arg;
			delete func;
		} else {
			TestResult("[VScriptTable] LookupFunction", false);
		}
		delete testTbl;
	}

	// Cleanup global variables
	root.ClearKey("TestTable");
	delete root;

	delete tbl;
}

// ============================================================================
// VScriptArray Tests
// ============================================================================

void Test_Array() {
	PrintToServer("\n[VScriptArray] Testing array methods...");

	VScriptArray arr = new VScriptArray();

	// Test Push methods
	arr.PushInt(10);
	arr.PushFloat(3.14);
	arr.PushBool(true);
	arr.PushString("test");
	float vec[3] = {1.0, 2.0, 3.0};
	arr.PushVector(vec);

	// Test PushValue
	ScriptVariant pushVal = ScriptVariant.FromInt(99);
	arr.PushValue(pushVal);
	delete pushVal;

	TestResultWithValue("[VScriptArray] Length after pushes", arr.Length == 6, "(length=%d)", arr.Length);

	// Test Get methods
	TestResult("[VScriptArray] GetInt", arr.GetInt(0) == 10);
	TestResult("[VScriptArray] GetFloat", arr.GetFloat(1) == 3.14);
	TestResult("[VScriptArray] GetBool", arr.GetBool(2));

	char buffer[64];
	arr.GetString(3, buffer, sizeof(buffer));
	TestResultWithValue("[VScriptArray] GetString", StrEqual(buffer, "test"), "(%s)", buffer);

	float vecOut[3];
	arr.GetVector(4, vecOut);
	TestResult("[VScriptArray] GetVector", (vecOut[0] == 1.0 && vecOut[1] == 2.0 && vecOut[2] == 3.0));

	// Test GetValue
	ScriptVariant getVal = arr.GetValue(5);
	TestResult("[VScriptArray] GetValue", getVal.IsValid && getVal.Int == 99);
	delete getVal;

	// Test Set methods
	arr.SetInt(0, 20);
	TestResult("[VScriptArray] SetInt", arr.GetInt(0) == 20);

	// Test SetValue
	ScriptVariant setVal = ScriptVariant.FromInt(88);
	arr.SetValue(5, setVal);
	TestResult("[VScriptArray] SetValue", arr.GetInt(5) == 88);
	delete setVal;

	// Test Insert
	ScriptVariant insertVal = ScriptVariant.FromInt(999);
	arr.Insert(0, insertVal);
	TestResultWithValue("[VScriptArray] Insert", arr.GetInt(0) == 999, "(value=%d, length=%d)", arr.GetInt(0), arr.Length);
	delete insertVal;

	// Test Contains
	ScriptVariant searchVal = ScriptVariant.FromInt(20);
	TestResult("[VScriptArray] Contains", arr.Contains(searchVal));

	// Test IndexOf
	int idx = arr.IndexOf(searchVal);
	TestResultWithValue("[VScriptArray] IndexOf", idx >= 0, "(index=%d)", idx);
	delete searchVal;

	// Test Pop
	int lengthBefore = arr.Length;
	ScriptVariant popped = arr.Pop();
	TestResultWithValue("[VScriptArray] Pop", arr.Length == lengthBefore - 1, "(length: %d -> %d)", lengthBefore, arr.Length);
	delete popped;

	// Test Remove
	lengthBefore = arr.Length;
	arr.Remove(0);
	TestResultWithValue("[VScriptArray] Remove", arr.Length == lengthBefore - 1, "(length: %d -> %d)", lengthBefore, arr.Length);

	// Test Clear
	arr.Clear();
	TestResultWithValue("[VScriptArray] Clear", arr.Length == 0, "(length=%d)", arr.Length);

	delete arr;
}

// ============================================================================
// VScriptFunction Tests
// ============================================================================

void Test_Function() {
	PrintToServer("\n[VScriptFunction] Testing function methods...");

	VScriptScope root = VScriptScope.GetRoot();

	// Define test functions
	VScript.Run("function NoArgs() { return 42; }");
	VScript.Run("function WithArgs(a, b) { return a + b; }");
	VScript.Run("function WithScope() { return this.value; }");

	// Test Call (no arguments)
	VScriptFunction noArgsFunc = root.LookupFunction("NoArgs");
	if (noArgsFunc) {
		ScriptVariant result = noArgsFunc.Call(null);
		TestResultWithValue("[VScriptFunction] Call (no args)", result.IsValid && result.Int == 42, "(result=%d)", result.Int);
		delete result;
		delete noArgsFunc;
	} else {
		TestResult("[VScriptFunction] Call (no args)", false);
	}

	// Test CallWithArgs
	VScriptFunction withArgsFunc = root.LookupFunction("WithArgs");
	if (withArgsFunc) {
		ScriptVariant arg1 = ScriptVariant.FromInt(10);
		ScriptVariant arg2 = ScriptVariant.FromInt(32);
		ScriptVariant result = withArgsFunc.CallWithArgs(null, arg1, arg2);
		TestResultWithValue("[VScriptFunction] CallWithArgs", result.IsValid && result.Int == 42, "(result=%d)", result.Int);
		delete result;
		delete arg2;
		delete arg1;
		delete withArgsFunc;
	} else {
		TestResult("[VScriptFunction] CallWithArgs", false);
	}

	// Test Call with scope
	VScriptTable scopeTable = new VScriptTable();
	scopeTable.SetInt("value", 100);
	VScriptFunction withScopeFunc = root.LookupFunction("WithScope");
	if (withScopeFunc) {
		ScriptVariant result = withScopeFunc.Call(scopeTable);
		TestResultWithValue("[VScriptFunction] Call (with scope)", result.IsValid && result.Int == 100, "(result=%d)", result.Int);
		delete result;
		delete withScopeFunc;
	} else {
		TestResult("[VScriptFunction] Call (with scope)", false);
	}
	delete scopeTable;

	// Test compiled function
	VScriptFunction compiled = VScript.Compile("return function(x) { return x * 2; }");
	if (compiled) {
		ScriptVariant arg = ScriptVariant.FromInt(21);
		ScriptVariant result = compiled.CallWithArgs(null, arg);
		TestResultWithValue("[VScriptFunction] Compiled function", result.IsValid && result.Int == 42, "(result=%d)", result.Int);
		delete result;
		delete arg;
		delete compiled;
	} else {
		TestResult("[VScriptFunction] Compiled function", false);
	}

	// Cleanup global variables
	root.ClearKey("NoArgs");
	root.ClearKey("WithArgs");
	root.ClearKey("WithScope");

	delete root;
}

// ============================================================================
// ScriptVariant Tests
// ============================================================================

void Test_Variant() {
	PrintToServer("\n[ScriptVariant] Testing variant methods...");

	// Test FromInt
	ScriptVariant intVar = ScriptVariant.FromInt(42);
	TestResultWithValue("[ScriptVariant] FromInt", intVar.Type == FIELD_INTEGER, "(type=%d, value=%d)", intVar.Type, intVar.Int);
	delete intVar;

	// Test FromFloat
	ScriptVariant floatVar = ScriptVariant.FromFloat(3.14);
	TestResultWithValue("[ScriptVariant] FromFloat", floatVar.Type == FIELD_FLOAT, "(type=%d, value=%.2f)", floatVar.Type, floatVar.Float);
	delete floatVar;

	// Test FromBool
	ScriptVariant boolVar = ScriptVariant.FromBool(true);
	TestResultWithValue("[ScriptVariant] FromBool", boolVar.Type == FIELD_BOOLEAN, "(type=%d, value=%d)", boolVar.Type, boolVar.Bool);
	delete boolVar;

	// Test FromString
	ScriptVariant stringVar = ScriptVariant.FromString("hello");
	char buffer[64];
	stringVar.GetString(buffer, sizeof(buffer));
	TestResultWithValue("[ScriptVariant] FromString", stringVar.Type == FIELD_CSTRING, "(type=%d, value=%s)", stringVar.Type, buffer);
	delete stringVar;

	// Test FromVector
	float vec[3] = {1.0, 2.0, 3.0};
	ScriptVariant vectorVar = ScriptVariant.FromVector(vec);
	float vecOut[3];
	vectorVar.GetVector(vecOut);
	TestResultWithValue("[ScriptVariant] FromVector", vectorVar.Type == FIELD_VECTOR, "(type=%d)", vectorVar.Type);
	delete vectorVar;

	// Test FromTable
	VScriptTable tbl = new VScriptTable();
	tbl.SetInt("key", 123);
	ScriptVariant tableVar = ScriptVariant.FromTable(tbl);
	VScriptTable tblOut = tableVar.Table;
	TestResultWithValue("[ScriptVariant] FromTable", tableVar.Type == FIELD_HSCRIPT && tblOut, "(type=%d, value=%d)", tableVar.Type, tblOut ? tblOut.GetInt("key") : 0);
	delete tblOut;
	delete tableVar;
	delete tbl;

	// Test FromArray
	VScriptArray arr = new VScriptArray();
	arr.PushInt(456);
	ScriptVariant arrayVar = ScriptVariant.FromArray(arr);
	VScriptArray arrOut = arrayVar.Array;
	TestResultWithValue("[ScriptVariant] FromArray", arrayVar.Type == FIELD_HSCRIPT && arrOut, "(type=%d, value=%d)", arrayVar.Type, arrOut ? arrOut.GetInt(0) : 0);
	delete arrOut;
	delete arrayVar;
	delete arr;

	// Test Null
	ScriptVariant nullVar = ScriptVariant.Null();
	TestResultWithValue("[ScriptVariant] Null", nullVar.Type == FIELD_VOID && nullVar.IsNull, "(type=%d, IsNull=%d)", nullVar.Type, nullVar.IsNull);
	delete nullVar;

	// Test FromEntity (find a valid client)
	int testClient = 0;
	for (int i = 1; i <= MaxClients; i++) {
		if (IsClientInGame(i)) {
			testClient = i;
			break;
		}
	}

	if (testClient > 0) {
		ScriptVariant entityVar = ScriptVariant.FromEntity(testClient);
		if (entityVar) {
			int entityIdx = entityVar.ToEntity();
			TestResultWithValue("[ScriptVariant] FromEntity/ToEntity", entityIdx == testClient, "(client=%d, result=%d)", testClient, entityIdx);
			delete entityVar;
		} else {
			TestResult("[ScriptVariant] FromEntity/ToEntity", false);
		}
	} else {
		PrintToServer("[ScriptVariant] FromEntity/ToEntity: SKIP (no client)");
	}

	// Test Function property - Execute returns a function
	VScriptScope root = VScriptScope.GetRoot();
	VScript.Run("function TestFuncProp() { return 10; }");
	ScriptVariant funcVar = root.Execute("return TestFuncProp");
	if (funcVar) {
		VScriptFunction funcOut = funcVar.Function;
		TestResult("[ScriptVariant] Function property", funcOut != null);

		// Test function execution
		if (funcOut) {
			ScriptVariant result = funcOut.Call();
			TestResultWithValue("[ScriptVariant] Function execution", result.IsValid && result.Int == 10, "(result=%d)", result.Int);
			if (result) delete result;
			delete funcOut;
		}

		delete funcVar;
	}

	// Cleanup global variables
	root.ClearKey("TestFuncProp");
	delete root;
}
