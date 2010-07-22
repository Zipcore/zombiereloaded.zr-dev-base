/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          infection.zr.inc
 *  Type:          Module
 *  Description:   Handles infection for classic ZR.
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
 * The weapon classname to fire player_death with.
 * Default: "zombie_claws_of_death" for compatibility.
 */
#define ZOMBIE_WEAPON_CLASSNAME "zombie_claws_of_death"

/**
 * This module's identifier.
 */
new Module:g_moduleZRInfect;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRInfect_GetIdentifier() { return g_moduleZRInfect; }

/**
 * Event ID variables.
 */
new ProjectEvent:g_EvOnClientInfect;
new ProjectEvent:g_EvOnClientHuman;

/**
 * Cvar handles.
 */
new Handle:g_hCvarMZombieMinTime;
new Handle:g_hCvarMZombieMaxTime;
new Handle:g_hCvarMZombieRatio;
new Handle:g_hCvarMZombieBlockConsecutive;

/**
 * Variable/function set to check/set if zombies are present.
 */
new bool:g_bZombiesPresent;
stock bool:ZRInfect_AreZombiesPresent() { return g_bZombiesPresent; }
stock ZRInfect_SetZombiesPresent(bool:zombiespresent) { g_bZombiesPresent = zombiespresent; }

/**
 * Timer handles for mother zombie selection and round timer.
 */
new Handle:g_hMotherInfect;
new Handle:g_hMotherInfectCountdown;

/**
 * Keeps track of who the mother zombies are for the current round.
 */
new bool:g_bMotherZombie[MAXPLAYERS + 1];

/**
 * Keeps track of if a client was mother zombie in the previous round.
 * See cvar g_hCvarMZombieBlockConsecutive.
 */
new bool:g_bWasMotherZombie[MAXPLAYERS + 1];

/**
 * Register this module.
 */
ZRInfect_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "ZR Infection Handler");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zrinfection");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Handles infection for classic ZR.");
    moduledata[ModuleData_Dependencies][0] = ZR_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRInfect = ModuleMgr_Register(moduledata);
    
    // Create custom events.
    g_EvOnClientInfect =    EventMgr_CreateEvent("Event_OnClientInfect");
    g_EvOnClientHuman =     EventMgr_CreateEvent("Event_OnClientHuman");
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnEventsRegister",          "ZRInfect_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRInfect_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnPluginEnd",               "ZRInfect_OnPluginEnd");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnMapStart",                "ZRInfect_OnMapStart");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnMapEnd",                  "ZRInfect_OnMapEnd");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnClientPutInServer",       "ZRInfect_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_OnClientDisconnect",        "ZRInfect_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_RoundStart",                "ZRInfect_RoundStart");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_RoundFreezeEnd",            "ZRInfect_RoundFreezeEnd");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_RoundEnd",                  "ZRInfect_RoundEnd");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_PlayerSpawn",               "ZRInfect_PlayerSpawn");
    EventMgr_RegisterEvent(g_moduleZRInfect, "Event_PlayerHurt",                "ZRInfect_PlayerHurt");
    //EventMgr_RegisterEvent(g_moduleZRInfect, "Event_PlayerDeath",               "ZRInfect_PlayerDeath");
    
    #endif
}

/**
 * Plugin is loading.
 */
ZRInfect_OnPluginStart()
{
    // Register the module.
    ZRInfect_Register();
    
    // Create cvars.
    g_hCvarMZombieMinTime =             CreateConVar("zr_mzombie_min_time", "1.0", "Minimum time from the start of the round until picking the mother zombie(s).");
    g_hCvarMZombieMaxTime =             CreateConVar("zr_mzombie_max_time", "1.0", "Maximum time from the start of the round until picking the mother zombie(s).");
    g_hCvarMZombieRatio =               CreateConVar("zr_mzombie_ratio", "5", "Number of humans to infect as mother zombies in proportion to number of humans on the server. ['0' = Always one mother zombie]");
    g_hCvarMZombieBlockConsecutive =    CreateConVar("zr_mzombie_block_consecutive", "1", "Prevent a player from being chosen as mother zombie two rounds in a row.");
    
    // Create commands.
    RegConsoleCmd("zr_infect", ZRInfect_InfectCommand, "Turn a player into a zombie. Usage: zr_infect <#userid|name> [respawn - 1/0]");
    RegConsoleCmd("zr_human", ZRInfect_HumanCommand, "Turn a player into a human. Usage: zr_human <#userid|name> [respawn - 1/0] [protect - 1/0]");
}

