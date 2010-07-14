/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          gamedata.inc
 *  Type:          Module
 *  Description:   Handles virtual offsets and signatures.
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
 * General module defines.
 */
#define GAMEDATA_FILE "plugin.zombiereloaded"

/**
 * This module's identifier.
 */
//new Module:g_moduleGameData;


/**
 * Gamedata handles.
 */
#if defined PROJECT_GAME_CSS
new Handle:g_hGameConfig;
new Handle:g_hTerminateRound;
new Handle:g_hCSWeaponDrop;
new Handle:g_hFlashlightIsOn;
new Handle:g_hFlashlightTurnOn;
new Handle:g_hFlashlightTurnOff;
#endif

/**
 * Register this module.
 */
GameData_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "GameData");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "gamedata");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Handles virtual offsets and signatures.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    /*g_moduleGameData = */
    ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    
}

/**
 * Plugin is loading.
 */
GameData_OnPluginStart()
{
    // Register the module.
    GameData_Register();
    
    // TF2 doesn't need a signature to end the round.
    #if defined PROJECT_GAME_CSS
    
    // Load game config file.
    g_hGameConfig = LoadGameConfigFile(GAMEDATA_FILE);
    
    // If gamedata file can't be loaded, then stop the plugin.
    if (g_hGameConfig == INVALID_HANDLE)
    {
        SetFailState("Can't load game config file (%s) from the \"gamedata\" directory.", GAMEDATA_FILE);
    }
    
    // Prep the SDKCall for "TerminateRound."
    StartPrepSDKCall(SDKCall_GameRules);
    PrepSDKCall_SetFromConf(g_hGameConfig, SDKConf_Signature, "TerminateRound");
    PrepSDKCall_AddParameter(SDKType_Float, SDKPass_Plain);
    PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
    g_hTerminateRound = EndPrepSDKCall();
    
    // If offset "TerminateRound" can't be found, then stop the plugin.
    if(g_hTerminateRound == INVALID_HANDLE)
    {
        SetFailState("Signature \"CGameRules::TerminateRound\" was not found.");
    }
    
    // Prep the SDKCall for "CSWeaponDrop."
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(g_hGameConfig, SDKConf_Signature, "CSWeaponDrop");
    PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
    PrepSDKCall_AddParameter(SDKType_Bool, SDKPass_Plain);
    PrepSDKCall_AddParameter(SDKType_Bool, SDKPass_Plain);
    g_hCSWeaponDrop = EndPrepSDKCall();
    
    // If offset "CSWeaponDrop" can't be found, then stop the plugin.
    if(g_hCSWeaponDrop == INVALID_HANDLE)
    {
        SetFailState("Signature \"CBasePlayer::CSWeaponDrop\" was not found.");
    }
    
    // Prep the SDKCall for "FlashlightIsOn"
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(g_hGameConfig, SDKConf_Virtual, "FlashlightIsOn");
    PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_Plain);
    g_hFlashlightIsOn = EndPrepSDKCall();
    
    // If offset "FlashlightIsOn" can't be found, then stop the plugin.
    if(g_hFlashlightIsOn == INVALID_HANDLE)
    {
        
        SetFailState("Signature \"CCSPlayer::FlashlightIsOn\" was not found.");
    }
    
    // Prep the SDKCall for "FlashlightTurnOn"
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(g_hGameConfig, SDKConf_Virtual, "FlashlightTurnOn");
    g_hFlashlightTurnOn = EndPrepSDKCall();
    
    // If offset "FlashlightTurnOn" can't be found, then stop the plugin.
    if(g_hFlashlightTurnOn == INVALID_HANDLE)
    {
        SetFailState("Signature \"CCSPlayer::FlashlightTurnOn\" was not found.");
    }
    
    // Prep the SDKCall for "FlashlightTurnOff"
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(g_hGameConfig, SDKConf_Virtual, "FlashlightTurnOff");
    g_hFlashlightTurnOff = EndPrepSDKCall();
    
    // If offset "FlashlightTurnOff" can't be found, then stop the plugin.
    if(g_hFlashlightTurnOff == INVALID_HANDLE)
    {
        SetFailState("Signature \"CCSPlayer::FlashlightTurnOff\" was not found.");
    }
    
    #endif
}

#if defined PROJECT_GAME_CSS

/**
 * All round end reasons.
 * Used an enum instead of defines so the include order doesn't matter.
 */
