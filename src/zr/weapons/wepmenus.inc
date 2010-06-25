/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *  Dependencies: weapons.inc, weprestrict.inc
 *
 *  File:          menus.inc
 *  Type:          Module
 *  Description:   Handles weapons management menu.
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
 * Sends main weapon menu to client.
 * 
 * @param client    The client index.
 */
stock WepMenus_Main(client)
{
    // Create menu handle.
    new Handle:menu_weapons_main = CreateMenu(WepMenu_MainHandle);
    
    decl String:title[128];
    decl String:restrict[128];
    
    Format(title, sizeof(title), "%T\n ", "Weapons menu main title", client);
    Format(restrict, sizeof(restrict), "%T", "Weapons menu restrict main restrict", client);
    
    // Draw items, make unselectable if module is disabled.
    SetMenuTitle(menu_weapons_main, title);
    AddMenuItem(menu_weapons_main, "restrict", restrict, Util_GetMenuItemDraw(GetConVarBool(g_hCvarRestrict)));
    
    SetMenuExitBackButton(menu_weapons_main, true);
    DisplayMenu(menu_weapons_main, client, MENU_TIME_FOREVER);
}

/**
 * Called when client selects option in the weapons main menu, and handles it.
 *  
 * @param menu_weapons_main     Handle of the menu being used.
 * @param action                The action done on the menu (see menus.inc, enum MenuAction).
 * @param client                The client index.
 * @param slot                  The slot index selected (starting from 0).
 */
public WepMenu_MainHandle(Handle:menu_weapons_main, MenuAction:action, client, slot)
{
    
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        switch(slot)
        {
            case 0: WepMenus_Types(client);
        }
    }
    // Client is closing the menu.
    else if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
        {
            //ZAdminMenu(client);
            // TODO: Link back to admin menu.
        }
    }
    // Menu is closing.
    else if (action == MenuAction_End)
        CloseHandle(menu_weapons_main);
}

/**
 * Sends weapon type list to client.
 *  
 * @param client    The client index.
 */
stock WepMenus_Types(client)
{
    // Create menu handle.
    new Handle:menu_weapons_types = CreateMenu(WepMenus_TypesHandle);
    
    SetMenuTitle(menu_weapons_types,  "%T\n ", "Weapons menu restrict types title", client);
    
    decl String:typename[64];
    for (new wtindex = 0; wtindex < g_iWeaponTypeCount; wtindex++)
    {
        Weapons_ReadWTypeName(wtindex, typename, sizeof(typename));
        AddMenuItem(menu_weapons_types, typename, typename);
    }
    
    // If there are no weapons, add an "(Empty)" line.
    if (g_iWeaponTypeCount == 0)
    {
        decl String:empty[64];
        Format(empty, sizeof(empty), "(%T)", "_Empty", client);
        AddMenuItem(menu_weapons_types, "empty", empty, ITEMDRAW_DISABLED);
    }
    
    // Set exit back button.
    SetMenuExitBackButton(menu_weapons_types, true);
    
    DisplayMenu(menu_weapons_types, client, MENU_TIME_FOREVER);
}

/**
 * Called when client selects option in the weapons list menu, and handles it.
 *  
 * @param menu_weapons_types    Handle of the menu being used.
 * @param action                The action done on the menu (see menus.inc, enum MenuAction).
 * @param client                The client index.
 * @param slot                  The slot index selected (starting from 0).
 */
public WepMenus_TypesHandle(Handle:menu_weapons_types, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
        WepMenus_TypeWeapons(client, slot);
    
    // Client closed the menu.
    else if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
            WepMenus_Main(client);
    }
    // Client hit "Exit" button.
    else if (action == MenuAction_End)
        CloseHandle(menu_weapons_types);
}

/**
 * Sends a list of weapons of a certain type in a menu to the client.
 * 
 * @param client    The client index.
 * @param typeindex The array index in g_hWeaponTypes of the weapon type to put in the menu.
 */
