/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          menu_weapons.inc
 *  Type:          Core
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
 * An index of a weapon or weapon group to store on clients traversing the weapons menus.
 */
new g_iWeaponsCurType[MAXPLAYERS + 1];

/**
 * Sends main weapon menu to client.
 * 
 * @param client    The client index.
 */
stock Weapons_MenuMain(client)
{
    // Create menu handle.
    new Handle:menu_weapons_main = CreateMenu(Weapons_MenuMainHandle);
    
    decl String:title[128];
    decl String:restrict[128];
    decl String:zmarket[128];
    
    Format(title, sizeof(title), "%T\n ", "Weapons menu restrict main title", client);
    Format(restrict, sizeof(restrict), "%T", "Weapons menu restrict main restrict", client);
    Format(zmarket, sizeof(zmarket), "%T", "Weapons menu restrict main market", client);
    
    // Draw items, make unselectable if module is disabled.
    SetMenuTitle(menu_weapons_main, title);
    AddMenuItem(menu_weapons_main, "restrict", restrict, MenuGetItemDraw(GetConVarBool(g_hCvarsList[CVAR_WEAPONS_RESTRICT])));
    AddMenuItem(menu_weapons_main, "zmarket", zmarket, MenuGetItemDraw(GetConVarBool(g_hCvarsList[CVAR_ZMARKET])));
    
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
public Weapons_MenuMainHandle(Handle:menu_weapons_main, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        switch(slot)
        {
            case 0: WeaponsMenuTypes(client);
            case 1: WeaponsMenuZMarket(client);
        }
    }
    // Client is closing the menu.
    else if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
            ZAdminMenu(client);
    }
    // Menu is closing.
    else if (action == MenuAction_End)
    {
        CloseHandle(menu_weapons_main);
    }
}

/**
 * Sends weapon type list to client.
 *  
 * @param client    The client index.
 */
stock Weapons_MenuTypes(client)
{
    // Create menu handle.
    new Handle:menu_weapons_types = CreateMenu(WeaponsMenuTypesHandle);
    
    SetMenuTitle(menu_weapons_types,  "%T\n ", "Weapons menu restrict types title", client);
    
    decl String:typename[WEAPONS_MAX_LENGTH];
    
    new size = GetArraySize(arrayWeaponTypes);
    for (new wtindex = 0; wtindex < size; wtindex++)
    {
        RestrictWeaponTypeGetName(wtindex, typename, sizeof(typename));
        AddMenuItem(menu_weapons_types, typename, typename);
    }
    
    // If there are no weapons, add an "(Empty)" line.
    if (size == 0)
    {
        // TODO: Make a general "_Empty" phrase.
        decl String:empty[64];
        Format(empty, sizeof(empty), "%T", "Menu empty", client);
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
public Weapons_MenuTypesHandle(Handle:menu_weapons_types, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        // Menu slot index is = weapon type index.
        g_iWeaponsCurType[client] = slot;
        WeaponsMenuTypeWeapons(client);
    }
    // Client closed the menu.
    else if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
            WeaponsMenuMain(client);
    }
    // Client hit "Exit" button.
    else if (action == MenuAction_End)
    {
        CloseHandle(menu_weapons_types);
    }
}

/**
 * Sends a list of weapons of a certain type in a menu to the client.
 *  
 * @param client    The client index.
 */
Weapons_MenuTypeWeapons(client)
{
    // Create menu handle.
    new Handle:menu_weapons_typeweapons = CreateMenu(Weapons_MenuTypeWeaponsHandle);
    
    decl String:typename[WEAPONS_MAX_LENGTH];
    RestrictWeaponTypeGetName(g_iWeaponsCurType[client], typename, sizeof(typename));
    
    decl String:title[128];
    decl String:restrictall[128];
    decl String:unrestrictall[128];
    
    Format(title, sizeof(title), "%T\n ", "Weapons menu restrict types weapon type title", client, typename);
    Format(restrictall, sizeof(restrictall), "%T", "Weapons menu restrict types restrict all", client, typename);
    Format(unrestrictall, sizeof(unrestrictall), "%T\n ", "Weapons menu restrict types unrestrict all", client, typename);
    
    // Draw items as selectable only if not all weapons within the type are restricted or unrestricted.
    SetMenuTitle(menu_weapons_typeweapons, title);
    AddMenuItem(menu_weapons_typeweapons, "restrictall", restrictall, MenuGetItemDraw(!RestrictIsTypeUniform(true, g_iWeaponsCurType[client])));
    AddMenuItem(menu_weapons_typeweapons, "unrestrictall", unrestrictall, MenuGetItemDraw(!RestrictIsTypeUniform(false, g_iWeaponsCurType[client])));
    
    decl String:typeweapon[64];
    decl String:display[64 + 2]; // +2 to allow room for the [ ] if its restricted.
    
    // Get an array populated with all weapons of the given type.
    new Handle:arrayTypeWeapons;
    new count = RestrictGetTypeWeapons(g_iWeaponsCurType[client], arrayTypeWeapons);
    
    for (new x = 0; x < count; x++)
    {
        // Get weapon index to check restricted status of.
        new weaponindex = GetArrayCell(arrayTypeWeapons, x);
        
        // Get name of weapon.
        WeaponsGetName(weaponindex, typeweapon, sizeof(typeweapon));
        strcopy(display, sizeof(display), typeweapon);
        
        if (RestrictIsWeaponRestricted(weaponindex))
        {
            Format(display, sizeof(display), "[%s]", typeweapon);
        }
        
        // Disable option if it isn't toggleable.
        AddMenuItem(menu_weapons_typeweapons, typeweapon, display, MenuGetItemDraw(WeaponsGetToggleable(weaponindex)));
    }
    
    // Destroy the array handle.
    CloseHandle(arrayTypeWeapons);
    
    // Set menu back button.
    SetMenuExitBackButton(menu_weapons_typeweapons, true);
    
    // Display menu to client.
    DisplayMenu(menu_weapons_typeweapons, client, MENU_TIME_FOREVER);
}

