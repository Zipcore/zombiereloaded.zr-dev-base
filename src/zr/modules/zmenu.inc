/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          zmenu.inc
 *  Type:          Module
 *  Description:   A list of functions for clients.
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
 * Include libraries.
 */
#include "zr/libraries/menulib"

/**
 * This module's identifier.
 */
new Module:g_moduleZMenu;

/**
 * Handle to ZMenu.
 */
new Handle:g_hZMenu;

/**
 * Register this module.
 */
ZMenu_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZMenu");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zmenu");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "A list of functions for clients.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZMenu = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZMenu, "Event_OnEventsRegister",             "ZMenu_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZMenu_OnEventsRegister()
{
    // Register all the events needed for this module.
}

/**
 * Plugin is loading.
 */
ZMenu_OnPluginStart()
{
    // Register the module.
    ZMenu_Register();
    
    // Register the zmenu command.
    RegConsoleCmd("zmenu", Command_ZMenu);
    
    // Register menu with menulib.
    g_hZMenu = MenuLib_CreateMenu("zmenu", "ZMenu title", true);
}

/**
 * Command callback (zmenu)
 * Open up zmenu.
 * 
 * @param client    The client index.
 * @param argc      Argument count.
 */
public Action:Command_ZMenu(client, argc)
{
    if (client == SERVER_INDEX)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, INVALID_MODULE, false, "Must be player");
        return;
    }
    
    MenuLib_DisplayMenu(g_hZMenu, client);
}