enum RoundEndReasons
{
    ROUNDEND_TARGET_BOMBED = 0,         /** Target Successfully Bombed! */
    ROUNDEND_VIP_ESCAPED,               /** The VIP has escaped! */
    ROUNDEND_VIP_ASSASSINATED,          /** VIP has been assassinated! */
    ROUNDEND_TERRORISTS_ESCAPED,        /** The terrorists have escaped! */
    ROUNDEND_CTS_PREVENTESCAPE,         /** The CT's have prevented most of the terrorists from escaping! */
    ROUNDEND_ESC_TS_NEUTRALIZED,        /** Escaping terrorists have all been neutralized! */
    ROUNDEND_BOMB_DEFUSED,              /** The bomb has been defused! */
    ROUNDEND_CTS_WIN,                   /** Counter-Terrorists Win! */
    ROUNDEND_TERRORISTS_WIN,            /** Terrorists Win! */
    ROUNDEND_ROUND_DRAW,                /** Round Draw! */
    ROUNDEND_ALL_HOSTAGES_RESCUED,      /** All Hostages have been rescued! */
    ROUNDEND_TARGET_SAVED,              /** Target has been saved! */
    ROUNDEND_HOSTAGES_NOT_RESCUED,      /** Hostages have not been rescued! */
    ROUNDEND_TERRORISTS_NOT_ESCAPED,    /** Terrorists have not escaped! */
    ROUNDEND_VIP_NOT_ESCAPED,           /** VIP has not escaped! */
    ROUNDEND_GAME_COMMENCING            /** Game Commencing! */
}

/**
 * Takes a winning team index and returns its corresponding round end reason.
 * Ex: Takes index '2' and returns the Terrorists_Win round end reason.
 * 
 * @param teamindex     The team index that won the round.
 * 
 * @return              The round end reason.  ROUNDEND_GAME_COMMENCING if invalid teamindex.
 */
stock RoundEndReasons:Util_TeamToReason(teamindex)
{
    if (teamindex == TEAM_2)
        return ROUNDEND_TERRORISTS_WIN;
    
    else if (teamindex == TEAM_3)
        return ROUNDEND_CTS_WIN;
    
    return ROUNDEND_GAME_COMMENCING;
}

/**
 * Terminates the round. (CS:S)
 * 
 * @team    The team index of the winner.
 */
stock GameData_TerminateRound(teamindex)
{
    new RoundEndReasons:reason = Util_TeamToReason(teamindex);
    
    // Delay is always 5 seconds for any reason except when the game is commencing.
    new Float:delay;
    if (reason != ROUNDEND_GAME_COMMENCING)
        delay = 5.0;
    else
        delay = 3.0;
    
    SDKCall(g_hTerminateRound, delay, _:reason);
}

/**
 * Forces client to drop a weapon. (CS:S)
 * 
 * @param client    The client index.
 * @param weapon    The weapon entity index to force client to drop.
 */
stock GameData_CSDropWeapon(client, weapon)
{
    SDKCall(g_hCSWeaponDrop, client, weapon, true, false);
}

/**
 * Check if the client's flashlight is turned on. (CS:S)
 * 
 * @param client    The client index.
 * 
 * @return          True if the light is on, false if off.
 */
stock bool:GameData_FlashlightIsOn(client)
{
    return bool:SDKCall(g_hFlashlightIsOn, client);
}

/**
 * Force a client's flashlight on. (CS:S)
 * 
 * @param client    The client index.
 */
stock GameData_FlashlightTurnOn(client)
{
    SDKCall(g_hFlashlightTurnOn, client);
}

/**
 * Force a client's flashlight off. (CS:S)
 * 
 * @param client    The client index.
 */
stock GameData_FlashlightTurnOff(client)
{
    SDKCall(g_hFlashlightTurnOff, client);
}

#endif

#if defined PROJECT_GAME_TF2

/**
 * Terminates the round. (TF2)
 * Credits to toazron1 for this one :)
 * 
 * @team    The team index of the winner.
 */
stock GameData_TerminateRound(teamindex)
{
    new ent = FindEntityByClassname(-1, "team_control_point_master");
    if (ent == -1)
    {
        ent = CreateEntityByName("team_control_point_master");
        DispatchSpawn(ent);
        AcceptEntityInput(ent, "Enable");
    }
    
    SetVariantInt(teamindex);
    AcceptEntityInput(ent, "SetWinner");
}

#endif
