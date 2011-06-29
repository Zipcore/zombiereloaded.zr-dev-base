/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          soundfx.inc
 *  Type:          Module
 *  Description:   Sound effects!
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
 * TODO
 * this needs priority in OnMapStart over any module using it in their configs.
 */

/**
 * The path to this module's sound profiles config file, relative to the path defined in gamerules.txt.
 */
#define SOUNDFX_CONFIG_FILE "soundprofiles.txt"

/**
 * This module's identifier.
 */
new Module:g_moduleSoundFX;

/**
 * Handle to sound profile cache.
 */
new Handle:g_hSoundFXCache;

/**
 * Register this module.
 */
SoundFX_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Sound Effects");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "soundfx");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Sound effects!");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleSoundFX = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleSoundFX, "Event_OnEventsRegister",       "SoundFX_OnEventsRegister");
    
    // Register config file(s) that this module will use.
    ConfigMgr_Register(g_moduleSoundFX, "SoundFX_OnConfigReload", "");
}

/**
 * Register all events here.
 */
public SoundFX_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleSoundFX, "Event_OnPluginEnd",                "SoundFX_OnPluginEnd");
    EventMgr_RegisterEvent(g_moduleSoundFX, "Event_OnMyModuleEnable",           "SoundFX_OnMyModuleEnable");
    EventMgr_RegisterEvent(g_moduleSoundFX, "Event_OnMyModuleDisable",          "SoundFX_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleSoundFX, "Event_OnMapStart",                 "SoundFX_OnMapStart");
    
    #if defined PROJECT_GAME_CSS
    
    #endif
}

/**
 * Plugin is loading.
 */
SoundFX_OnPluginStart()
{
    // Register the module.
    SoundFX_Register();
    
    // Create array.
    g_hSoundFXCache = CreateArray();
}

/**
 * Plugin is ending.
 */