/**
 * Called when client selects option in the weapon group menu, and handles it.
 *  
 * @param menu_weapons_typeweapons  Handle of the menu being used.
 * @param action                    The action done on the menu (see menus.inc, enum MenuAction).
 * @param client                    The client index.
 * @param slot                      The slot index selected (starting from 0).
 */ 
public WeaponsMenuTypeWeaponsHandle(Handle:menu_weapons_typeweapons, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        // Get name of current weapon type.
        decl String:typename[WEAPONS_MAX_LENGTH];
        RestrictWeaponTypeGetName(g_iWeaponsCurType[client], typename, sizeof(typename));
        
        new RestrictQuery:query;
        new bool:single;
        new bool:restrict;
        decl String:returntarget[WEAPONS_MAX_LENGTH];
        
        switch(slot)
        {   
            case 0:
            {
                // Restrict all weapons of this type.
                restrict = true;
                query = RestrictWeapon(true, typename, single, returntarget, sizeof(returntarget));
            }
            case 1:
            {
                // Unrestrict all weapons of this type.
                restrict = false;
                query = RestrictWeapon(false, typename, single, returntarget, sizeof(returntarget));
            }
            default:
            {
                // Get weappon name.
                decl String:typeweapon[MENU_LINE_REG_LENGTH];
                GetMenuItem(menu_weapons_typeweapons, slot, typeweapon, sizeof(typeweapon));
                
                // Get weapon index.
                new weaponindex = WeaponsNameToIndex(typeweapon);
                
                // If weapon index is -1, then something went very wrong.
                if (weaponindex == -1)
                {
                    CloseHandle(menu_weapons_typeweapons);
                    return;
                }
                
                // If weapon isn't restricted, then restrict it.
                if (!RestrictIsWeaponRestricted(weaponindex))
                {
                    // Restrict this weapon.
                    restrict = true;
                    query = RestrictWeapon(true, typeweapon, single, returntarget, sizeof(returntarget));
                }
                else
                {
                    // Unrestrict this weapon.
                    restrict = false;
                    query = RestrictWeapon(false, typeweapon, single, returntarget, sizeof(returntarget));
                }
            }
        }
        
        // Print query response.
        RestrictPrintQueryResponse(client, query, single, restrict, returntarget);
        
        // Resend menu.
        WeaponsMenuTypeWeapons(client);
    }
    // Client closed the menu.
    if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
        {
            WeaponsMenuTypes(client);
        }
    }
    // Client hit "Exit" button.
    else if (action == MenuAction_End)
    {
        CloseHandle(menu_weapons_typeweapons);
    }
}

/**
 * Sends ZMarket options menu to client.
 *  
 * @param client    The client index.
 */
WeaponsMenuZMarket(client)
{
    // Create menu handle.
    new Handle:menu_weapons_market = CreateMenu(WeaponsMenuZMarketHandle);
    
    // Get "yes" or "no" settings from respective cvar.
    decl String:buyzonesetting[MENU_LINE_SMALL_LENGTH];
    ConfigBoolToSetting(GetConVarBool(g_hCvarsList[CVAR_ZMARKET_BUYZONE]), buyzonesetting, sizeof(buyzonesetting), true, client);
    
    SetGlobalTransTarget(client);
    
    decl String:title[MENU_LINE_TITLE_LENGTH];
    decl String:buyzone[MENU_LINE_REG_LENGTH];
    
    // Add options to menu.
    Format(title, sizeof(title), "%t\n ", "Weapons menu restrict zmarket title");
    Format(buyzone, sizeof(buyzone), "%t", "Weapons menu restrict zmarket buyzone", buyzonesetting);
    
    SetMenuTitle(menu_weapons_market, title);
    AddMenuItem(menu_weapons_market, "buyzone", buyzone);
    
    // Create a "Back" button to the weapons main menu.
    SetMenuExitBackButton(menu_weapons_market, true);
    
    // Send menu
    DisplayMenu(menu_weapons_market, client, MENU_TIME_FOREVER);
}

/**
 * Called when client selects option in the weapons main menu, and handles it.
 *  
 * @param menu_weapons_market   Handle of the menu being used.
 * @param action                The action done on the menu (see menus.inc, enum MenuAction).
 * @param client                The client index.
 * @param slot                  The slot index selected (starting from 0).
 */ 
public WeaponsMenuZMarketHandle(Handle:menu_weapons_market, MenuAction:action, client, slot)
{
    // Client selected an option.
    if (action == MenuAction_Select)
    {
        switch(slot)
        {
            // Buyzone.
            case 0:
            {
                // Toggle cvar.
                new bool:zmarketbuyzone = GetConVarBool(g_hCvarsList[CVAR_ZMARKET_BUYZONE]);
                SetConVarBool(g_hCvarsList[CVAR_ZMARKET_BUYZONE], !zmarketbuyzone);
            }
        }
        
        // Resend menu.
        WeaponsMenuZMarket(client);
    }
    // Client closed the menu.
    if (action == MenuAction_Cancel)
    {
        // Client hit "Back" button.
        if (slot == MenuCancel_ExitBack)
        {
            WeaponsMenuMain(client);
        }
    }
    // Client hit "Exit" button.
    else if (action == MenuAction_End)
    {
        CloseHandle(menu_weapons_market);
    }
}
