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
 * For interfacing purposes.
 */
public bool:ZRiotInfect_AreZombiesPresent() { return true; }

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
    EventMgr_RegisterEvent(g_moduleZRiotInfect, "Event_OnMyCoreActivate",       "ZRiotInfect_OnMyCoreActivate");
    EventMgr_RegisterEvent(g_moduleZRiotInfect, "Event_OnMyCoreShutdown",       "ZRiotInfect_OnMyCoreShutdown");
    
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
}

/**
 * Forwarded to the core module being shutdown before ZRC_OnMyCoreActivate and ZRC_OnGameRulesApplied.
 */
public ZRiotInfect_OnMyCoreShutdown(Module:module)
{
    if (module == g_moduleZRiotInfect)
    {
        // Reset infection interface for the next game mode to set up.
        IInfection_SetClientToZombie(INVALID_FUNCTION);
        IInfection_SetClientToHuman(INVALID_FUNCTION);
        IInfection_SetAreZombiesPresent(INVALID_FUNCTION);
    }
}

/**
 * Forwarded to the core module being activated after ZRC_OnMyCoreShutdown but before ZRC_OnGameRulesApplied.
 */
public ZRiotInfect_OnMyCoreActivate(Module:module)
{
    if (module == g_moduleZRiotInfect)
    {
        // Set up infection interface.
        IInfection_SetClientToZombie(GetFunctionByName(GetMyHandle(), "ZRiotInfect_ClientToZombie"));
        IInfection_SetClientToHuman(GetFunctionByName(GetMyHandle(), "ZRiotInfect_ClientToHuman"));
        IInfection_SetAreZombiesPresent(GetFunctionByName(GetMyHandle(), "ZRiotInfect_AreZombiesPresent"));
    }
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
    
    // Automatically turn a client into a zombie or human depending on their virtual team.
    if (TLib_IsClientZombie(client))
        ZRiotInfect_ClientToZombie(client);
    else if (TLib_IsClientHuman(client))
        ZRiotInfect_ClientToHuman(client);
}

#endif

/**
 * Turn a human into a zombie.
 * 
 * @param client    The client being infected.
 */
public ZRiotInfect_ClientToZombie(client)
{
    PrintToChatAll("infected %N (hp to 500)", client);
    SetEntityHealth(client, 500);
    
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
public ZRiotInfect_ClientToHuman(client)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientHuman, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
}
