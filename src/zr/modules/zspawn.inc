/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          zspawn.inc
 *  Type:          Module
 *  Description:   Allows players to spawn into the game late.
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

#include "zr/libraries/authcachelib"

/**
 * TODO:
 * The rest should be compiler errors.
 */

/**
 * This module's identifier.
 */
new Module:g_moduleZSpawn;

/**
 * Cvar handles.
 */
new Handle:g_hCvarZSpawnTeam;
new Handle:g_hCvarZSpawnTimeLimit;
new Handle:g_hCvarZSpawnTimeLimitRules;

/**
 * Stores the auth cache handle.
 */
new Handle:g_hZSpawnAuthCache;

/**
 * ZSpawn timer handle.
 */
new Handle:g_hZSpawnTimer;

/**
 * Register this module.
 */
ZSpawn_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZSpawn");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zspawn");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Allows players to spawn into the game late.");
    
    // Send this array of data to the module manager.
    g_moduleZSpawn = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_OnMyModuleDisable",        "ZSpawn_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_OnMapStart",               "ZSpawn_OnMapStart");
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_OnClientDisconnect",       "ZSpawn_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_RoundStart",               "ZSpawn_RoundStart");
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_RoundFreezeEnd",           "ZSpawn_RoundFreezeEnd");
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_RoundEnd",                 "ZSpawn_RoundEnd");
    EventMgr_RegisterEvent(g_moduleZSpawn, "Event_PlayerDeath",              "ZSpawn_PlayerDeath");
    
    #endif
}

/**
 * Plugin is loading.
 */
ZSpawn_OnPluginStart()
{
    // Register the module.
    ZSpawn_Register();
    
    // Create cvars.
    g_hCvarZSpawnTeam =             Project_CreateConVar("zspawn_team_zombie", "0", "Team to spawn player on when using ZSpawn. ['0' = Zombie | '1' = Human]");
    g_hCvarZSpawnTimeLimit =        Project_CreateConVar("zspawn_timelimit", "120.0", "The amount of time from round start before ZSpawn changes its behavior. ['0' = No timelimit]");
    g_hCvarZSpawnTimeLimitRules =   Project_CreateConVar("zspawn_timelimit_rules", "1", "How ZSpawn should work after the timelimit has expired. ['-1' = Block ZSpawn | '0' = Spawn as zombie | '1' = Spawn as human | Dependency: zspawn_team_zombie&zspawn_timelimit]");
    
    // Create commands.
    RegConsoleCmd("zspawn", ZSpawn_SayCommand, "Spawn into the game.");
}

/**
 * The module that hooked this event callback has been disabled.
 */
public ZSpawn_OnMyModuleDisable()
{
    // End the timer.
    Util_CloseHandle(g_hZSpawnTimer);
}

/**
 * The map has started.
 */
public ZSpawn_OnMapStart()
{
    // Initialize auth cache.
    Util_CloseHandle(g_hZSpawnAuthCache);
    g_hZSpawnAuthCache = AuthCache_Create();
    
    // End the timer.
    Util_CloseHandle(g_hZSpawnTimer);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public ZSpawn_OnClientDisconnect(client)
{
    // If client clicked "cancel" during connection or is a bot, then stop.
    if (!IsClientInGame(client) || IsFakeClient(client))
        return;
    
    // Add client to cache.
    AuthCache_AddClient(g_hZSpawnAuthCache, client);
}

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 */
public ZSpawn_RoundStart()
{
    // Clear everyone from the cache.
    AuthCache_Clear(g_hZSpawnAuthCache);
    
    // End the timer.
    Util_CloseHandle(g_hZSpawnTimer);
}

/**
 * Pre-round freezetime has finished.
 */
public ZSpawn_RoundFreezeEnd()
{
    // End the timer.
    Util_CloseHandle(g_hZSpawnTimer);
    
    // Get timelimit, and check if it's disabled.
    new Float:timelimit = GetConVarFloat(g_hCvarZSpawnTimeLimit);
    if (timelimit > 0.0)
        g_hZSpawnTimer = CreateTimer(timelimit, ZSpawn_TimeLimitTimer, _, TIMER_FLAG_NO_MAPCHANGE);
}

/**
 * Round has ended.
 */
public ZSpawn_RoundEnd()
{
    // End the timer.
    Util_CloseHandle(g_hZSpawnTimer);
}

/**
 * Client has been killed.
 * 
 * @param victim    The index of the killed client.
 * @param attacker  The killer of the victim.
 * @param weapon    The weapon classname used to kill the victim. (No weapon_ prefix)
 * @param headshot  True if the death was by headshot, false if not.
 */
public ZSpawn_PlayerDeath(victim, attacker, const String:weapon[], bool:headshot)
{
    // Add client to cache.
    AuthCache_AddClient(g_hZSpawnAuthCache, victim);
}

#endif

/**
 * ZSpawn timer callback.
 * 
 * @param timer     The timer handle.
 */
public Action:ZSpawn_TimeLimitTimer(Handle:timer)
{
    // Clear timer handle variable.
    g_hZSpawnTimer = INVALID_HANDLE;
}

/**
 * Command callback: zspawn
 * Client command to spawn into the game late.
 * 
 * @param client    The client index.
 * @param argc      Argument count.
 */
public Action:ZSpawn_SayCommand(client, argc)
{
    // Check if the module is disabled.
    if (ModuleMgr_IsDisabled(g_moduleZSpawn))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "Feature is disabled");
        return Plugin_Handled;
    }
    
    // If client is console, then stop and tell them this feature is for players only.
    if (client == SERVER_INDEX)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "Must be player");
        return Plugin_Handled;
    }
    
    // Check if client is on a team.
    if (!Util_IsClientOnTeam(client))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "Must be on team");
        return Plugin_Handled;
    }
    
    // Check if client is alive.
    if (IsPlayerAlive(client))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "Must be dead");
        return Plugin_Handled;
    }
    
    // Check if client has already played in this round.
    if (AuthCache_ClientExists(g_hZSpawnAuthCache, client))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "ZSpawn double spawn");
        return Plugin_Handled;
    }
    
    new Float:timelimit = GetConVarFloat(g_hCvarZSpawnTimeLimit);
    if (timelimit == 0 || g_hZSpawnTimer != INVALID_HANDLE)
    {
        // Spawn as a human first, then switch to zombie if we need to.
        TLib_SetClientTeam(client, VTeam_Human);
        CS_RespawnPlayer(client);
        
        
        // Set the client's team and then spawn them.
        new spawnteam = GetConVarInt(g_hCvarZSpawnTeam);
        if (spawnteam == 0)
            ZRCInfect_HumanToZombie(client, _, false);
    }
    else
    {
        // Timelimit is expired, new rules.
        new timelimitrules = GetConVarInt(g_hCvarZSpawnTimeLimitRules);
        switch(timelimitrules)
        {
            case -1:
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, g_moduleZSpawn, _, "ZSpawn timelimit", RoundToNearest(timelimit));
                return Plugin_Handled;
            }
            case 0:
            {
                TLib_SetClientTeam(client, VTeam_Human);
                CS_RespawnPlayer(client);
                ZRCInfect_HumanToZombie(client, _, false);
            }
            case 1:
            {
                TLib_SetClientTeam(client, VTeam_Human);
                CS_RespawnPlayer(client);
            }
            default:
            {
                LogMgr_Print(g_moduleZSpawn, LogType_Error, "Cvar \"zspawn_timelimit_rules\"", "Invalid value \"%d\" defined in this cvar.", timelimitrules);
            }
        }
    }
    
    // This stops the "Unknown command" message in client's console.
    return Plugin_Handled;
}

