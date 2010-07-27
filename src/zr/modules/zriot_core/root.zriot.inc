/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          root.zriot.inc
 *  Type:          Module
 *  Description:   Root module for the ZRiot core.  All ZRiot core modules depend on this.
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
new Module:g_moduleZRiotRoot;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRiot_GetIdentifier() { return g_moduleZRiotRoot; }

/**
 * Register this module.
 */
ZRiot_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "ZRiot Root");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zriot_root");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Top of the chain module for ZRiot's set of core modules.  This can not be enabled/disabled by anything other than game rules.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    moduledata[ModuleData_Root] = true;
    
    // Send this array of data to the module manager.
    g_moduleZRiotRoot = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRiotRoot, "Event_OnEventsRegister",         "ZRiot_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRiot_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRiotRoot, "Event_OnMyModuleEnable",         "ZRiot_OnMyModuleEnable");
}

// Include sub-modules.

/**
 * Plugin is loading.
 */
ZRiot_OnPluginStart()
{
    // Register the module.
    ZRiot_Register();
    
    // Forward event to sub-modules.
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @return      Return Plugin_Handled to stop load, and Plugin_Continue to allow it.
 */
public ZRiot_OnMyModuleEnable()
{
    // Enable core ZR modules.
    new Module:dependentmodules[CM_DATA_DEPENDENCIES];
    new count = ModuleMgr_FindDependentModules(g_moduleZRiotRoot, dependentmodules);
    for (new dmoduleindex = 0; dmoduleindex < count; dmoduleindex++)
        ModuleMgr_Enable(dependentmodules[dmoduleindex]);
}