new g_WepMenusSelectedIndex[MAXPLAYERS + 1];
stock WepMenus_TypeWeapons(client, typeindex)
{
    // Create menu handle.
    new Handle:menu_weapons_typeweapons = CreateMenu(WepMenus_TypeWeaponsHandle);
    
    decl String:typename[64];
    Weapons_ReadWTypeName(typeindex, typename, sizeof(typename));
    
    decl String:title[128];
    decl String:restrictall[128];
    decl String:unrestrictall[128];
    
    Format(title, sizeof(title), "%T\n ", "Weapons menu restrict types weapon type title", client, typename);
    Format(restrictall, sizeof(restrictall), "%T", "Weapons menu restrict types restrict all", client, typename);
    Format(unrestrictall, sizeof(unrestrictall), "%T\n ", "Weapons menu restrict types unrestrict all", client, typename);
    
    // Draw items as selectable only if not all weapons within the type are restricted or unrestricted.
    SetMenuTitle(menu_weapons_typeweapons, title);
    
    AddMenuItem(menu_weapons_typeweapons, "restrictall", restrictall, Util_GetMenuItemDraw(!WepRestrict_AreWeaponsUniform(typeindex, true)));
    AddMenuItem(menu_weapons_typeweapons, "unrestrictall", unrestrictall, Util_GetMenuItemDraw(!WepRestrict_AreWeaponsUniform(typeindex, true)));
    
    decl String:typeweapon[WEPCONFIG_NAME];
    decl String:display[64 + 2]; // +2 to allow room for the [ ] if its restricted.
    
    // Get an array populated with all weapons of the given type.
    new Handle:hWeapons;
    new count = Weapons_GetWeaponsOfType(typeindex, hWeapons);
    new windex;
    
    for (new weparrayindex = 0; weparrayindex < count; weparrayindex++)
    {
        windex = GetArrayCell(hWeapons, weparrayindex);
        
        // Get name of weapon.
        Weapons_ReadString(windex, WepConfig_Name, typeweapon, sizeof(typeweapon));
        strcopy(display, sizeof(display), typeweapon);
        
        // If the weapon is restricted, then format brackets around it as an indicator.
        if (WepRestrict_IsRestricted(windex))
            Format(display, sizeof(display), "[%s]", typeweapon);
        
        // Add the item to the menu
        AddMenuItem(menu_weapons_typeweapons, typeweapon, display, Util_GetMenuItemDraw(bool:Weapons_ReadCell(windex, WepConfig_Toggleable)));
    }
    
    CloseHandle(hWeapons);
    
    SetMenuExitBackButton(menu_weapons_typeweapons, true);
    DisplayMenu(menu_weapons_typeweapons, client, MENU_TIME_FOREVER);
    
    g_WepMenusSelectedIndex[client] = typeindex;
}

/**
 * Called when client selects option in the weapon group menu, and handles it.
 * 
 * @param menu_weapons_typeweapons  Handle of the menu being used.
 * @param action                    The action done on the menu (see menus.inc, enum MenuAction).
 * @param client                    The client index.
 * @param slot                      The slot index selected (starting from 0).
 */
public WepMenus_TypeWeaponsHandle(Handle:menu_weapons_typeweapons, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        // Get name of current weapon type.
        decl String:typename[64];
        Weapons_ReadWTypeName(g_WepMenusSelectedIndex[client], typename, sizeof(typename));
        
        new RestrictQuery:query;
        new bool:single;
        new bool:restrict;
        decl String:returntarget[WEPCONFIG_NAME];
        
        switch(slot)
        {   
            case 0:
            {
                // Restrict all weapons of this type.
                restrict = true;
                query = WepRestrict_Restrict(true, typename, single, returntarget, sizeof(returntarget));
            }
            case 1:
            {
                // Unrestrict all weapons of this type.
                restrict = false;
                query = WepRestrict_Restrict(false, typename, single, returntarget, sizeof(returntarget));
            }
            default:
            {
                // Get weappon name.
                decl String:typeweapon[64];
                GetMenuItem(menu_weapons_typeweapons, slot, typeweapon, sizeof(typeweapon));
                
                // Get weapon index.
                new windex = Weapons_NameToIndex(typeweapon);
                if (windex == -1)
                {
                    LogMgr_Print(g_moduleWeapons, LogType_Error, "Weapon Restriction Menu", "Invalid weapon (%s) in the restriction menu", typeweapon);
                    return;
                }
                
                // If weapon isn't restricted, then restrict it.
                if (!WepRestrict_IsRestricted(windex))
                {
                    // Restrict this weapon.
                    restrict = true;
                    query = WepRestrict_Restrict(true, typeweapon, single, returntarget, sizeof(returntarget));
                }
                else
                {
                    // Unrestrict this weapon.
                    restrict = false;
                    query = WepRestrict_Restrict(false, typeweapon, single, returntarget, sizeof(returntarget));
                }
            }
        }
        
        // Print query response.
        WepRestrict_PrintQueryResponse((query != Query_Successful) ? client : 0, query, single, restrict, returntarget);
        
        // Resend menu.
        WepMenus_TypeWeapons(client, g_WepMenusSelectedIndex[client]);
    }
    
    // Client closed the menu.
    else if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
            WepMenus_Types(client);
    }
    // Client hit "Exit" button.
    else if (action == MenuAction_End)
        CloseHandle(menu_weapons_typeweapons);
}
