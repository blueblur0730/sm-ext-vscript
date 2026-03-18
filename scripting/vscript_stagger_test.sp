#include <sourcemod>
#include <sdktools>
#include <vscript_ext>

public Plugin myinfo = {
	name = "VScript Stagger Test",
	author = "ProjectSky",
	description = "Test CTerrorPlayerAPI.Stagger on all alive survivors",
	version = "1.0.0",
	url = "https://github.com/ProjectSky/sm-ext-vscript"
};

public void OnPluginStart() {
	RegAdminCmd("sm_test_stagger", Cmd_TestStagger, ADMFLAG_ROOT, "Test Stagger on all alive survivors");
}

Action Cmd_TestStagger(int client, int args) {
	VScriptScope root = VScriptScope.GetRoot();

	// Get CTerrorPlayerAPI wrapper table
	VScriptTable terrorAPI = root.GetTable("CTerrorPlayerAPI");
	if (!terrorAPI) {
		PrintToServer("Failed to get CTerrorPlayerAPI");
		delete root;
		return Plugin_Handled;
	}

	// Lookup Stagger function from the API wrapper
	VScriptFunction staggerFunc = terrorAPI.LookupFunction("Stagger");
	if (!staggerFunc) {
		PrintToServer("Failed to lookup Stagger function");
		delete terrorAPI;
		delete root;
		return Plugin_Handled;
	}

	int staggerCount = 0;

	for (int i = 1; i <= MaxClients; i++) {
		if (!IsClientInGame(i)) continue;
		if (GetClientTeam(i) != 2) continue;  // Team 2 = Survivors
		if (!IsPlayerAlive(i)) continue;

		// Get player position and create stagger position slightly in front
		float pos[3], angles[3], fwd[3], staggerPos[3];
		GetClientAbsOrigin(i, pos);
		GetClientEyeAngles(i, angles);
		GetAngleVectors(angles, fwd, NULL_VECTOR, NULL_VECTOR);

		// Stagger position 50 units in front of player
		staggerPos[0] = pos[0] + fwd[0] * 50.0;
		staggerPos[1] = pos[1] + fwd[1] * 50.0;
		staggerPos[2] = pos[2] + fwd[2] * 50.0;

		// Get player instance
		ScriptVariant playerInstance = ScriptVariant.FromEntity(i);
		if (!playerInstance) continue;

		// Create position variant
		ScriptVariant posVar = ScriptVariant.FromVector(staggerPos);

		// Call CTerrorPlayerAPI.Stagger(entity, position)
		staggerFunc.CallWithArgs(null, playerInstance, posVar);

		delete posVar;
		delete playerInstance;
		staggerCount++;
	}

	delete staggerFunc;
	delete terrorAPI;
	delete root;

	PrintToServer("Staggered %d survivor(s)", staggerCount);
	if (client > 0) {
		ReplyToCommand(client, "[VScript] Staggered %d survivor(s)", staggerCount);
	}

	return Plugin_Handled;
}
