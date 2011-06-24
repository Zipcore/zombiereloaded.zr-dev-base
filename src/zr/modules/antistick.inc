/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          antistick.inc
 *  Type:          Module
 *  Description:   Detects collisions between players and unsticks them.
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
new Module:g_moduleAntiStick;

/**
 * Default player hull width.
 */
#define MODEL_HULL_LENGTH 32.0

/**
 * List of components that make up rectangular boundaries.
 * 
 * F = Front
 * B = Back
 * L = Left
 * R = Right
 * U = Upper
 * D = Down
 */
enum AntiStickBoxBound
{
    BoxBound_FUR = 0, /** Front upper right */
    BoxBound_FUL,     /** etc.. */
    BoxBound_FDR,
    BoxBound_FDL,
    BoxBound_BUR,
    BoxBound_BUL,
    BoxBound_BDR,
    BoxBound_BDL,
}

/**
 * Register this module.
 */
AntiStick_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = true;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Anti-Stick");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "antistick");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Detects collisions between players and unsticks them.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleAntiStick = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleAntiStick, "Event_OnEventsRegister",         "AntiStick_OnEventsRegister");
}

/**
 * Register all events here.
 */
public AntiStick_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleAntiStick, "Hook_StartTouch",                "AntiStick_StartTouch");
    
    #if defined PROJECT_GAME_CSS
    
    #endif
}

/**
 * Plugin is loading.
 */
AntiStick_OnPluginStart()
{
    // Register the module.
    AntiStick_Register();
}

/**
 * Called when the hooked entity touches another entity.
 * 
 * @param client    The hooked client.
 * @param other     The entity thats being touched by the client.
 * 
 * @noreturn
 */
public AntiStick_StartTouch(client, entity)
{
    if (Client_IsIngame(entity))
    {
        //new Float:vecMins[3] = {1.0, 1.0, 1.0};
        //new Float:vecMaxs[3] = {2.0, 2.0, 2.0};
        //Entity_SetMinMaxSize(client, vecMins, vecMaxs);
        //Entity_GetMaxSize(client, vecMins);
        //PrintToServer("%f %f %f",vecMins[0],vecMins[1],vecMins[2]);
    }
}
