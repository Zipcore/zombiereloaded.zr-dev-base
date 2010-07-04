/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          
 *  Type:          Module
 *  Description:   
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
new Module:g_moduleZRRoot;

/**
 * Register this module.
 */
ZR_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Zombie:Reloaded Root");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zr_root");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Top of the chain module for ZR's set of core modules.  Disabling this will end ZR gameplay.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    moduledata[ModuleData_Root] = true;
    
    // Send this array of data to the module manager.
    g_moduleZRRoot = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnAllPluginsLoaded",       "ZR_OnAllPluginsLoaded");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnPluginEnd",              "ZR_OnPluginEnd");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnAllModulesLoaded",       "ZR_OnAllModulesLoaded");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnModuleEnable",           "ZR_OnModuleEnable");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnMyModuleEnable",         "ZR_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnModuleDisable",          "ZR_OnModuleDisable");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnMyModuleDisable",        "ZR_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnMapStart",               "ZR_OnMapStart");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnMapEnd",                 "ZR_OnMapEnd");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnAutoConfigsBuffered",    "ZR_OnAutoConfigsBuffered");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnConfigsExecuted",        "ZR_OnConfigsExecuted");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnClientPutInServer",      "ZR_OnClientPutInServer");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnClientDisconnect",       "ZR_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_RoundStart",               "ZR_RoundStart");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_RoundFreezeEnd",           "ZR_RoundFreezeEnd");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_RoundEnd",                 "ZR_RoundEnd");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_PlayerTeam",               "ZR_PlayerTeam");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_PlayerSpawn",              "ZR_PlayerSpawn");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_PlayerHurt",               "ZR_PlayerHurt");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_PlayerDeath",              "ZR_PlayerDeath");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_PlayerJump",               "ZR_PlayerJump");
    //EventMgr_RegisterEvent(g_moduleZRRoot, "Event_WeaponFire",               "ZR_WeaponFire");
    
    #endif
    
    // Register config file(s) that this module will use.
    // ConfigMgr_Register(g_moduleZRRoot, "ZR_OnConfigReload", "configs/<projectshortname>/<config>.txt");
}

/**
 * Plugin is loading.
 */
ZR_OnPluginStart()
{
    // Register the module.
    ZR_Register();
}

/**
 * All plugins have loaded.
 */
public ZR_OnAllPluginsLoaded()
{
}

/**
 * Plugin is ending.
 */
public ZR_OnPluginEnd()
{
}

/**
 * All modules have been registered.
 */
public ZR_OnAllModulesLoaded()
{
}

/**
 * A module has been enabled.
 */
public ZR_OnModuleEnable(Module:module)
{
}

/**
 * The module that hooked this event callback has been enabled.
 */
public ZR_OnMyModuleEnable()
{
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @return      Return Plugin_Handled to stop load, and Plugin_Continue to allow it.
 */
public Action:ZR_OnMyModuleEnable()
{
}

/**
 * A module has been disabled.
 */
public ZR_OnModuleDisable(Module:module)
{
}

/**
 * The module that hooked this event callback has been disabled.
 */
public ZR_OnMyModuleDisable()
{
}

/**
 * The map has started.
 */
public ZR_OnMapStart()
{
}

/**
 * The map has ended.
 */
public ZR_OnMapEnd()
{
}

/**
 * This is called before OnConfigsExecuted but any time after OnMapStart.
 * Per-map settings should be set here. 
 */
public ZR_OnAutoConfigsBuffered()
{
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public ZR_OnConfigsExecuted()
{
}

/**
 * Called when a registered config file (by this module) is manually.
 */
public ZR_OnConfigReload(configindex)
{
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public ZR_OnClientPutInServer(client)
{
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public ZR_OnClientDisconnect(client)
{
}

/**
 * Round has started.
 */
public ZR_RoundStart()
{
}

/**
 * Pre-round freezetime has finished.
 */
public ZR_RoundFreezeEnd()
{
}

/**
 * Round has ended.
 */
public ZR_RoundEnd()
{
}

/**
 * Client has joined a team.
 * 
 * @param client        The client index.
 * @param team          The client's new team.
 * @param oldteam       The team the client is switching from.
 * @param disconnect    True if the client is disconnecting, false if switching to a team.
 */
public ZR_PlayerTeam(client, team, oldteam, bool:disconnect)
{
}

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 */
public ZR_PlayerSpawn(client)
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
public ZR_PlayerHurt(victim, attacker, health, armor, const String:weapon[], dmg_health, dmg_armor, hitgroup)
{
}

/**
 * Client has been killed.
 * 
 * @param victim    The index of the killed client.
 * @param attacker  The killer of the victim.
 * @param weapon    The weapon classname used to kill the victim. (No weapon_ prefix)
 * @param headshot  True if the death was by headshot, false if not.
 */
public ZR_PlayerDeath(victim, attacker, const String:weapon[], bool:headshot)
{
}

/**
 * Client has jumped.
 * 
 * @param client        The client index.
 */
public ZR_PlayerJump(client)
{
}

/**
 * Client has fired a weapon.
 * 
 * @param client        The client index.
 * @param weapon        The weapon classname fired. (No weapon_ prefix)
 */
public ZR_WeaponFire(client, const String:weapon[])
{
}