public SoundFX_OnPluginEnd()
{
    // Cleanup data stored by the sound effects module.
    SoundFX_CleanCache();
    CloseHandle(g_hSoundFXCache);
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @param refusalmsg    The string that is printed if Plugin_Handled is returned and it is non-empty.
 * @param maxlen        The max length of the string.
 * 
 * @return              Return Plugin_Handled to stop enable, and Plugin_Continue to allow it.
 */
public Action:SoundFX_OnMyModuleEnable(String:refusalmsg[], maxlen)
{
    // Create array.
    g_hSoundFXCache = CreateArray();
    
    // Don't let the module load unless the sound profiles are cached successfully.
    if (!SoundFX_CacheProfiles())
    {
        Format(refusalmsg, maxlen, "%T", "SoundFX refuse enable", LANG_SERVER);
        return Plugin_Handled;
    }
    
    return Plugin_Continue;
}

/**
 * The module that hooked this event callback has been disabled.
 * 
 * @param refusalmsg    The string that is printed if Plugin_Handled is returned and it is non-empty.
 * @param maxlen        The max length of the string.
 * 
 * @return              Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public Action:SoundFX_OnMyModuleDisable(String:refusalmsg[], maxlen)
{
    // Cleanup data stored by the sound effects module.
    SoundFX_CleanCache();
    CloseHandle(g_hSoundFXCache);
}

/**
 * The map has started.
 */
public SoundFX_OnMapStart()
{
    SoundFX_CacheProfiles();
}

/**
 * Called when a registered config file (by this module) is manually reloaded.
 */
public SoundFX_OnConfigReload(configindex)
{
    SoundFX_CacheProfiles();
}

/**
 * Destroys all profile data from the cache.
 */
static stock SoundFX_CleanCache()
{
    // Cleanup data stored by the sound effects module.
    new count = GetArraySize(g_hSoundFXCache);
    for (new i = 0; i < count; i++)
        CloseHandle(GetArrayCell(g_hSoundFXCache, i));
}

/**
 * Loops through each section of the keyvalues tree.
 * Note: ADT array in 'g_moduleSoundFX' should be cleared before using this.
 * 
 * @param kv            The keyvalues handle of the config file. (Don't close this)
 * @param sectionindex  The index of the current keyvalue section, starting from 0.
 * @param sectionname   The name of the current keyvalue section.
 * 
 * @return              See enum KvCache.
 */
public KvCache:SoundFX_Cache(Handle:kv, sectionindex, const String:sectionname[])
{
    if (!KvGotoFirstSubKey(kv, false))
    {
        LogMgr_Print(g_moduleSoundFX, LogType_Error, "Config Validation", "Empty or invalid sound profile: \"%s\"", sectionname);
        return KvCache_Ignore;
    }
    
    new Handle:hSoundProfile = CreateArray(PLATFORM_MAX_PATH);
    PushArrayString(hSoundProfile, sectionname);
    
    decl String:path[PLATFORM_MAX_PATH];
    decl String:fullpath[PLATFORM_MAX_PATH];
    do
    {
        // Get sound file reference.
        KvGetString(kv, "path", path, sizeof(path));
        
        Format(fullpath, sizeof(fullpath), "sound/%s", path);
        if (FileExists(fullpath, true))
        {
            PushArrayString(hSoundProfile, path);
            AddFileToDownloadsTable(fullpath);
        }
        else
        {
            LogMgr_Print(g_moduleSoundFX, LogType_Error, "Config Validation", "Invalid sound file specified in sound profile \"%s\": \"%s\"", sectionname, path);
            continue;
        }
    } while (KvGotoNextKey(kv, false));
    KvGoBack(kv);
    
    PushArrayCell(g_hSoundFXCache, hSoundProfile);
    return KvCache_Continue;
}

/**
 * Re-cache all sound profiles from disk.
 * 
 * @return      True if cached successfully, false if the file couldn't be found or no usable data was inside.
 */
bool:SoundFX_CacheProfiles()
{
    // Get the config path from the gamerules module.
    decl String:configfile[PLATFORM_MAX_PATH];
    GameRules_GetNamedConfig("soundprofiles", configfile);
    
    if (ConfigMgr_ValidateFile(configfile))
        ConfigMgr_WriteString(g_moduleSoundFX, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, configfile);
    else
    {
        LogMgr_Print(g_moduleSoundFX, LogType_Fatal_Module, "Config Validation", "Invalid config file path defined in gamerules: \"%s\".  Disabling module.", configfile);
        return false;
    }
    
    SoundFX_CleanCache();
    ClearArray(g_hSoundFXCache);
    ConfigMgr_CacheKv(g_moduleSoundFX, CM_CONFIGINDEX_FIRST, "SoundFX_Cache");
    
    return true;
}

/**
 * Find a profile index based off its name.
 * 
 * @param profile   The name of the profile
 * 
 * @return          The index this profile is at in the array.  -1 if doesn't exist.
 */
static stock SoundFX_GetProfileIndex(const String:profile[])
{
    decl String:profilename[64];
    new count = GetArraySize(g_hSoundFXCache);
    for (new i = 0; i < count; i++)
    {
        GetArrayString(GetArrayCell(g_hSoundFXCache, i), 0, profilename, sizeof(profilename)); // Get the name of this profile at index 0.
        if (StrEqual(profile, profilename, false))
            return i;
    }
    return -1;
}

/**
 * Get a sound from a given profile and index.
 * 
 * @param profile   Sound profile get sound from.
 * @param index     The index of the sound.
 * @param sound     Output buffer for the sound path.
 * @param maxlen    Max length of the buffer.
 * 
 * @return          True if found, false otherwise.
 */
static stock bool:SoundFX_GetProfileSound(const String:profile[], index, String:sound[], maxlen)
{
    new pindex = SoundFX_GetProfileIndex(profile);
    if (pindex == -1)
        return false;
    
    new Handle:hSoundProfile = GetArrayCell(g_hSoundFXCache, pindex);
    if (index < 0 || index >= GetArraySize(hSoundProfile)-1)
        return false;
    
    GetArrayString(hSoundProfile, index, sound, maxlen);
    return true;
}

/**
 * Get a random sound from a profile.
 * 
 * @param profile   Profile to get random sound from.
 * @param sound     Output buffer for the sound path.
 * @param maxlen    Max length of the buffer.
 * 
 * @return          True if succeeded, false otherwrise.
 */
stock bool:SoundFX_GetRandProfileSound(const String:profile[], String:sound[], maxlen)
{
    new pindex = SoundFX_GetProfileIndex(profile);
    if (pindex == -1)
        return false;
    
    new Handle:hSoundProfile = GetArrayCell(g_hSoundFXCache, pindex);
    new randsindex = Math_GetRandomInt(1, GetArraySize(hSoundProfile)-1);
    GetArrayString(hSoundProfile, randsindex, sound, maxlen);
    return true;
}
