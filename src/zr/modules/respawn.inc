/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          respawn.inc
 *  Type:          Module
 *  Description:   Respawns players back into the game.
 *
 *  Copyright (C) 2009-2010  Greyscale, Richard Helgeby
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 */

/**
 * TODO:
 * Events
 */

// Include interfaces.
#include "zr/interfaces/infection.interface"

/**
 * This module's identifier.
 */
new Module:g_moduleRespawn;

/**
 * Cvar handles.
 */
new Handle:g_hCvarRespawn;
new Handle:g_hCvarRespawnDelay;
new Handle:g_hCvarRespawnTeam;
new Handle:g_hCvarRespawnZombieIfSuicide;

/**
 * Per-client respawn timer handles.
 */
new Handle:g_hRespawnTimer[MAXPLAYERS + 1];

/**
 * Keeps track of players who suicide
 */
new bool:g_bDeathBySuicide[MAXPLAYERS + 1];

/**
 * Register this module.
 */
Respawn_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Respawn");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "respawn");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Respawns players back into the game.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleRespawn = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_OnEventsRegister",           "Respawn_OnEventsRegister");
}

/**
 * Register all events here.
 */
public Respawn_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_OnClientPutInServer",        "Respawn_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_OnClientDisconnect",         "Respawn_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_RoundEnd",                   "Respawn_RoundEnd");
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_PlayerSpawn",                "Respawn_PlayerSpawn");
    EventMgr_RegisterEvent(g_moduleRespawn, "Event_PlayerDeath",                "Respawn_PlayerDeath");
    
    #endif
}

/**
 * Plugin is loading.
 */
