/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          teammanager.zr.inc
 *  Type:          Module
 *  Description:   Manages zombie-based teams for classic zombie gameplay.
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
new Module:g_moduleZRTM;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRTM_GetIdentifier() { return g_moduleZRTM; }

/**
 * Register this module.
 */
ZRTM_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "ZR Team Manager");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zrteammanager");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Manages zombie-based teams for classic zombie gameplay.");
    moduledata[ModuleData_Dependencies][0] = ZR_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRTM = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_OnEventsRegister",          "ZRTM_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRTM_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_OnClientPutInServer",       "ZRTM_OnClientPutInServer");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_RoundEnd",                  "ZRTM_RoundEnd");
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_PlayerTeamPre",             "ZRTM_PlayerTeamPre");
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_PlayerTeam",                "ZRTM_PlayerTeam");
    
    #endif
    
    // Register custom events.
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_OnClientInfect",            "ZRTM_OnClientInfect");
    EventMgr_RegisterEvent(g_moduleZRTM, "Event_OnClientHuman",             "ZRTM_OnClientHuman");
}

/**
 * Plugin is loading.
 */
ZRTM_OnPluginStart()
{
    // Register the module.
    ZRTM_Register();
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public ZRTM_OnClientPutInServer(client)
{
    // Initialize the client's team.
    TLib_SetClientTeam(client, ZTeam_Unassigned);
}

/**
 * Client has been infected.
 * 
 * @param client        The infected client.
 * @param attacker      The zombie that infected the client.
 * @param mzombie       True if the client has been infected as a mother zombie.
 */
public ZRTM_OnClientInfect(client, attacker, bool:mzombie)
{
    #if defined PROJECT_GAME_CSS
        // If a mother zombie is being infected then update all clients' teams.
        // If not then just switch them to T.
        if (mzombie)
            ZRTM_UpdateClientTeams();
        else
            CS_SwitchTeam(client, TEAM_2);
    #endif
}

/**
 * Client has turned back to human.
 * 
 * @param client        The client that became human.
 */
public ZRTM_OnClientHuman(client)
{
    #if defined PROJECT_GAME_CSS
        CS_SwitchTeam(client, TEAM_3);
    #endif
}

#if defined PROJECT_GAME_CSS

/**
 * Round has ended.
 */
public ZRTM_RoundEnd()
{
    ZRTM_BalanceTeams();
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
public Action:ZRTM_PlayerTeamPre(client, team, oldteam, bool:disconnect)
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
public ZRTM_PlayerTeam(client, team, oldteam, bool:disconnect)
{
    if (team == TEAM_1)
        TLib_SetClientTeam(client, ZTeam_Spectator);
    
    // If client isn't already in the game, then assign them to human by default.
    if (!TLib_IsClientPlaying(client) && team > TEAM_1)
        TLib_SetClientTeam(client, ZTeam_Human);
}

#endif

#if defined PROJECT_GAME_CSS

/**
 * Move all clients to their respective game teams based off ZTeam.
 */
stock ZRTM_UpdateClientTeams()
{
    new ZTeam:team;
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        team = TLib_GetClientTeam(client);
        
        if (team == ZTeam_Zombie)
            CS_SwitchTeam(client, TEAM_2);
        else if (team == ZTeam_Human)
            CS_SwitchTeam(client, TEAM_3);
    }
}

#endif

/**
 * Randomly put an equal amount of clients on both teams.
 */
stock ZRTM_BalanceTeams()
{
    new bool:team1 = true;
    
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        if (!TLib_IsClientPlaying(client))
            continue;
        
        #if defined PROJECT_GAME_CSS
            if (team1)
                CS_SwitchTeam(client, TEAM_2);
            else
                CS_SwitchTeam(client, TEAM_3);
        #endif
        
        team1 = !team1;
    }
}
