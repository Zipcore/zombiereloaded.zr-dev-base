/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          weapons.inc
 *  Type:          Module
 *  Description:   Weapon control.
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
new Module:g_moduleWeapons;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:Weapons_GetIdentifier() { return g_moduleWeapons; }

/**
 * Cvar handles.
 */
new Handle:g_hCvarWeaponsConfigFile;

/**
 * The max string lengths of weapon config data variables.
 */
#define WEPCONFIG_NAME 32
#define WEPCONFIG_ENTITY 64
#define WEPCONFIG_TYPE 128
#define WEPCONFIG_AMMOTYPE 64

/**
 * The longest string that can be stored in the weapon's data.
 */
#define WEAPON_DATA_LONGEST_STRING 128

/**
 * Weapon config data indexes.
 */
enum WepConfigs
{
    // Values cached from config file.
    String:WepConfig_Name[WEPCONFIG_NAME] = 0,      /** Name to reference the weapon by. */
    String:WepConfig_Entity[WEPCONFIG_ENTITY],      /** The entity classname of the weapon. */
    String:WepConfig_Type[WEPCONFIG_TYPE],          /** Custom weapon categorization, delimited by "," */
    WepLib_Slots:WepConfig_Slot,                    /** The slot the weapon resides in. */
    String:WepConfig_AmmoCvar[WEPCONFIG_AMMOTYPE],  /** Cvar that contains the weapon's max reserve count.  See ammo_* cvars. */
    WepConfig_AmmoPrice,                            /** How much to charge for ammo. */
    bool:WepConfig_RestrictDefault,                 /** Whether to have the weapon restricted by default or not. */
    bool:WepConfig_Toggleable,                      /** Allow the restriction status to be changed mid-game. */
    Float:WepConfig_Knockback,                      /** Knockback multipler for the weapon. */
    WepConfig_Price,                                /** The price of the weapon. */
    WepConfig_BuyLimit,                             /** The limit on how many times the weapon can be bought by a client for the round. */
    
    // Per-weapon restriction.
    bool:WepConfig_Restricted,                      /** Used internally.  True if the weapon is restricted, false if not. */
    WepConfig_BuyCount,                             /** Used internally per-client.  The number of times a client has bought a weapon in their current life. */
}

/**
 * Dummy array used to see how many cells are required to store all weapon data.
 */
stock g_WeaponsDummyArray[WepConfigs];

/**
 * Use this to refer to the global weapon data in the array above.
 */
#define WEAPONS_GLOBAL 0

/**
 * Global and per-client weapon data array.
 * Index 0 holds the global data, the other indexes are for client-specific settings.
 */
new Handle:g_hWeapons[MAXPLAYERS + 1];

/**
 * Small function to check if the weapon configs have been cached.
 */
stock bool:Weapons_AreConfigsCached() { return bool:GetArraySize(g_hWeapons[WEAPONS_GLOBAL]); }

/**
 * Stores weapon type data.
 */
new Handle:g_hWeaponTypes;

/**
 * The number of weapons/weapon types configured in the weapons config file.
 */
new g_iWeaponCount;
new g_iWeaponTypeCount;

// Include sub-modules.
#include "zr/weapons/weprestrict"
#include "zr/weapons/zmarket"
#include "zr/weapons/wepalpha"
#include "zr/weapons/wepmenus"
#include "zr/weapons/gpbridge"

/**
 * Register this module.
 */
Weapons_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Weapons");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "weapons");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Monitors anything to do with weapons.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleWeapons = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleWeapons, "Event_OnPluginEnd",            "Weapons_OnPluginEnd");
    EventMgr_RegisterEvent(g_moduleWeapons, "Event_OnConfigsExecuted",      "Weapons_OnConfigsExecuted");
    EventMgr_RegisterEvent(g_moduleWeapons, "Event_OnClientPutInServer",    "Weapons_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleWeapons, "Event_OnClientDisconnect",     "Weapons_OnClientDisconnect");
    
    // Register config file(s) that this module will use.
    ConfigMgr_Register(g_moduleWeapons, "Weapons_OnConfigReload", "");
}

