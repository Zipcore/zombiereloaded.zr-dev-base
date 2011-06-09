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
 * Respawn command
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
    g_hCvarRespawn = CreateConVar("zr_respawn", "0", "Players will respawn after they are killed.");
    g_hCvarRespawnDelay = CreateConVar("zr_respawn_delay", "5", "Time player will wait after dying before they respawn.");
    g_hCvarRespawnTeam = CreateConVar("zr_respawn_team", "zombie", "Team to respawn player on. ['zombie' = zombie team | 'human' = human team]");
    g_hCvarRespawnZombieIfSuicide = CreateConVar("zr_respawn_zombie_if_suicide", "1", "Respawn player as a zombie if player suicides regardless of other settings.");
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
 */
public Respawn_RoundEnd()
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
    {
        g_bDeathBySuicide[client] = false;
        TLib_SetClientTeam(client, VTeam_Zombie);
    }
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
        TLib_SetClientTeam(client, team);
    }
    TLib_UpdateClientTeam(client); // Switch them to the right team and then spawn.
    CS_RespawnPlayer(client);
}
