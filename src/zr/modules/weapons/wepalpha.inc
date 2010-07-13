/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *  Dependencies: weapons.inc
 *
 *  File:          wepalpha.inc
 *  Type:          Module
 *  Description:   Weapon alpha functions, and alpha updating on drop/pickup.
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
new Module:g_moduleWepAlpha;

/**
 * Register this module.
 */
WepAlpha_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Weapon Alpha Updater");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "wepalpha");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Updates weapon alpha when a player's alpha is changed.");
    moduledata[ModuleData_Dependencies][0] = Weapons_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleWepAlpha = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleWepAlpha, "Event_OnClientPutInServer",       "WepAlpha_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleWepAlpha, "Event_OnClientDisconnect",        "WepAlpha_OnClientDisconnect");
    EventMgr_RegisterEvent(g_moduleWepAlpha, "Event_OnClientAlphaChanged",      "WepAlpha_OnClientAlphaChanged");
}

/**
 * Plugin is loading.
 */
WepAlpha_OnPluginStart()
{
    // Register the module (sub-module of weapons)
    WepAlpha_Register();
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @return      Return Plugin_Handled to stop load, and Plugin_Continue to allow it.
 */
public WepAlpha_OnMyModuleEnable()
{
    // Hook all clients in the server.
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        SDKHook(client, SDKHook_WeaponEquip, WepAlpha_Equip);
        SDKHook(client, SDKHook_WeaponDrop, WepAlpha_Drop);
    }
}

/**
 * The module that hooked this event callback has been disabled.
 */
public WepAlpha_OnMyModuleDisable()
{
    // Unhook all clients in the server.
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        SDKUnhook(client, SDKHook_WeaponEquip, WepAlpha_Equip);
        SDKUnhook(client, SDKHook_WeaponDrop, WepAlpha_Drop);
    }
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public WepAlpha_OnClientPutInServer(client)
{
    SDKHook(client, SDKHook_WeaponEquip, WepAlpha_Equip);
    SDKHook(client, SDKHook_WeaponDrop, WepAlpha_Drop);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public WepAlpha_OnClientDisconnect(client)
{
    SDKUnhook(client, SDKHook_WeaponEquip, WepAlpha_Equip);
    SDKUnhook(client, SDKHook_WeaponDrop, WepAlpha_Drop);
}

/**
 * Client has just picked up a weapon.
 * 
 * @param client    The client index.
 * @param weapon    The weapon index.
 */
public Action:WepAlpha_Equip(client, weapon)
{
    // Get the client's color.
    new color[4];
    OffsLib_GetEntityColor(client, color);
    
    // Set new alpha on weapon.
    SetEntityRenderMode(client, RENDER_TRANSALPHA);
    SetEntityRenderColor(client, _, _, _, color[3]);
}

/**
 * Called when a client drops their weapon.
 * 
 * @param client    The client index.
 * @param weapon    The weapon index.
 */
public Action:WepAlpha_Drop(client, weapon)
{
    // If weapon isn't a valid entity, then stop.
    if (weapon < MaxClients)
    {
        return;
    }
    
    // Reset alpha to default on the dropped weapon.
    SetEntityRenderMode(client, RENDER_TRANSALPHA);
    SetEntityRenderColor(client, _, _, _, 255);
}

/**
 * A client's alpha has changed.
 * 
 * @param client    The client index.
 * @param newalpha  The client's new alpha. 
 */
public WepAlpha_OnClientAlphaChanged(client, newalpha)
{
    // Set new alpha on weapons.
    WepAlpha_Apply(client, newalpha);
}

/**
 * Sets an alpha on all of a client's weapons.
 * 
 * @param client    The client index.
 * @param alpha     The alpha to set the weapons to.
 */
stock WepAlpha_Apply(client, alpha)
{
    // Get client's list of weapons.
    new weapons[WepLib_Slots];
    WepLib_GetWeapons(client, weapons);
    
    // Loop through array slots and set alpha.
    // x = weapon slot.
    for (new sindex = 0; sindex < WEPLIB_SLOT_COUNT; sindex++)
    {
        // Check if there is a weapon in the slot.
        if (weapons[sindex] == -1)
            continue;
        
        SetEntityRenderMode(weapons[sindex], RENDER_TRANSALPHA);
        SetEntityRenderColor(weapons[sindex], _, _, _, alpha);
    }
}
