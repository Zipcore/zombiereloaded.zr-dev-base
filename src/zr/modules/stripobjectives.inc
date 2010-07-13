/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          stripobjectives.inc
 *  Type:          Module
 *  Description:   
 *
 *  Copyright (C) 2009-2010  Greyscale
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
 * A list delimited by "|" of objective entities, specific to each game.
 */

#if defined PROJECT_GAME_CSS
    #define STRIPOBJ_ENTITIES "func_bomb_target|func_hostage_rescue|c4|hostage_entity"
#endif

#if defined PROJECT_GAME_TF2
    #define STRIPOBJ_ENTITIES "idontknowtheobjectiveentities"
#endif

/**
 * This module's identifier.
 */
new Module:g_moduleStripObj;

/**
 * Cvar handles.
 */
new Handle:g_hCvarStripObjectives;

/**
 * Register this module.
 */
StripObj_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Objective Stripper");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "objstripper");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Strips objectives from all maps.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleStripObj = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleStripObj, "Event_OnMyModuleDisable",     "StripObj_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleStripObj, "Event_RoundStart",            "StripObj_RoundStart");
}

/**
 * Plugin is loading.
 */
StripObj_OnPluginStart()
{
    // Register the module.
    StripObj_Register();
    
    // Create cvars.
    g_hCvarStripObjectives = Project_CreateConVar("strip_objectives", "1", "Removes all objectives from the map.");
}

/**
 * The module that hooked this event callback has been disabled.
 */
public StripObj_OnMyModuleDisable()
{
    TransMgr_PrintText(SERVER_INDEX, MsgFormat_Both, MsgType_Server, g_moduleStripObj, false, "ObjStrip disable");
}

/**
 * Round has started.
 */
public StripObj_RoundStart()
{
    // Don't strip objectives if the cvar is disabled.
    if (!GetConVarBool(g_hCvarStripObjectives))
        return;
    
    // Get max entity count.
    decl String:classname[64];
    new maxentities = GetMaxEntities();
    for (new entity = 0; entity <= maxentities; entity++)
    {
        if(!IsValidEntity(entity))
            continue;
        
        // Get valid edict's classname.
        GetEdictClassname(entity, classname, sizeof(classname));
        
        // If the entity is within the objective entity string, then kill it.
        if(StrContains(STRIPOBJ_ENTITIES, classname) > -1)
            RemoveEdict(entity);
    }
}