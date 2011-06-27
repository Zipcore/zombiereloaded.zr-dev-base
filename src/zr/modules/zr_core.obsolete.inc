/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          zr_core.inc
 *  Type:          Module
 *  Description:   Game mode-independent stuff.
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

// Include libraries.

/**
 * This module's identifier.
 */
new Module:g_moduleZRCore;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRCore_GetIdentifier() { return g_moduleZRCore; }

/**
 * Cvar handles.
 */
new Handle:g_hCvarZombieTeam;

/**
 * Register this module.
 */
ZRCore_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZRCore");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "core");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Game mode-independent stuff.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRCore = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRCore, "Event_OnEventsRegister",              "ZRCore_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRCore_OnEventsRegister()
{
    // Register all the events needed for this module.
    
    #if defined PROJECT_GAME_CSS
    
    #endif
}

/**
 * Plugin is loading.
 */
ZRCore_OnPluginStart()
{
    // Register the module.
    ZRCore_Register();
}
