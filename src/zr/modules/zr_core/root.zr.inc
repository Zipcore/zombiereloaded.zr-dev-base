/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          root.zr.inc
 *  Type:          Module
 *  Description:   Root module for the ZR core.  All ZR-specific modules depend on this.
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
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZR_GetIdentifier() { return g_moduleZRRoot; }

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
    EventMgr_RegisterEvent(g_moduleZRRoot, "Event_OnMyModuleEnable",        "ZR_OnMyModuleEnable");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRRoot, "Event_RoundStart",            "ZR_RoundStart");
    
    #endif
}

// Include sub-modules.
#include "zr/modules/zr_core/weapons.zr.inc"
#include "zr/modules/zr_core/infection.zr.inc"
#include "zr/modules/zr_core/teammanager.zr.inc"
#include "zr/modules/zr_core/damage.zr.inc"

/**
 * Plugin is loading.
 */
ZR_OnPluginStart()
{
    // Register the module.
    ZR_Register();
    
    // Forward event to sub-modules.
    ZRWeapons_OnPluginStart();  // This module's RoundEnd needs to fire before ZRGamePlay module's.
    ZRInfect_OnPluginStart();
    ZRTM_OnPluginStart();
    ZRDmg_OnPluginStart();
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @return      Return Plugin_Handled to stop load, and Plugin_Continue to allow it.
 */
public ZR_OnMyModuleEnable()
{
    // Enable core ZR modules.
    new Module:dependentmodules[CM_DATA_DEPENDENCIES];
    new count = ModuleMgr_FindDependentModules(g_moduleZRRoot, dependentmodules);
    for (new dmoduleindex = 0; dmoduleindex < count; dmoduleindex++)
        ModuleMgr_Enable(dependentmodules[dmoduleindex]);
}

/**
 * Round has started.
 */
public ZR_RoundStart()
{
    // Print ZR objective.
    TransMgr_PrintTextAll(true, false, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR round objective");
}