/**
 * Plugin is ending.
 */
public ZRInfect_OnPluginEnd()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
}

/**
 * The map has started.
 */
public ZRInfect_OnMapStart()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
}

/**
 * The map has ended.
 */
public ZRInfect_OnMapEnd()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public ZRInfect_OnClientPutInServer(client)
{
    g_bMotherZombie[client] = false;
    g_bWasMotherZombie[client] = false;
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public ZRInfect_OnClientDisconnect(client)
{
    // If there are no more zombies then pick a new one.
    if (TLib_CountTeam(ZTeam_Zombie, true) <= 1)
        ZRInfect_MotherZombie(1);
}

/**
 * Round has started.
 */
public ZRInfect_RoundStart()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
    
    // Fresh round, no zombies.
    g_bZombiesPresent = false;
}

/**
 * Pre-round freezetime has finished.
 */
public ZRInfect_RoundFreezeEnd()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
    
    // Get min and max times and pick a random time inbetween.
    new Float:mintime = GetConVarFloat(g_hCvarMZombieMinTime);
    new Float:maxtime = GetConVarFloat(g_hCvarMZombieMaxTime);
    new randomtime = RoundToNearest(GetRandomFloat(mintime, maxtime));
    
    g_hMotherInfect = CreateTimer(float(randomtime), ZRInfect_InitMotherZombie, false, TIMER_FLAG_NO_MAPCHANGE);
}

/**
 * Round has ended.
 */
public ZRInfect_RoundEnd()
{
    // Stop timers.
    Util_CloseHandle(g_hMotherInfect);
    Util_CloseHandle(g_hMotherInfectCountdown);
    
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        // Only reset clients that are in the game to human.  Ignore unassigned/spectators.
        if (TLib_IsClientPlaying(client))
            TLib_SetClientTeam(client, ZTeam_Human);
        
        // Keep track of who was mother zombie this round for next round.
        g_bWasMotherZombie[client] = g_bMotherZombie[client];
        g_bMotherZombie[client] = false;
    }
    
    // No more zombies.
    g_bZombiesPresent = false;
}

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 * 
 */
public ZRInfect_PlayerSpawn(client)
{
}


/**
 * Client has been damaged.
 * 
 * @param victim        The index of the hurt client.
 * @param attacker      The index of the attacking client.
 * @param health        How much health the client has after the damage.
 * @param armor         How much armor the client has after the damage.
 * @param weapon        The weapon classname used to hurt the victim. (No weapon_ prefix)
 * @param dmg_health    The amount of health the victim lost.
 * @param dmg_armor     The amount of armor the victim lost.
 * @param hitgroup      The hitgroup index of the victim that was damaged.
 */
public ZRInfect_PlayerHurt(victim, attacker, health, armor, const String:weapon[], dmg_health, dmg_armor, hitgroup)
{
    // Check if the victim and attacker are valid clients.
    if (!Util_IsClientInGame(victim) || !Util_IsClientInGame(attacker))
        return;
    
    // Check if a zombie is attacking a human.
    if (TLib_IsClientZombie(attacker) && TLib_IsClientHuman(victim))
    {
        #if defined PROJECT_GAME_CSS
            if (StrEqual(weapon, "knife"))
            {
                ZRInfect_HumanToZombie(victim, attacker, false);
            }
        #endif
    }
}

/**
 * Client has been killed.
 * 
 * @param victim    The index of the killed client.
 * @param attacker  The killer of the victim.
 * @param weapon    The weapon classname used to kill the victim. (No weapon_ prefix)
 * @param headshot  True if the death was by headshot, false if not.
 */
public ZRInfect_PlayerDeath(victim, attacker, const String:weapon[], bool:headshot)
{
}

/**
 * Turn a human into a zombie.
 * 
 * @param client    The client being infected.
 * @param attacker  The client doing the infecting.
 * @param mzombie   True to make a mother zombie, false for normal zombie. 
 */
