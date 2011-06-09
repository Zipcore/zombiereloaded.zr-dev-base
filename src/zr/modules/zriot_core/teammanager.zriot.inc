/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          teammanager.zriot.inc
 *  Type:          Module
 *  Description:   Manages zombie-based teams for ZRiot gameplay.
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
 * This module's identifier.
 */
new Module:g_moduleZRiotTM;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRiotTM_GetIdentifier() { return g_moduleZRiotTM; }

/**
 * Cvar handles.
 */
new Handle:g_hCvarZRiotZombieTeam;

/**
 * Register this module.
 */
ZRiotTM_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZRiot Team Manager");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zriotteammanager");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Manages zombie-based teams for ZRiot gameplay.");
    moduledata[ModuleData_Dependencies][0] = ZRiot_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRiotTM = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_OnEventsRegister",           "ZRiotTM_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRiotTM_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_OnConfigsExecuted",          "ZRiotTM_OnConfigsExecuted");
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_OnClientPutInServer",        "ZRiotTM_OnClientPutInServer");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_RoundStart",                 "ZRiotTM_RoundStart");
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_PlayerTeamPre",              "ZRiotTM_PlayerTeamPre");
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_PlayerTeam",                 "ZRiotTM_PlayerTeam");
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_PlayerSpawn",                "ZRiotTM_PlayerSpawn");
    
    #endif
    
    // Register custom events.
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_OnClientInfected",           "ZRiotTM_OnClientInfected");
    EventMgr_RegisterEvent(g_moduleZRiotTM, "Event_OnClientHuman",              "ZRiotTM_OnClientHuman");
}

/**
 * Plugin is loading.
 */
ZRiotTM_OnPluginStart()
{
    // Register the module.
    ZRiotTM_Register();
    
    // Create cvars.
    g_hCvarZRiotZombieTeam  = Project_CreateConVar("zombie_team", "t", "Define the game team that will be the zombie team.  Opposite team will be the human team.  ['t' = Terrorists | 'ct' = Counter-Terrorists]");
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public ZRiotTM_OnConfigsExecuted()
{
    ZRiotTM_UpdateZombieTeam();
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public ZRiotTM_OnClientPutInServer(client)
{
    // Initialize the client's team.
    TLib_SetClientTeam(client, VTeam_Unassigned);
}

/**
 * Client has been infected.
 * 
 * @param client        The infected client.
 * @param attacker      The zombie that infected the client.
 * @param mzombie       True if the client has been infected as a mother zombie.
 */
public ZRiotTM_OnClientInfected(client, attacker, bool:mzombie)
{
    Util_SwitchTeam(client, TLib_GetGameTeamIndex(VTeam_Zombie));
}

/**
 * Client has turned back to human.
 * 
 * @param client        The client that became human.
 */
public ZRiotTM_OnClientHuman(client)
{
    Util_SwitchTeam(client, TLib_GetGameTeamIndex(VTeam_Human));
}

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 */
public ZRiotTM_RoundStart()
{
    // Update the zombie team every round start.
    // This shouldn't be updated real-time with the cvar, or else zombies would be on both teams during the round.
    ZRiotTM_UpdateZombieTeam();
}


/**
 * Client has joined a team. (pre)
 * 
 * @param client        The client index.
 * @param team          The client's new team.
 * @param oldteam       The team the client is switching from.
 * @param disconnect    True if the client is disconnecting, false if switching to a team.
 * 
 * @return              Plugin_Handled to stop the event from being broadcasted to clients.  Plugin_Continue to let it continue like normal.  
 */
public Action:ZRiotTM_PlayerTeamPre(client, team, oldteam, bool:disconnect)
{
    // Prevent this from being broadcasted to clients.
    return Plugin_Handled;
}

/**
 * Client has joined a team.
 * 
 * @param client        The client index.
 * @param team          The client's new team.
 * @param oldteam       The team the client is switching from.
 * @param disconnect    True if the client is disconnecting, false if switching to a team.
 */
public ZRiotTM_PlayerTeam(client, team, oldteam, bool:disconnect)
{
    if (team == TEAM_1)
        TLib_SetClientTeam(client, VTeam_Spectator);
    // If client isn't already in the game, then assign them to human by default.
    else if (team > TEAM_1 && !TLib_IsClientPlaying(client))
        TLib_SetClientTeam(client, VTeam_Human);
}

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 * 
 */
public ZRiotTM_PlayerSpawn(client)
{
    // player_spawn is fired during the connection process, this is how we filter it out.
    if (!Util_IsClientOnTeam(client))
        return;
    
    // Move client the right team for their virtual team if there are zombies present.
    if (g_bZombiesPresent)
        TLib_UpdateClientTeam(client);
}

#endif

/**
 * Update the game teams associated with the virtual teams, as set by cvar.
 */
static stock ZRiotTM_UpdateZombieTeam()
{
    decl String:strZombieTeam[32];
    GetConVarString(g_hCvarZRiotZombieTeam, strZombieTeam, sizeof(strZombieTeam));
    new iZombieTeamIndex = TLib_GTeamStringToIndex(strZombieTeam);
    if (iZombieTeamIndex > 1)
    {
        // Associate the zombie virtual team with the given value.
        TLib_SetGameTeamIndex(VTeam_Zombie, iZombieTeamIndex);
        
        // For humans, use the opposite team as the one given for zombies.
        TLib_SetGameTeamIndex(VTeam_Human, (iZombieTeamIndex == 2) ? 3 : 2);
    }
    else
    {
        LogMgr_Print(g_moduleZRiotTM, LogType_Error, "Cvar Validation", "Invalid team name (%s) in cvar \"zr_zombie_team\".  Check the description in zombiereloaded.cfg.  Assuming zombies are terrorist team.", strZombieTeam);
        TLib_SetGameTeamIndex(VTeam_Zombie, 2);
        TLib_SetGameTeamIndex(VTeam_Human, 3);
    }
}
