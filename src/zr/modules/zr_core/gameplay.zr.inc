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
 * TODO:
 * Unhook cvar hooks when module is disabled.
 * Waiting for SM to not error when i do this on OnPluginEnd
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
 * Timer handle for counting down the round time.
 */
new Handle:g_hRoundTimer;

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
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnEventsReady",              "ZRGPlay_OnEventsReady");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnMyModuleEnable",           "ZRGPlay_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnMyModuleDisable",          "ZRGPlay_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnMapStart",                 "ZRGPlay_OnMapStart");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_OnClientInfect",             "ZRGPlay_OnClientInfect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_RoundStart",                 "ZRGPlay_RoundStart");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_RoundFreezeEnd",             "ZRGPlay_RoundFreezeEnd");
    EventMgr_RegisterEvent(g_moduleZRGPlay, "Event_RoundEnd",                   "ZRGPlay_RoundEnd");
    
    #endif
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public ZRGPlay_OnEventsReady()
{
    // Push down priority for Event_OnClientInfect to below zrteammanager module.
    EventMgr_TakePriority("Event_OnClientInfect", g_moduleZRGPlay, ZRTM_GetIdentifier());
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

/**
 * The map has started.
 */
public ZRGPlay_OnMapStart()
{
    // Reset timer handle.
    g_hRoundTimer = INVALID_HANDLE;
}

/**
 * Client has been infected.
 * 
 * @param client        The infected client.
 * @param attacker      The zombie that infected the client.
 * @param mzombie       True if the client has been infected as a mother zombie.
 */
public ZRGPlay_OnClientInfect(client, attacker, bool:mzombie)
{
    // The core won't infect all clients as mother zombie so don't waste time checking.
    // This could be a big performance boost if 10 mother zombies are being spawned simultaneously.
    if (mzombie)
        return;
    
    // If there are no more humans then zombies win.
    if (TLib_CountTeam(ZTeam_Human, true) == 0)
    {
        SDKToolsLib_TerminateRound(3.0, ROUNDEND_TERRORISTS_WIN);
    }
}

/**
 * Client has turned back to human.
 * 
 * @param client        The client that became human.
 */
public ZRGPlay_OnClientHuman(client)
{
    // If there are no more humans then humans win.
    if (TLib_CountTeam(ZTeam_Zombie, true) == 0)
    {
        SDKToolsLib_TerminateRound(3.0, ROUNDEND_CTS_WIN);
    }
}

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 */
public ZRGPlay_RoundStart()
{
    // Print ZR objective.
    TransMgr_PrintTextAll(true, false, MsgFormat_Plugin, MsgType_Chat, _, _, "ZR gameplay round objective");
    
    // Stop round timer.
    Util_CloseHandle(g_hRoundTimer);
}

/**
 * Pre-round freezetime has finished.
 */
public ZRGPlay_RoundFreezeEnd()
{
    // Stop round timer.
    Util_CloseHandle(g_hRoundTimer);
    
    // Get the roundtime, and save it for future calls.
    static Handle:hMPRoundTime;
    if (hMPRoundTime == INVALID_HANDLE)
    {
        hMPRoundTime = FindConVar("mp_roundtime");
    }
    
    g_hRoundTimer = CreateTimer(GetConVarFloat(hMPRoundTime) * 60.0, ZRGPlay_RoundEndTimer, _, TIMER_FLAG_NO_MAPCHANGE);
}

/**
 * Timer callback.
 * End the round, humans win.
 * 
 * @param timer     The timer handle.
 */
public Action:ZRGPlay_RoundEndTimer(Handle:timer)
{
    g_hRoundTimer = INVALID_HANDLE;
    
    // Humans held out against the zombies, they win!
    SDKToolsLib_TerminateRound(3.0, ROUNDEND_CTS_WIN);
}

/**
 * Round has ended.
 */
public ZRGPlay_RoundEnd()
{
    // Stop round timer.
    Util_CloseHandle(g_hRoundTimer);
}

#endif

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
    new time = StringToInt(newValue);
    if (time == 0)
        return;
    
    // End round with "Game commencing"
    SDKToolsLib_TerminateRound(float(time), ROUNDEND_GAME_COMMENCING);
    
    // This basically makes mp_restartgame useless.
    SetConVarInt(convar, 0);
}

#endif