ZRInfect_HumanToZombie(client, attacker = 0, bool:mzombie)
{
    PrintToChatAll("INFECT %N infected %N", attacker, client);
    TLib_SetClientTeam(client, ZTeam_Zombie);
    
    TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR infected");
    
    // Definitely zombies present now.
    g_bZombiesPresent = true;
    
    if (mzombie)
    {
        g_bMotherZombie[client] = true;
    }
    
    #if defined PROJECT_GAME_CSS
        // Only fire the event if there is an attacker.
        if (attacker != 0)
        {
            new Handle:hEventPlayerDeath = CreateEvent("player_death", true);
            SetEventInt(hEventPlayerDeath, "userid", GetClientUserId(client));
            SetEventInt(hEventPlayerDeath, "attacker", GetClientUserId(attacker));
            SetEventString(hEventPlayerDeath, "weapon", ZOMBIE_WEAPON_CLASSNAME);
            SetEventBool(hEventPlayerDeath, "headshot", false);
            FireEvent(hEventPlayerDeath);
        }
    #endif
    
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][1];
    
    eventdata[0][0] = client;
    eventdata[1][0] = attacker;
    eventdata[2][0] = mzombie;
    
    EventMgr_Forward(g_EvOnClientInfect, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
}

/**
 * Turn a zombie into a human.
 * 
 * @param client    The zombie client to turn human.
 */
ZRInfect_ZombieToHuman(client)
{
    TLib_SetClientTeam(client, ZTeam_Human);
    
    TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR zombie to human");
    
    // If client was a mother zombie then set to false.
    g_bMotherZombie[client] = false;
    
    // Forward event to all modules.
    new any:eventdata[1][1];
    
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientHuman, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
}

/**
 * Choose a random mother zombie.
 * 
 * @param override_count    If this is set to something > 0 then this is how many mother zombies will be spawned.
 */
ZRInfect_MotherZombie(override_count = 0)
{
    // Create eligible player list.
    new Handle:adtClients;
    new count = Util_BuildClientList(adtClients, UTILS_FILTER_TEAM | UTILS_FILTER_ALIVE | UTILS_FILTER_HUMAN);
    
    // If there are no clients that can become a zombie then stop.
    if (count == 0)
    {
        CloseHandle(adtClients);
        return;
    }
    
    // Prune the list further.
    new client;
    new bool:blockconsinfect = GetConVarBool(g_hCvarMZombieBlockConsecutive);
    for (new cindex = 0; cindex < count; cindex++)
    {
        // Don't pick a mother zombie if there is only 1 client left at this point.
        if (count <= 1)
            break;
        
        // Get client stored in array index.
        client = GetArrayCell(adtClients, cindex);
        
        // Prune out clients that were mother zombies last round to prevent double-infection.
        if (blockconsinfect && g_bWasMotherZombie[client])
        {
            // Prune, subtract 1 from count, and backtrack one index.
            RemoveFromArray(adtClients, cindex); count--; cindex--;
        }
    }
    
    // Determine how many mother zombies to spawn.
    new mzombiecount;
    if (override_count > 0)
    {
        mzombiecount = override_count;
    }
    else
    {
        // Ratio of mother zombies to humans.
        new ratio = GetConVarInt(g_hCvarMZombieRatio);
        
        // Count living clients on each team.
        //new zombiecount = TLib_CountTeam(ZTeam_Zombie, true);
        new humancount = TLib_CountTeam(ZTeam_Human, true);
        
        // Calculate mother zombie count.
        mzombiecount = (ratio > 0) ? RoundToNearest(float(humancount) / float(ratio)) : 1;
        
        // If mothercount is 0, then set to 1.
        if (mzombiecount == 0)
            mzombiecount = 1;
    }
    
    new randindex; 
    for (new cindex = 0; cindex < mzombiecount; cindex++)
    {
        // Should always leave at least one human.
        if (TLib_CountTeam(ZTeam_Human, true) <= 1)
            break;
        
        // Recount clients
        count = GetArraySize(adtClients);
        
        // Get a random client from the array, infect, and remove from list.
        randindex = GetRandomInt(0, count - 1);
        ZRInfect_HumanToZombie(GetArrayCell(adtClients, randindex), _, true);
        RemoveFromArray(adtClients, randindex);
    }
    
    CloseHandle(adtClients);
}

