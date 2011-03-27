/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          infection.zriot.inc
 *  Type:          Module
 *  Description:   Handles infection for ZRiot.
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
new Module:g_moduleZRiotInfect;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRiotInfect_GetIdentifier() { return g_moduleZRiotInfect; }

/**
 * Register this module.
 */
ZRiotInfect_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZRiot Infection Handler");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zriotinfection");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Handles infection for ZRiot.");
    moduledata[ModuleData_Dependencies][0] = ZRiot_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRiotInfect = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRiotInfect, "Event_OnEventsRegister",          "ZRiotInfect_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRiotInfect_OnEventsRegister()
{
    // Register all the events needed for this module.
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRiotInfect, "Event_PlayerSpawn",            "ZRiotInfect_PlayerSpawn");
    
    #endif
}

/**
 * Plugin is loading.
 */
ZRiotInfect_OnPluginStart()
{
    // Register the module.
    ZRiotInfect_Register();
    
    // Create commands.
    RegConsoleCmd("zriot_infect", ZRiotInfect_InfectCommand, "Turn a player into a zombie. Usage: zriot_infect <#userid|name> [respawn - 1/0]");
    RegConsoleCmd("zriot_human", ZRiotInfect_HumanCommand, "Turn a player into a human. Usage: zriot_human <#userid|name> [respawn - 1/0] [protect - 1/0]");
}

#if defined PROJECT_GAME_CSS

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 * 
 */
public ZRiotInfect_PlayerSpawn(client)
{
    // player_spawn is fired during the connection process, this is how we filter it out.
    if (!Util_IsClientOnTeam(client))
        return;
    
    // Check if the client is on the zombie team.
    if (GetClientTeam(client) == TLib_GetGameTeamIndex(VTeam_Zombie))
    {
        ZRiotInfect_HumanToZombie(client);
    }
}

#endif

/**
 * Turn a human into a zombie.
 * 
 * @param client    The client being infected.
 */
ZRiotInfect_HumanToZombie(client)
{
    TLib_SetClientTeam(client, VTeam_Zombie);
    
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][1];
    
    eventdata[0][0] = client;
    eventdata[1][0] = 0;        // No attacker in ZRiot.
    eventdata[2][0] = false;    // No mother zombies in ZRiot.
    
    EventMgr_Forward(g_EvOnClientInfected, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
}

/**
 * Turn a zombie into a human.
 * 
 * @param client    The zombie client to turn human.
 */
ZRiotInfect_ZombieToHuman(client)
{
    TLib_SetClientTeam(client, VTeam_Human);
    
    // If client was a mother zombie then set to false.
    g_bMotherZombie[client] = false;
    
    // Forward event to all modules.
    new any:eventdata[1][1];
    
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientHuman, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
}

/**
 * Command callback: zriot_infect
 * Turn a player into a zombie.
 * 
 * @param client    The client index. 
 * @param argc      The number of arguments that the server sent with the command.
 */
public Action:ZRiotInfect_InfectCommand(client, argc)
{
    // Check if the this core module is disabled, if so then don't do anything with it.
    if (ModuleMgr_IsDisabled(g_moduleZRiotInfect))
    {
        return Plugin_Continue;
    }
    
    // Check if the client has permission to use this.
    if (!AccessMgr_HasAccess(client, g_moduleZRiotInfect))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "No access to command");
        return Plugin_Handled;
    }
    
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command infect syntax");
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
            ZRiotInfect_HumanToZombie(targets[tindex]);
            TransMgr_PrintText(targets[tindex], MsgFormat_Plugin, MsgType_Chat, _, _, "ZRiot human to zombie");
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command infect successful", targets[tindex]);
            }
        }
        else
        {
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command infect unsuccessful", targets[tindex]);
            }
        }
    }
    
    return Plugin_Handled;
}

/**
 * Command callback: zriot_human
 * Turn a player into a human.
 * 
 * @param client    The client index. 
 * @param argc      The number of arguments that the server sent with the command.
 */
public Action:ZRiotInfect_HumanCommand(client, argc)
{
    // Check if the this core module is disabled, if so then don't do anything with it.
    if (ModuleMgr_IsDisabled(g_moduleZRiotInfect))
    {
        return Plugin_Continue;
    }
    
    // Check if the client has permission to use this.
    if (!AccessMgr_HasAccess(client, g_moduleZRiotInfect))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "No access to command");
        return Plugin_Handled;
    }
    
    // If not enough arguments given, then stop.
    if (argc < 1)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command human syntax");
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
            ZRiotInfect_ZombieToHuman(targets[tindex]);
            TransMgr_PrintText(targets[tindex], MsgFormat_Plugin, MsgType_Chat, _, _, "ZRiot zombie to human");
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command human successful", targets[tindex]);
            }
        }
        else
        {
            if (result == 1)
            {
                TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ZRiot command human unsuccessful", targets[tindex]);
            }
        }
    }
    
    return Plugin_Handled;
}
