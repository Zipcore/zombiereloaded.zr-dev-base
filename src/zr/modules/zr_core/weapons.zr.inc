/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          weapons.zr.inc
 *  Type:          Module
 *  Description:   Manages weapons for classic zombie gameplay.
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
new Module:g_moduleZRWeapons;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRWeapons_GetIdentifier() { return g_moduleZRWeapons; }

/**
 * Cvar handles.
 */
#if defined PROJECT_GAME_CSS
  new Handle:g_hCvarZRWeaponsDrop;
  new Handle:g_hCvarZRWeaponsEndEquip;
#endif

/**
 * Array to block any client from picking up weapons.
 */
new bool:g_bZRWeaponsBlockWeapon[MAXPLAYERS + 1];

/**
 * Register this module.
 */
ZRWeapons_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "ZR Weapon Control");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zrweapons");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Manages weapons for classic zombie gameplay.");
    moduledata[ModuleData_Dependencies][0] = ZR_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRWeapons = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnEventsRegister",         "ZRWeapons_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRWeapons_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnEventsReady",            "ZRWeapons_OnEventsReady");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_RoundEnd",                 "ZRWeapons_RoundEnd");
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_PlayerSpawn",              "ZRWeapons_PlayerSpawn");
    
    #endif
    
    // Register custom events.
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Event_OnClientInfect",           "ZRWeapons_OnClientInfect");
    EventMgr_RegisterEvent(g_moduleZRWeapons, "Hook_WeaponCanUse",              "ZRWeapons_CanUse");
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public ZRWeapons_OnEventsReady()
{
    // Bump up priority for Event_RoundEnd to above zrinfect module.
    EventMgr_GivePriority("Event_RoundEnd", g_moduleZRWeapons, ZRInfect_GetIdentifier());
}

/**
 * Plugin is loading.
 */
ZRWeapons_OnPluginStart()
{
    // Register the module.
    ZRWeapons_Register();
    
    // Create cvars.
    #if defined PROJECT_GAME_CSS
        g_hCvarZRWeaponsDrop =      CreateConVar("zr_weapons_drop", "0", "Force player to drop all weapons when infected.  Disabling this will strip weapons instead.");
        g_hCvarZRWeaponsEndEquip =  CreateConVar("zr_weapons_endequip", "1", "Restricts zombies from picking up weapons after the round has ended but before the next round has begun.");
    #endif
    
    #if defined PROJECT_GAME_CSS
        // Listen for buy commands.
        AddCommandListener(ZRWeapons_BuyListener, "buy");
        AddCommandListener(ZRWeapons_BuyListener, "autobuy");
        AddCommandListener(ZRWeapons_BuyListener, "rebuy");
    #endif
}

/**
 * Client has been infected.
 * 
 * @param client        The infected client.
 * @param attacker      The zombie that infected the client.
 * @param motherzombie  True if the client has been infected as a mother zombie.
 */
public ZRWeapons_OnClientInfect(client, attacker, bool:motherzombie)
{
    #if defined PROJECT_GAME_CSS
        WepLib_RemoveAllWeapons(client, GetConVarBool(g_hCvarZRWeaponsDrop));
        GivePlayerItem(client, "weapon_knife");
    #endif
}

#if defined PROJECT_GAME_CSS

/**
 * Round has ended.
 * Note: This needs to be BEFORE the event that sets all clients back to human.
 */
public ZRWeapons_RoundEnd()
{
    if (!GetConVarBool(g_hCvarZRWeaponsEndEquip))
        return;
    
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        if (TLib_IsClientZombie(client))
            g_bZRWeaponsBlockWeapon[client] = true;
    }
}

/**
 * Client has spawned.
 * 
 * @param client    The client index.
 * 
 */
public ZRWeapons_PlayerSpawn(client)
{
    if (!Util_IsClientOnTeam(client))
        return;
    
    // Reset block weapons flag.
    g_bZRWeaponsBlockWeapon[client] = false;
}

#endif

/**
 * Called when a client is trying to pick up a weapon.
 * 
 * @param client    The client index.
 * @param weapon    The weapon index.
 * 
 * @return          Hook action.  See include/core.inc.
 */
public Action:ZRWeapons_CanUse(client, weapon)
{
    new String:weaponentity[64];
    GetEdictClassname(weapon, weaponentity, sizeof(weaponentity));
    
    // Knife should never be restricted.
    if (StrEqual(weaponentity, "weapon_knife"))
        return Plugin_Continue;
    
    // The flat override for stopping a client from picking up a weapon.
    if (g_bZRWeaponsBlockWeapon[client])
        return Plugin_Handled;
    
    // Only humans can pick up weapons.
    if (!TLib_IsClientHuman(client))
        return Plugin_Handled;
    
    return Plugin_Continue;
}

/**
 * Command listener for the buy, autorebuy, and rebuy commands.
 * Used to block use of this command under certain conditions.
 * 
 * @param client    The client index.
 * @param argc      Argument count.
 */
public Action:ZRWeapons_BuyListener(client, const String:command[], argc)
{
    // Not a client using the command.
    if (client <= 0)
        return Plugin_Continue;
    
    if (!IsClientInGame(client))
        return Plugin_Continue;
    
    // Only humans can buy weapons.
    if (!TLib_IsClientHuman(client))
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR weapons zombie cant use weapon");
        return Plugin_Handled;
    }
    
    return Plugin_Continue;
}