/**
 * Timer callback.
 * Choose a mother zombie. 
 * 
 * @param timer     The timer handle.
 */
public Action:ZRInfect_InitMotherZombie(Handle:timer)
{
    g_hMotherInfect = INVALID_HANDLE;
    
    // Pick a random mother zombie.
    ZRInfect_MotherZombie();
}

/**
 * Command callback: zr_infect
 * Turn a player into a zombie.
 * 
 * @param client    The client index. 
 * @param argc      The number of arguments that the server sent with the command.
 */
public Action:ZRInfect_InfectCommand(client, argc)
{
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command infect syntax");
        return Plugin_Handled;
    }
    
    decl String:target[MAX_NAME_LENGTH], String:targetname[MAX_NAME_LENGTH];
    new targets[MAXPLAYERS], bool:tn_is_ml, result;
    
    // Get targetname.
    GetCmdArg(1, target, sizeof(target));
    
    // Find a target.
    result = ProcessTargetString(target, client, targets, sizeof(targets), COMMAND_FILTER_ALIVE, targetname, sizeof(targetname), tn_is_ml);
        
    // Check if there was a problem finding a client.
    if (result <= 0)
    {
        TransMgr_ReplyToTargetError(client, result);
        return Plugin_Handled;
    }
    
    // Check optional parameter.
    decl String:strRespawn[4];
    GetCmdArg(2, strRespawn, sizeof(strRespawn));
    new bool:bRespawn = bool:StringToInt(strRespawn);
    
    for (new tindex = 0; tindex < result; tindex++)
    {
        // Can't infect a zombie.
        if (TLib_IsClientHuman(targets[tindex]))
        {
            // If a human is infected, cancel the timer that infects a mother zombie.
            if (g_bZombiesPresent)
            {
                ZRInfect_HumanToZombie(targets[tindex], _, false);
                if (result == 1)
                {
                    TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command infect successful", targets[tindex]);
                }
            }
            else
            {
                Util_CloseHandle(g_hMotherInfect);
                ZRInfect_HumanToZombie(targets[tindex], _, true);
                if (result == 1)
                {
                    TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command infect mother successful", targets[tindex]);
                }
            }
        }
        else
        {
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command infect unsuccessful", targets[tindex]);
            }
        }
    }
    
    return Plugin_Handled;
}

/**
 * Command callback: zr_human
 * Turn a player into a human.
 * 
 * @param client    The client index. 
 * @param argc      The number of arguments that the server sent with the command.
 */
public Action:ZRInfect_HumanCommand(client, argc)
{
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command infect syntax");
        return Plugin_Handled;
    }
    
    decl String:target[MAX_NAME_LENGTH], String:targetname[MAX_NAME_LENGTH];
    new targets[MAXPLAYERS], bool:tn_is_ml, result;
    
    // Get targetname.
    GetCmdArg(1, target, sizeof(target));
    
    // Find a target.
    result = ProcessTargetString(target, client, targets, sizeof(targets), COMMAND_FILTER_ALIVE, targetname, sizeof(targetname), tn_is_ml);
        
    // Check if there was a problem finding a client.
    if (result <= 0)
    {
        TransMgr_ReplyToTargetError(client, result);
        return Plugin_Handled;
    }
    
    // Check optional parameters.
    decl String:strRespawn[4];
    GetCmdArg(2, strRespawn, sizeof(strRespawn));
    new bool:bRespawn = bool:StringToInt(strRespawn);
    
    decl String:strProtect[4];
    GetCmdArg(3, strProtect, sizeof(strProtect));
    new bool:bProtect = bool:StringToInt(strProtect);
    
    for (new tindex = 0; tindex < result; tindex++)
    {
        // Can't infect a zombie.
        if (TLib_IsClientZombie(targets[tindex]))
        {
            ZRInfect_ZombieToHuman(targets[tindex]);
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command human successful", targets[tindex]);
            }
        }
        else
        {
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZR command human unsuccessful", targets[tindex]);
            }
        }
    }
    
    return Plugin_Handled;
}
