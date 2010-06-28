/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          zombiereloaded.inc
 *  Type:          Module
 *  Description:   Core module, contains various utility functions.
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
new Module:g_moduleCore;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:Core_GetIdentifier() { return g_moduleCore; }

/**
 * Register this module.
 */
Core_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = true;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Core");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "core");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleCore = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnAllPluginsLoaded",      "Core_OnAllPluginsLoaded");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnPluginEnd",             "Core_OnPluginEnd");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnAllModulesLoaded",      "Core_OnAllModulesLoaded");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnModuleEnable",          "Core_OnModuleEnable");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnMyModuleEnable",        "Core_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnModuleDisable",         "Core_OnModuleDisable");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnModuleMyDisable",       "Core_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnMapStart",              "Core_OnMapStart");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnMapEnd",                "Core_OnMapEnd");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnAutoConfigsBuffered",   "Core_OnAutoConfigsBuffered");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnConfigsExecuted",       "Core_OnConfigsExecuted");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnClientPutInServer",     "Core_OnClientPutInServer");
    //EventMgr_RegisterEvent(g_moduleCore, "Event_OnClientDisconnect",      "Core_OnClientDisconnect");
}

/**
 * Plugin is loading.
 */
Core_OnPluginStart()
{
    // Register the module.
    Core_Register();
}

// **********************************************
//                 Utilities
// **********************************************

/**
 * Check if a client index is on a team.
 * 
 * @param client    The client index.
 * 
 * @return          True if client is on a team, false otherwise.
 */
stock bool:Util_IsClientOnTeam(client)
{
    #if defined PROJECT_GAME_CSS
    new cteam = GetClientTeam(client);
    return (cteam == CS_TEAM_T || cteam == CS_TEAM_CT);
    #endif
}

/**
 * Retrieve a keyvalue as a string and convert it to a boolean value.
 * 
 * @param kv            The kv handle.
 * @param key           The keyvalue key to get value from.
 * @param boolphrase    The boolean pair to check.  See enum BoolPhrases.
 * @param defvalue      The string value used if the key doesn't exist.
 */
stock bool:Util_KvGetPhraseAsBool(Handle:kv, const String:key[], BoolPhrases:boolphrase, const String:defvalue[] = "no")
{
    decl String:phrase[16];
    KvGetString(kv, key, phrase, sizeof(phrase), defvalue);
    
    new PhraseToBool:result = TransMgr_PhraseToBoolEx(boolphrase, phrase);
    switch(result)
    {
        case PhraseToBool_False:    return false;
        case PhraseToBool_True:     return true;
        case PhraseToBool_Neither:  return false;
    }
    
    return false;
}

/**
 * Converts a boolean value into a menu draw define.
 * true =   ITEMDRAW_DEFAULT
 * false =  ITEMDRAW_DISABLED
 * 
 * @param condition The boolean to convert. 
 */
stock Util_GetMenuItemDraw(bool:condition)
{
    return condition ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED;
}