/**
 * Plugin is loading.
 */
Weapons_OnPluginStart()
{
    // Register the module.
    Weapons_Register();
    
    // Create cvars.
    g_hCvarWeaponsConfigFile = Project_CreateConVar("weapons_configfile",  "configs/zr/weapons.txt",   "Path to weapons config file.  Path is relative to the SourceMod directory.");
    
    // Create dynamic arrays to store weapon data.
    g_hWeapons[WEAPONS_GLOBAL] = CreateArray(sizeof(g_WeaponsDummyArray));
    g_hWeaponTypes = CreateArray(32);
    
    // Forward event to sub-modules.
    WepRestrict_OnPluginStart();
    ZMarket_OnPluginStart();
    WepAlpha_OnPluginStart();
    GPBridge_OnPluginStart();
}

/**
 * Plugin is ending.
 */
public Weapons_OnPluginEnd()
{
    // Cleanup data stored by the weapons module.
    for (new index = 0; index < MAXPLAYERS + 1; index++)
    {
        if (g_hWeapons[index] != INVALID_HANDLE)
            CloseHandle(g_hWeapons[index]);
        g_hWeapons[index] = INVALID_HANDLE;
    }
}

/**
 * The map has started.
 */
public Weapons_OnConfigsExecuted()
{
    Weapons_CacheWeaponData();
}

/**
 * Called when a registered config file (by this module) is manually reloaded.
 */
