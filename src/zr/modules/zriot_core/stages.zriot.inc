/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          stages.zriot.inc
 *  Type:          Module
 *  Description:   Handles and applies stage configs.
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
new Module:g_moduleZRiotStages;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRiotStages_GetIdentifier() { return g_moduleZRiotStages; }

/**
 * Cvar handles.
 */


/**
 * Register this module.
 */
ZRiotStages_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZRiot Stages");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zriotstages");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Handles and applies stage configs.");
    moduledata[ModuleData_Dependencies][0] = ZRiot_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRiotStages = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnEventsRegister",        "ZRiotStages_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRiotStages_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnEventsReady",          "ZRiotStages_OnEventsReady");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnMyModuleEnable",       "ZRiotStages_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnMyModuleDisable",       "ZRiotStages_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnMapStart",              "ZRiotStages_OnMapStart");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_OnClientInfect",          "ZRiotStages_OnClientInfect");
    
    #if defined PROJECT_GAME_CSS
    
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_RoundStart",              "ZRiotStages_RoundStart");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_RoundFreezeEnd",          "ZRiotStages_RoundFreezeEnd");
    //EventMgr_RegisterEvent(g_moduleZRiotStages, "Event_RoundEnd",                "ZRiotStages_RoundEnd");
    
    #endif
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public ZRiotStages_OnEventsReady()
{
}

/**
 * Plugin is loading.
 */
ZRiotStages_OnPluginStart()
{
    // Register the module.
    ZRiotStages_Register();
    
    // Create cvars.
    
}
