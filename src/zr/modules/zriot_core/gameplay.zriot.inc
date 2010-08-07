/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          gameplay.zriot.inc
 *  Type:          Module
 *  Description:   Modifies game behaviors to fit ZRiot gameplay.
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
new Module:g_moduleZRiotGPlay;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ZRiotGPlay_GetIdentifier() { return g_moduleZRiotGPlay; }

/**
 * Cvar handles.
 */
new Handle:g_hCvarObeyRoundTime;

/**
 * Timer handle for counting down the round time.
 */
new Handle:g_hZRiotRoundTimer;

/**
 * Register this module.
 */
ZRiotGPlay_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "ZRiot Gameplay Modifier");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "zriotgameplay");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Modifies game behaviors to fit ZRiot gameplay.");
    moduledata[ModuleData_Dependencies][0] = ZRiot_GetIdentifier();
    moduledata[ModuleData_Dependencies][1] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleZRiotGPlay = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnEventsRegister",         "ZRiotGPlay_OnEventsRegister");
}

/**
 * Register all events here.
 */
public ZRiotGPlay_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnEventsReady",           "ZRiotGPlay_OnEventsReady");
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnMyModuleEnable",        "ZRiotGPlay_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnMyModuleDisable",       "ZRiotGPlay_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnMapStart",              "ZRiotGPlay_OnMapStart");
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_OnClientInfect",          "ZRiotGPlay_OnClientInfect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_RoundStart",              "ZRiotGPlay_RoundStart");
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_RoundFreezeEnd",          "ZRiotGPlay_RoundFreezeEnd");
    EventMgr_RegisterEvent(g_moduleZRiotGPlay, "Event_RoundEnd",                "ZRiotGPlay_RoundEnd");
    
    #endif
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public ZRiotGPlay_OnEventsReady()
{
    // Push down priority for Event_OnClientInfect to below zrteammanager module.
    EventMgr_TakePriority("Event_OnClientInfect", g_moduleZRiotGPlay, ZRiotTM_GetIdentifier());
}

/**
 * Plugin is loading.
 */
ZRiotGPlay_OnPluginStart()
{
    // Register the module.
    ZRiotGPlay_Register();
    
    // Create cvars.
    g_hCvarObeyRoundTime = CreateConVar("zriot_obey_roundtime", "0", "When the time set by mp_roundtime expires, then zombies win.  Otherwise, the only way to win is to eliminate the opposing team.");
    
    #if defined PROJECT_GAME_CSS
        HookConVarChange(FindConVar("mp_restartgame"), ZRiotGPlay_CvarHookMPRGame);
        CvarLib_LockCvar("mp_limitteams", "0");
        CvarLib_LockCvar("mp_autoteambalance", "0");
    #endif
}

/**
 * The module that hooked this event callback has been enabled.
 */
public ZRiotGPlay_OnMyModuleEnable()
{
    #if defined PROJECT_GAME_CSS
        HookConVarChange(FindConVar("mp_restartgame"), ZRiotGPlay_CvarHookMPRGame);
        CvarLib_LockCvar("mp_limitteams", "0");
        CvarLib_LockCvar("mp_autoteambalance", "0");
    #endif
}

/**
 * The module that hooked this event callback has been disabled.
 * 
 * @return      Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public ZRiotGPlay_OnMyModuleDisable()
{
    #if defined PROJECT_GAME_CSS
        UnhookConVarChange(FindConVar("mp_restartgame"), ZRiotGPlay_CvarHookMPRGame);
        CvarLib_UnlockCvar("mp_limitteams");
        CvarLib_UnlockCvar("mp_autoteambalance");
    #endif
}

/**
 * The map has started.
 */
public ZRiotGPlay_OnMapStart()
{
    // Reset timer handle.
    g_hZRiotRoundTimer = INVALID_HANDLE;
}

/**
 * Client has been infected.
 * 
 * @param client        The infected client.
 * @param attacker      The zombie that infected the client.
 * @param mzombie       True if the client has been infected as a mother zombie.
 */
public ZRiotGPlay_OnClientInfect(client, attacker, bool:mzombie)
{
    // The core won't infect all clients as mother zombie so don't waste time checking.
    // This could be a big performance boost if 10 mother zombies are being spawned simultaneously.
    if (mzombie)
        return;
    
    // If there are no more humans then zombies win.
    if (TLib_CountTeam(VTeam_Human, true) == 0)
    {
        SDKToolsLib_TerminateRound(3.0, Util_TeamToReason(TLib_GetGameTeamIndex(VTeam_Zombie)));
    }
}

/**
 * Client has turned back to human.
 * 
 * @param client        The client that became human.
 */
public ZRiotGPlay_OnClientHuman(client)
{
    // If there are no more humans then humans win.
    if (TLib_CountTeam(VTeam_Zombie, true) == 0)
    {
        SDKToolsLib_TerminateRound(3.0, Util_TeamToReason(TLib_GetGameTeamIndex(VTeam_Human)));
    }
}

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 */
public ZRiotGPlay_RoundStart()
{
    // Print ZRiot objective.
    TransMgr_PrintTextAll(true, false, MsgFormat_Plugin, MsgType_Chat, _, _, "ZRiot gameplay round objective");
    
    // Stop round timer.
    Util_CloseHandle(g_hZRiotRoundTimer);
}

/**
 * Pre-round freezetime has finished.
 */
public ZRiotGPlay_RoundFreezeEnd()
{
    // Stop round timer.
    Util_CloseHandle(g_hZRiotRoundTimer);
    
    // Start round timer if it's enabled.
    if (GetConVarBool(g_hCvarObeyRoundTime))
    {
        // Get the roundtime, and save it for future calls.
        static Handle:hMPRoundTime;
        if (hMPRoundTime == INVALID_HANDLE)
        {
            hMPRoundTime = FindConVar("mp_roundtime");
        }
        
        g_hZRiotRoundTimer = CreateTimer(GetConVarFloat(hMPRoundTime) * 60.0, ZRiotGPlay_RoundEndTimer, _, TIMER_FLAG_NO_MAPCHANGE);
    }
}

/**
 * Timer callback.
 * End the round, humans win.
 * 
 * @param timer     The timer handle.
 */
public Action:ZRiotGPlay_RoundEndTimer(Handle:timer)
{
    g_hZRiotRoundTimer = INVALID_HANDLE;
    
    // Humans held out against the zombies, they win!
    SDKToolsLib_TerminateRound(3.0, Util_TeamToReason(TLib_GetGameTeamIndex(VTeam_Zombie)));
}

/**
 * Round has ended.
 */
public ZRiotGPlay_RoundEnd()
{
    // Stop round timer.
    Util_CloseHandle(g_hZRiotRoundTimer);
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
public ZRiotGPlay_CvarHookMPRGame(Handle:convar, const String:oldValue[], const String:newValue[])
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
