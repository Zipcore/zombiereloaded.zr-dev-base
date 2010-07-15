/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          gameplay.zr.inc
 *  Type:          Module
 *  Description:   Modifies game behaviors to fit classic ZR gameplay.
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
new Module:g_moduleZRGPlay;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRGPlay_GetIdentifier() { return g_moduleZRGPlay; }

/**
 * Register this module.
 */
ZRGPlay_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "ZR Gameplay Modifier");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "zrgameplay");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Modifies game behaviors to fit classic ZR gameplay.");
    moduledata[ModuleData_Dependencies][0] = ZR_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRGPlay = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnEventsRegister",         "ZRGPlay_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRGPlay_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnMyModuleEnable",        "ZRGPlay_OnMyModuleEnable");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnMyModuleDisable",       "ZRGPlay_OnMyModuleDisable");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_RoundStart",              "ZRGPlay_RoundStart");
    
    #endif
}

/**
 * Plugin is loading.
 */
ZRGPlay_OnPluginStart()
{
    // Register the module.
    ZRGPlay_Register();
    
    #if defined PROJECT_GAME_CSS
        HookConVarChange(FindConVar("mp_restartgame"), ZRGPlay_CvarHookMPRGame);
        CvarLib_LockCvar("mp_limitteams", "0");
        CvarLib_LockCvar("mp_autoteambalance", "0");
    #endif
}

/**
 * The module that hooked this event callback has been enabled.
 */
public ZRGPlay_OnMyModuleEnable()
{
    #if defined PROJECT_GAME_CSS
        HookConVarChange(FindConVar("mp_restartgame"), ZRGPlay_CvarHookMPRGame);
        CvarLib_LockCvar("mp_limitteams", "0");
        CvarLib_LockCvar("mp_autoteambalance", "0");
    #endif
}

/**
 * The module that hooked this event callback has been disabled.
 * 
 * @return      Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public ZRGPlay_OnMyModuleDisable()
{
    #if defined PROJECT_GAME_CSS
        UnhookConVarChange(FindConVar("mp_restartgame"), ZRGPlay_CvarHookMPRGame);
        CvarLib_UnlockCvar("mp_limitteams");
        CvarLib_UnlockCvar("mp_autoteambalance");
    #endif
}

// **********************************************
//            Cvar Hook Callbacks
// **********************************************

#if defined PROJECT_GAME_CSS

/**
 * Called when a console variable's value is changed.
 * 
 * @param convar		Handle to the convar that was changed.
 * @param oldValue		String containing the value of the convar before it was changed.
 * @param newValue		String containing the new value of the convar.
 */
public ZRGPlay_CvarHookMPRGame(Handle:convar, const String:oldValue[], const String:newValue[])
{
    // If the value is 0 then ignore, game isn't restarting.
    if (StringToInt(newValue) == 0)
        return;
    
    // End round with "Game commencing"
    GameData_TerminateRound(0);
    
    // This basically makes mp_restartgame useless.
    SetConVarInt(convar, 0);
}

#endif

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 */
public ZRGPlay_RoundStart()
{
    // Print ZR objective.
    TransMgr_PrintTextAll(true, false, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR round objective");
}

#endif