Respawn_OnPluginStart()
{
    // Register the module.
    Respawn_Register();
    
    // Create cvars.
    g_hCvarRespawn = Project_CreateConVar("respawn", "0", "Players will respawn after they are killed.");
    g_hCvarRespawnDelay = Project_CreateConVar("respawn_delay", "5", "Time player will wait after dying before they respawn.");
    g_hCvarRespawnTeam = Project_CreateConVar("respawn_team", "zombie", "Team to respawn player on. ['zombie' = zombie team | 'human' = human team]");
    g_hCvarRespawnZombieIfSuicide = Project_CreateConVar("respawn_zombie_if_suicide", "1", "Respawn player as a zombie if player suicides regardless of other settings.");
    
    // Create commands.
    Project_RegConsoleCmd("spawn", Respawn_Command, "Respawn a client on a given team.  Usage: <prefix>_spawn <client> <zombie|human>");
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public Respawn_OnClientPutInServer(client)
{
    g_hRespawnTimer[client] = INVALID_HANDLE;
    
    g_bDeathBySuicide[client] = false;
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public Respawn_OnClientDisconnect(client)
{
    Util_CloseHandle(g_hRespawnTimer[client]);
}

/**
 * Round has ended.
 * 
 * @param winner    The index of the winning team.
 */
public Respawn_RoundEnd(winner)
{
    for (new client = 1; client <= MaxClients; client++)
    {
        if (IsClientInGame(client))
            Util_CloseHandle(g_hRespawnTimer[client]);
    }
}

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 */
public Respawn_PlayerSpawn(client)
{
    // Reset this variable since they're alive now.
    g_bDeathBySuicide[client] = false;
    
    // Kill the timer that respawns them.
    Util_CloseHandle(g_hRespawnTimer[client]);
}

/**
 * Client has been killed.
 * 
 * @param victim    The index of the killed client.
 * @param attacker  The killer of the victim.
 * @param weapon    The weapon classname used to kill the victim. (No weapon_ prefix)
 * @param headshot  True if the death was by headshot, false if not.
 */
public Respawn_PlayerDeath(victim, attacker, const String:weapon[], bool:headshot)
{
    if (TLib_IsClientZombie(victim))
        g_bDeathBySuicide[victim] = (attacker < 0 || attacker > MaxClients);
    
    // Stop the timer just in case it's running. (it never should at this point.)
    Util_CloseHandle(g_hRespawnTimer[victim]);
    
    // Start timer to respawn them if enabled.
    if (GetConVarBool(g_hCvarRespawn))
    {
        new Float:delay = GetConVarFloat(g_hCvarRespawnDelay);
        g_hRespawnTimer[victim] = CreateTimer(delay, Respawn_Timer, victim, TIMER_FLAG_NO_MAPCHANGE);
    }
}

/**
 * Timer callback, respawns a player.
 * 
 * @param timer     The timer handle.
 * @param client    The client index.
 */    
public Action:Respawn_Timer(Handle:timer, any:client)
{
    g_hRespawnTimer[client] = INVALID_HANDLE;
    
    if (!IsClientInGame(client))
        return;
    
    if (IsPlayerAlive(client))
        return;
    
    if (!TLib_IsClientPlaying(client))
        return;
    
    // If the client suicided and the cvar is enabled, they will be respawned as zombie.
    if (GetConVarBool(g_hCvarRespawnZombieIfSuicide) && g_bDeathBySuicide[client])
        TLib_SpawnOnTeam(client, VTeam_Zombie);
    else
    {
        // Update the client's virtual team with the one defined in this cvar.
        decl String:respawn_team[32];
        GetConVarString(g_hCvarRespawnTeam, respawn_team, sizeof(respawn_team));
        new VTeam:team = TLib_VTeamStringToIndex(respawn_team);
        if (team == VTeam_Invalid)
        {
            LogMgr_Print(g_moduleRespawn, LogType_Fatal_Module, "Cvar zr_respawn_team", "Invalid value (%s) set in cvar,", respawn_team);
            return;
        }
        TLib_SpawnOnTeam(client, team);
    }
}

/**
 * Command callback: zr_(re)spawn
 * Spawn a client on a given team.
 * 
 * @param client    The client index. 
 * @param argc      The number of arguments that the server sent with the command.
 */
public Action:Respawn_Command(client, argc)
{
    // Check if the this core module is disabled, if so then don't do anything with it.
    if (ModuleMgr_IsDisabled(g_moduleRespawn))
        return Plugin_Continue;
    
    // Check if the client has permission to use this.
    if (!AccessMgr_HasAccess(client, g_moduleRespawn))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "No access to command");
        return Plugin_Handled;
    }
    
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "Respawn command syntax", PROJECT_CMD_PREFIX);
        return Plugin_Handled;
    }
    
    decl String:target[MAX_NAME_LENGTH], String:targetname[MAX_NAME_LENGTH];
    new targets[MAXPLAYERS], bool:tn_is_ml, result;
    
    // Get targetname.
    GetCmdArg(1, target, sizeof(target));
    
    // Find a target.
    result = ProcessTargetString(target, client, targets, sizeof(targets), COMMAND_FILTER_DEAD, targetname, sizeof(targetname), tn_is_ml);
    
    // Check if there was a problem finding a client.
    if (result <= 0)
    {
        TransMgr_ReplyToTargetError(client, result);
        return Plugin_Handled;
    }
    
    // Check optional parameters.
    decl String:strTeam[32];
    GetCmdArg(2, strTeam, sizeof(strTeam));
    new VTeam:team = VTeam_Invalid;
    if (strTeam[0])
    {
        team = VTeam:TLib_VTeamStringToIndex(strTeam);
        if (team == VTeam_Invalid)
        {
            TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "Respawn command invalid team");
            return Plugin_Handled;
        }
    }
    
    new VTeam:respawnteam;
    for (new tindex = 0; tindex < result; tindex++)
    {
        // Set the variable to the team this client should respawn on.
        if (team == VTeam_Zombie || team == VTeam_Human)
            respawnteam = team;
        else
            respawnteam = TLib_GetClientTeam(targets[tindex]);
        
        if (respawnteam == VTeam_Zombie || respawnteam == VTeam_Human)
        {
            TLib_SpawnOnTeam(targets[tindex], respawnteam);
            if (result == 1)
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "Respawn command success", targets[tindex]);
        }
        else
        {
            if(result == 1)
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "Respawn command fail", targets[tindex]);
        }
    }
    
    return Plugin_Handled;
}
