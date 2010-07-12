/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          damage.zr.inc
 *  Type:          Module
 *  Description:   Handles damage rules for classic ZR.
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
new Module:g_moduleZRDmg;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRDmg_GetIdentifier() { return g_moduleZRDmg; }

/**
 * Register this module.
 */
ZRDmg_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Zombie:Reloaded Damage Control");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zrdamage");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Handles damage rules for classic ZR.");
    moduledata[ModuleData_Dependencies][0] = ZR_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRDmg = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnMyModuleEnable",         "ZRDmg_OnMyModuleEnable");
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnMyModuleDisable",        "ZRDmg_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnClientPutInServer",      "ZRDmg_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnClientDisconnect",       "ZRDmg_OnClientDisconnect");
}

/**
 * Plugin is loading.
 */
ZRDmg_OnPluginStart()
{
    // Register the module.
    ZRDmg_Register();
}

/**
 * TODO: Hook traceattack and block FF.
 */

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @return      Return Plugin_Handled to stop load, and Plugin_Continue to allow it.
 */
public ZRDmg_OnMyModuleEnable()
{
    // Hook all clients in the server.
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        // Hook WeaponCanUse on the client for restricting the weapon.
        SDKHook(client, SDKHook_OnTakeDamage, ZRDmg_OnTakeDamage);
    }
}

/**
 * The module that hooked this event callback has been disabled.
 */
public ZRDmg_OnMyModuleDisable()
{
    // Unhook all clients in the server.
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        SDKUnhook(client, SDKHook_OnTakeDamage, ZRDmg_OnTakeDamage);
    }
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public ZRDmg_OnClientPutInServer(client)
{
    // Hook WeaponCanUse on the client for restricting the weapon.
    SDKHook(client, SDKHook_OnTakeDamage, ZRDmg_OnTakeDamage);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public ZRDmg_OnClientDisconnect(client)
{
    SDKUnhook(client, SDKHook_OnTakeDamage, ZRDmg_OnTakeDamage);
}

/**
 * Stop damage if zombie is infecting a human.
 * 
 * @param victim        The client being attacked.
 * @param attacker      The client attacking the victim.
 * @param inflictor     The entity doing damage?
 * @param damage        The amount of damage inflicted.
 * @param damagetype    A bit indicating type of damage that was being inflicted.
 *  
 * @return              Hook action.  See include/core.inc.
 */
public Action:ZRDmg_OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype)
{
    // Check if the victim and attacker are valid clients.
    if (!Util_IsClientInGame(victim) || !Util_IsClientInGame(attacker))
        return Plugin_Continue;
    
    // Check if a zombie is attacking a human.
    if (TLib_IsClientZombie(attacker) && TLib_IsClientHuman(victim))
    {
        // Knifing is included in DMG_BULLET in CS:S.
        // They couldn't just use DMG_SLASH because that would make sense.
        if (damagetype & DMG_BULLET)
        {
            SetEntityHealth(victim, GetClientHealth(victim) + RoundToNearest(damage));
        }
    }
    
    return Plugin_Continue;
}