public Weapons_OnConfigReload(configindex)
{
    Weapons_CacheWeaponData();
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public Weapons_OnClientPutInServer(client)
{
    // Client is joining before OnConfigsExecuted, they need to rejoin.
    if (!Weapons_AreConfigsCached())
        KickClient(client);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public Weapons_OnClientDisconnect(client)
{
}


/**
 * Loops through each section of the keyvalues tree.
 * Note: ADT array in 'g_hWeapons[WEAPONS_GLOBAL]' should be cleared before using this.
 * 
 * @param kv            The keyvalues handle of the config file. (Don't close this)
 * @param sectionindex  The index of the current keyvalue section, starting from 0.
 * @param sectionname   The name of the current keyvalue section.
 * 
 * @return              See enum KvCache.
 */
public KvCache:Weapons_Cache(Handle:kv, sectionindex, const String:sectionname[])
{
    new weapondata[WepConfigs];
    
    // Section name.
    strcopy(weapondata[WepConfig_Name], sizeof(weapondata[WepConfig_Name]), sectionname);
    
    // General weapon information.
    KvGetString(kv, "entity", weapondata[WepConfig_Entity], sizeof(weapondata[WepConfig_Entity]));
    KvGetString(kv, "type", weapondata[WepConfig_Type], sizeof(weapondata[WepConfig_Type]));
    weapondata[WepConfig_Slot] = WepLib_Slots:KvGetNum(kv, "slot");
    
    // Restriction.
    weapondata[WepConfig_RestrictDefault] = Util_KvGetPhraseAsBool(kv, "restrictdefault", BoolPhrase_YesNo, "no");
    weapondata[WepConfig_Restricted] = weapondata[WepConfig_RestrictDefault];
    weapondata[WepConfig_Toggleable] = Util_KvGetPhraseAsBool(kv, "toggleable", BoolPhrase_YesNo, "yes");
    
    // Weapon Ammo.
    KvGetString(kv, "ammocvar", weapondata[WepConfig_AmmoCvar], sizeof(weapondata[WepConfig_AmmoCvar]));
    weapondata[WepConfig_AmmoPrice] = KvGetNum(kv, "ammoprice", -1);
    
    // Knockback.
    weapondata[WepConfig_Knockback] = KvGetFloat(kv, "knockback", 1.0);
    
    // ZMarket.
    weapondata[WepConfig_Price] = KvGetNum(kv, "price", -1);
    weapondata[WepConfig_BuyLimit] = KvGetNum(kv, "buylimit", 0);
    
    // TODO: VALIDATION.  If fail, return KvCache_Ignore.
    // Push the array into the data.
    PushArrayArray(g_hWeapons[WEAPONS_GLOBAL], weapondata[0], sizeof(weapondata));
    
    return KvCache_Continue;
}

/**
 * Re-cache all weapon data from disk.
 * Never use this before OnConfigsExecuted!
 */
Weapons_CacheWeaponData()
{
    decl String:configfile[PLATFORM_MAX_PATH];
    GetConVarString(g_hCvarWeaponsConfigFile, configfile, sizeof(configfile));
    
    if (ConfigMgr_ValidateFile(configfile))
        ConfigMgr_WriteString(g_moduleWeapons, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, configfile);
    else
    {
        LogMgr_Print(g_moduleWeapons, LogType_Fatal_Module, "Config Validation", "Invalid config file path in cvar zr_weapons_configfile: \"%s\"", configfile);
        return;
    }
    
    ClearArray(g_hWeapons[WEAPONS_GLOBAL]);
    g_iWeaponCount = ConfigMgr_CacheKv(g_moduleWeapons, CM_CONFIGINDEX_FIRST, "Weapons_Cache");
    
    // There were no weapons configured.
    if (g_iWeaponCount == 0)
        LogMgr_Print(g_moduleWeapons, LogType_Fatal_Module, "Config Validation", "No usable data found in weapons config file: %s", configfile);
    
    // Build a list of used weapon types.
    Weapons_BuildWTList();
}

/**
 * Builds a list of weapon types the server admin has used.
 */
Weapons_BuildWTList()
{
    // Start fresh.
    ClearArray(g_hWeaponTypes);
    g_iWeaponTypeCount = 0;
    
    decl String:weapontypes[512];
    new String:weapontype[16][32];
    
    // Loop through the weapon cache.
    for (new windex = 0; windex < g_iWeaponCount; windex++)
    {
        // Read the string of weapon types.
        Weapons_ReadString(windex, WepConfig_Type, weapontypes, sizeof(weapontypes));
        
        // Explode the string into an array of types this weapon falls under.
        ExplodeString(weapontypes, ",", weapontype, sizeof(weapontype), sizeof(weapontype[]));
        for (new wtindex = 0; wtindex < sizeof(weapontype); wtindex++)
        {
            TrimString(weapontype[wtindex]);
            
            // If we've reached the end of the weapon's types, then stop.
            if (weapontype[wtindex][0] == 0)
                break;
            
            // If the weapon type isn't in the main array, then push it in.
            if (FindStringInArray(g_hWeaponTypes, weapontype[wtindex]) == -1)
            {
                PushArrayString(g_hWeaponTypes, weapontype[wtindex]);
                g_iWeaponTypeCount++;
            }
        }
    }
}

/**
 * Copies the weapon data straight from the global cache to the specified client.
 * This must be done on any clients that will have differing weapon data from the global cache.
 * 
 * @param client    The client index.
 * @param overwrite True to overwrite existing per-client data if it exists.
 * 
 * @return          True if there were no problems, false if the data hasn't been cached yet.
 */
stock bool:Weapons_PerClientInit(client, bool:overwrite = false)
{
    if (!Weapons_AreConfigsCached())
        return false;
    
    if (g_hWeapons[client] != INVALID_HANDLE)
    {
        // If we're going to overwrite the data, destroy it first.
        if (overwrite)
            CloseHandle(g_hWeapons[client]);
        else
            return true;
    }
    
    g_hWeapons[client] = CloneArray(g_hWeapons[WEAPONS_GLOBAL]);
    return true;
}

/**
 * Destroys the per-client weapon data on a client.
 * Be careful with this, other modules might depend on this data! 
 * 
 * @param client    The client index.
 * 
 * @return          True if the data was destroyed, false if there is no data.
 */
stock Weapons_PerClientDestroy(client)
{
    if (g_hWeapons[client] == INVALID_HANDLE)
        return false;
    
    CloseHandle(g_hWeapons[client]);
    g_hWeapons[client] = INVALID_HANDLE;
    return true;
}

/**
 * Checks if per-client weapon data exists for a client.
 * 
 * @param client    The client index.
 * 
 * @return          True if it exists, false if not.
 */
stock bool:Weapons_PerClientExists(client)
{
    return (g_hWeapons[client] != INVALID_HANDLE);
}

/**
 * Find the weapon's config index given its name.
 * 
 * @param weapon    The weapon's name.
 *  
 * @return          The array index containing the given weapon name.  -1 if doesn't exist.
 */
stock Weapons_NameToIndex(const String:weapon[])
{
    decl String:weaponname[WEPCONFIG_NAME];
    
    for (new windex = 0; windex < g_iWeaponCount; windex++)
    {
        Weapons_ReadString(windex, WepConfig_Name, weaponname, sizeof(weaponname));
        if (StrEqual(weapon, weaponname, false))
            return windex;
    }
    
    return -1;
}

/**
 * Find the weapon type's config index given its name.
 * 
 * @param weapon    The weapon type's name.
 *  
 * @return          The array index containing the given weapon name.  -1 if doesn't exist.
 */
stock Weapons_TypeNameToIndex(const String:wtype[])
{
    decl String:wtypename[64];
    
    for (new index = 0; index < g_iWeaponTypeCount; index++)
    {
        Weapons_ReadWTypeName(index, wtypename, sizeof(wtypename));
        
        if (StrEqual(wtype, wtypename, false))
            return index;
    }
    
    return -1;
}

/**
 * Takes a weapon's entity name and returns the display name from the weapons config file.
 * 
 * @param entityname    The entity to find the display of.
 * @param display       Buffer to store display name in.
 * @param maxlen        The max length of the display name.
 * @param noprefix      True to compare the entity names without the weapon_/item_ prefix.
 * 
 * @return              The matching weapon index.
 */
stock Weapons_ClsNameToDisplay(const String:entityname[], String:display[], maxlen, noprefix = false)
{
    new String:weaponentity[64];
    
    for (new windex = 0; windex < g_iWeaponCount; windex++)
    {
        Weapons_ReadString(windex, WepConfig_Entity, weaponentity, sizeof(weaponentity));
        
        // If noprefix is true, then strip the weapon_/item_ prefix.
        if (noprefix)
        {
            ReplaceString(weaponentity, sizeof(weaponentity), "weapon_", "");
            ReplaceString(weaponentity, sizeof(weaponentity), "item_", "");
        }
        
        if (StrEqual(entityname, weaponentity, false))
        {
            Weapons_ReadString(windex, WepConfig_Name, display, maxlen);
            return windex;
        }
    }
    
    return -1;
}

/**
 * Reads a string-typed config value from the cache.
 * 
 * @param index     The weapon cache index.
 * @param config    The config to read value from.  See enum WepConfigs.
 * @param output    The output string.
 * @param maxlen    The max length of the output string.
 * @param client    The client, or WEAPONS_GLOBAL for all clients, to read data for.
 */
stock Weapons_ReadString(index, WepConfigs:config, String:output[], maxlen, client = WEAPONS_GLOBAL)
{
    // Safety net for callers that don't know if the client has its own data.
    if (g_hWeapons[client] == INVALID_HANDLE)
        client = WEAPONS_GLOBAL;
    
    new weapondata[WepConfigs];
    GetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
    
    strcopy(output, maxlen, String:weapondata[config]);
}

/**
 * Writes a string-typed config value to the cache.
 * 
 * @param index     The weapon cache index.
 * @param config    The config to write value to.  See enum WepConfigs.
 * @param value     The new string.
 * @param client    The client, or WEAPONS_GLOBAL for all clients, to write data for.
 */
stock Weapons_WriteString(index, WepConfigs:config, const String:value[], client = WEAPONS_GLOBAL)
{
    // Safety net for callers that don't know if the client has its own data.
    if (g_hWeapons[client] == INVALID_HANDLE)
        client = WEAPONS_GLOBAL;
    
    new weapondata[WepConfigs];
    GetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
    
    strcopy(String:weapondata[config], WEAPON_DATA_LONGEST_STRING, value);
    
    SetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
}

/**
 * Reads a cell-typed config value from the cache.
 * 
 * @param index     The weapon cache index.
 * @param config    The config to read value from.  See enum WepConfigs.
 * @param client    The client, or WEAPONS_GLOBAL for all clients, to read data for.
 * 
 * @return          The value of the config.  
 */
stock Weapons_ReadCell(index, WepConfigs:config, client = WEAPONS_GLOBAL)
{
    // Safety net for callers that don't know if the client has its own data.
    if (g_hWeapons[client] == INVALID_HANDLE)
        client = WEAPONS_GLOBAL;
    
    new weapondata[WepConfigs];
    GetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
    
    return weapondata[config];
}

/**
 * Writes a cell-typed config value to the cache.
 * 
 * @param index     The weapon cache index.
 * @param config    The config to write value to.  See enum WepConfigs.
 * @param value     The new cell.
 * @param client    The client, or WEAPONS_GLOBAL for all clients, to write data for.
 */
stock Weapons_WriteCell(index, WepConfigs:config, value, client = WEAPONS_GLOBAL)
{
    // Safety net for callers that don't know if the client has its own data.
    if (g_hWeapons[client] == INVALID_HANDLE)
        client = WEAPONS_GLOBAL;
    
    new weapondata[WepConfigs];
    GetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
    
    weapondata[config] = value;
    
    SetArrayArray(g_hWeapons[client], index, weapondata[0], sizeof(weapondata));
}

/**
 * Reads a string-typed config value from the cache.
 * 
 * @param index     The weapon cache index.
 * @param type      The output string.
 * @param maxlen    The max length of the output string.
 */
stock Weapons_ReadWTypeName(index, String:wtypename[], maxlen)
{
    GetArrayString(g_hWeaponTypes, index, wtypename, maxlen);
}

/**
 * Returns an array containing all weapon cache indexes matching the given weapon type.
 * 
 * @param index             The weapon type index.
 * @param hWeapons          A handle to store array containing matching weapons.
 *                          Don't forget to close this!
 * 
 * @return                  The number of weapons in the array.
 */
stock Weapons_GetWeaponsOfType(index, &Handle:hWeapons)
{
    hWeapons = CreateArray();
    
    // Get name of the weapon type at given index.
    decl String:typename[32];
    Weapons_ReadWTypeName(index, typename, sizeof(typename));
    
    new count = 0;
    decl String:weapontypes[64];
    new String:weapontype[16][sizeof(weapontypes)];
    
    for (new windex = 0; windex < g_iWeaponCount; windex++)
    {
        Weapons_ReadString(windex, WepConfig_Type, weapontypes, sizeof(weapontypes));
        
        ExplodeString(weapontypes, ",", weapontype, sizeof(weapontype), sizeof(weapontype[]));
        for (new wtindex = 0; wtindex < sizeof(weapontype); wtindex++)
        {
            TrimString(weapontype[wtindex]);
            if (!weapontype[wtindex][0])
                break;
            
            // If types match, then add weapon to array.
            if (StrEqual(typename, weapontype[wtindex], false))
            {
                PushArrayCell(hWeapons, windex);
                count++;
            }
        }
    }
    
    return count;
}