// *********************************************************************************************

// STUFF BELOW THIS IS NO LONGER PART OF ZSPAWN MODULE.  IT SHOULD BE MOVED TO RESPAWN MODULE
// BECAUSE ITS BASICALLY JUST A RESPAWN COMMAND

/**
 * Command callback: zr_zspawn_force
 * Force ZSpawn on a client.
 * 
 * @param client    The client index.
 * @param argc      Argument count.
 */
/*
public Action:Command_ZSpawnForce(client, argc)
{
    // Check if privileged.
    if (!ZRIsClientPrivileged(client, OperationType_Generic))
    {
        TranslationReplyToCommand(client, "No access to command");
        return Plugin_Handled;
    }
    
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TranslationReplyToCommand(client, "ZSpawn command force syntax");
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
        ZRReplyToTargetError(client, result);
        return Plugin_Handled;
    }
    
    // Get item to give to client.
    decl String:strZombie[4];
    GetCmdArg(2, strZombie, sizeof(strZombie));
    
    // Copy value of second (optional) parameter to 'zombie'.
    // It will be false if the parameter wasn't specified.
    new bool:zombie = bool:StringToInt(strZombie);
    
    // x = Client index.
    for (new x = 0; x < result; x++)
    {
        // Give client the item.
        new bool:success = ZSpawnClient(targets[x], true, zombie);
        
        // Tell admin the outcome of the command if only 1 client was targetted.
        if (result == 1)
        {
            if (success)
            {
                TranslationReplyToCommand(client, "ZSpawn command force successful", targetname);
            }
            else
            {
                TranslationReplyToCommand(client, "ZSpawn command force unsuccessful", targetname);
            }
        }
    }
    
    // Log action to game events.
    LogEvent(false, LogTypeOld_Normal, LOG_GAME_EVENTS, LogModule_ZSpawn, "Force ZSpawn", "Admin \"%L\" forced player(s) to spawn. (zr_zspawn_force)", client);
    
    return Plugin_Handled;
}
*/

/**
 * Menu callback (zspawn_force)
 * Forces ZSpawn on a client.
 * 
 * @param menu      The menu handle.
 * @param action    Action client is doing in menu.
 * @param client    The client index.
 * @param slot      The menu slot selected. (starting from 0)
 */
/*
public ZSpawnForceHandle(Handle:menu_zspawn_force, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        // Get the client index of the selected client.
        new target = MenuGetClientIndex(menu_zspawn_force, slot);
        
        // If the target is 0, then the client left before being selected from the menu.
        if (target == 0)
        {
            // Re-send the menu.
            MenuClientList(client, ZSpawnForceHandle, true, true, false, "ZSpawn clients title");
            
            return;
        }
        
        // Get the target's name for future use.
        decl String:targetname[MAX_NAME_LENGTH];
        GetClientName(target, targetname, sizeof(targetname));
        
        // Force ZSpawn on the target.
        new bool:success = ZSpawnClient(target, true);
        
        // Tell admin the outcome of the action.
        if (success)
        {
            TranslationReplyToCommand(client, "ZSpawn command force successful", targetname);
        }
        else
        {
            TranslationReplyToCommand(client, "ZSpawn command force unsuccessful", targetname);
        }
        
        // Re-send the menu.
        MenuClientList(client, ZSpawnForceHandle, true, false, true, "ZSpawn clients title");
    }
    // Client closed the menu.
    if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
        {
            // Re-open admin menu.
            ZAdminMenu(client);
        }
    }
    // Client exited menu.
    if (action == MenuAction_End)
    {
        CloseHandle(menu_zspawn_force);
    }
}
*/
