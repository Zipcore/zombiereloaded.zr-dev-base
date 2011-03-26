/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          gamerules.inc
 *  Type:          Module
 *  Description:   Game rule controller.
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
 * cache/apply the gamerules file FIRST in OnMapStart before other modules.
 * implications:
 * all modules that are used for this game mode will be active by OnMapStart.(anything before might not get fired)
 * path must be non-cvar or hardcoded.
 */

/**
 * This module's identifier.
 */
new Module:g_moduleGameRules;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:GameRules_GetIdentifier() { return g_moduleGameRules; }

/**
 * Cvar handles.
 */
new Handle:g_hCvarGameRuleSet;

/**
 * Location of gamerules config file.
 */
#define GAMERULES_FILE "configs/zr/gamerules.txt"

/**
 * Available game rule actions.
 */
enum GRuleModuleCmds
{
    GRuleModuleCmd_Invalid = -1,
    GRuleModuleCmd_On,
    GRuleModuleCmd_ForceOn,
    GRuleModuleCmd_Off,
    GRuleModuleCmd_ForceOff
}

/**
 * Array/string sizes for the enum members below.
 */
#define GAMERULES_DATA_NAME 64

/**
 * Array handle for game rule sets.
 */
new Handle:g_hGameRules;

/**
 * Fallback game rule set.
 */
#define GRULESET_FALLBACK "zrclassic"

/**
 * Game rule data structure.
 */
enum GameRuleSet
{
    String:GameRuleSet_Name[GAMERULES_DATA_NAME],       /** Name of rule set. */
    Module:GameRuleSet_Core,                            /** The root module for the GRS. */
    Handle:GameRuleSet_Configs,                         /** File paths that point to config files for the game rule set. */
    Handle:GameRuleSet_ModuleCmds,                      /** Module cmds for the GRS. */
}

/**
 * Cache for the current rule set that's loaded.
 */
new g_RuleSetCache[GameRuleSet];

/**
 * Readability macros for reading data from the game rule set cache.
 */
#define GRULESET_NAME               g_RuleSetCache[GameRuleSet_Name]
#define GRULESET_CORE               g_RuleSetCache[GameRuleSet_Core]
#define GRULESET_CONFIGTRIE         g_RuleSetCache[GameRuleSet_Configs]
#define GRULESET_MODULECMDTRIE      g_RuleSetCache[GameRuleSet_ModuleCmds]

/**
 * The number of game rule sets that are configured in the game rules config file.
 */
new g_iGameRuleCount;

/**
 * The currently applied game rule set.
 */
new g_iCurGameRuleSet = -1;

/**
 * Register this module.
 */
GameRules_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Game Rules");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "gamerules");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Manipulates modules and configs to create unique game modes via rule sets.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleGameRules = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnEventsRegister",         "GameRules_OnEventsRegister");
    
    // Register config file(s) that this module will use.
    ConfigMgr_Register(g_moduleGameRules, "GameRules_OnConfigReload", "");
}

/**
 * Register all events here.
 */
public GameRules_OnEventsRegister()
{
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnEventsReady",            "GameRules_OnEventsReady");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleEnable",           "GameRules_OnModuleEnable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleDisable",          "GameRules_OnModuleDisable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnMyModuleDisable",        "GameRules_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnConfigsExecuted",        "GameRules_OnConfigsExecuted");
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public GameRules_OnEventsReady()
{
    // Bump up priority for Event_OnConfigsExecuted to above mapconfig the module to ensure game rule set cfgs are executed before map configs.
    EventMgr_GivePriority("Event_OnConfigsExecuted", g_moduleGameRules, MapConfig_GetIdentifier());
}

/**
 * Plugin is loading.
 */
GameRules_OnPluginStart()
{
    // Register the module.
    GameRules_Register();
    
    // Create cvars.
    g_hCvarGameRuleSet          = Project_CreateConVar("ruleset", "zrclassic", "Name of game rule set to use.");
    
    // Create array.
    g_hGameRules = CreateArray(sizeof(g_RuleSetCache));
}

/**
 * A module has been enabled.
 * 
 * @return      Return Plugin_Handled to stop enable, and Plugin_Continue to allow it.
 */
public Action:GameRules_OnModuleEnable(Module:module)
{
    // Ignore this module.
    if (module == g_moduleGameRules)
        return Plugin_Continue;
    
    // If no game rule set is applied yet, then let it be enabled.
    if (g_iCurGameRuleSet == -1)
        return Plugin_Continue;
    
    // Don't allow a root module that isn't defined by the current game mode be enabled.
    if (ModuleMgr_ReadCell(module, ModuleData_Root) && module != GRULESET_CORE)
        return Plugin_Handled;
    
    // If this module is forced off then don't let it enable.
    decl String:moduleshortname[MM_DATA_SHORTNAME];
    ModuleMgr_ReadString(module, ModuleData_ShortName, moduleshortname, sizeof(moduleshortname));
    if (GameRules_GetModuleCmd(moduleshortname) == GRuleModuleCmd_ForceOff)
        return Plugin_Handled;
    
    return Plugin_Continue;
}

/**
 * A module has been disabled.
 * 
 * @return      Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public Action:GameRules_OnModuleDisable(Module:module)
{
    // Ignore this module, it's already handled in GameRules_OnMyModuleDisable
    if (module == g_moduleGameRules)
        return Plugin_Continue;
    
    // If no game rule set is applied yet, then let it be disabled.
    if (g_iCurGameRuleSet == -1)
        return Plugin_Continue;
    
    // Don't allow a root module defined by the current game mode be disabled.
    if (module == GRULESET_CORE)
        return Plugin_Handled;
    
    // If this module is forced on then don't let it disable.
    decl String:moduleshortname[MM_DATA_SHORTNAME];
    ModuleMgr_ReadString(module, ModuleData_ShortName, moduleshortname, sizeof(moduleshortname));
    if (GameRules_GetModuleCmd(moduleshortname) == GRuleModuleCmd_ForceOn)
        return Plugin_Handled;
    
    return Plugin_Continue;
}

/**
 * The module that hooked this event callback has been disabled.
 * 
 * @return      Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public Action:GameRules_OnMyModuleDisable()
{
    return Plugin_Handled;
}

stock CleanupGameRulesData()
{
    // Cleanup tries in gamerules data.
    new gameRuleSet[GameRuleSet];
    for (new ruleSet = 0; ruleSet < g_iGameRuleCount; ruleSet++)
    {
        GetArrayArray(g_hGameRules, ruleSet, gameRuleSet[0], sizeof(gameRuleSet));
        
        if (gameRuleSet[GameRuleSet_Configs] != INVALID_HANDLE)
            CloseHandle(gameRuleSet[GameRuleSet_Configs]);
        
        if (gameRuleSet[GameRuleSet_ModuleCmds] != INVALID_HANDLE)
            CloseHandle(gameRuleSet[GameRuleSet_ModuleCmds]);
    }
}
    

/**
 * Loops through each section of the keyvalues tree.
 * 
 * @param kv            The keyvalues handle of the config file. (Don't close this)
 * @param sectionindex  The index of the current keyvalue section, starting from 0.
 * @param sectionname   The name of the current keyvalue section.
 * 
 * @return              See enum KvCache.
 */
public KvCache:GameRules_Cache(Handle:kv, sectionindex, const String:sectionname[])
{
    new gameRuleSet[GameRuleSet];
    
    // Section name = game rule set name.
    strcopy(gameRuleSet[GameRuleSet_Name], GAMERULES_DATA_NAME, sectionname);
    
    // Store data from known key names.
    
    // Get value and validate for "core"
    decl String:rootmodulename[MM_DATA_SHORTNAME];
    KvGetString(kv, "core", rootmodulename, sizeof(rootmodulename));
    new Module:rootmodule = ModuleMgr_FindByString(ModuleData_ShortName, rootmodulename);
    if (rootmodule != INVALID_MODULE)
    {
        gameRuleSet[GameRuleSet_Core] = rootmodule;
    }
    else
    {
        gameRuleSet[GameRuleSet_Core] = INVALID_MODULE;
        LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "No core specified for game rule set \"%s\"!", sectionname);
    }
    
    // Go down a level and store config path data.
    gameRuleSet[GameRuleSet_Configs] = CreateTrie();
    if (KvJumpToKey(kv, "configs"))
    {
        decl String:name[MM_DATA_SHORTNAME];
        decl String:strFilePath[PLATFORM_MAX_PATH];
        KvGotoFirstSubKey(kv, false);
        do
        {
            KvGetSectionName(kv, name, sizeof(name));
            KvGoBack(kv);
            KvGetString(kv, name, strFilePath, sizeof(strFilePath));
            KvJumpToKey(kv, name);
            
            if (!SetTrieString(gameRuleSet[GameRuleSet_Configs], name, strFilePath, false))
                LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Duplicate entry in ruleset \"%s\" section \"configs\": \"%s\"", sectionname, name);
        } while (KvGotoNextKey(kv, false));
        KvGoBack(kv);   // Go back to 2nd level.
        
        // Go back to 1st level.
        KvGoBack(kv);
    }
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Normal, "Config Validation", "Warning: Game rule set \"%s\" is missing the \"config\" section.  See gamerules config file.", sectionname);
    }
    
    // Go down a level and store module command data.
    gameRuleSet[GameRuleSet_ModuleCmds] = CreateTrie();
    if (KvJumpToKey(kv, "modulecmds"))
    {
        decl String:moduleshortname[MM_DATA_SHORTNAME];
        decl String:strModuleCmd[64];
        new Module:module;
        new GRuleModuleCmds:modulecmd;
        KvGotoFirstSubKey(kv, false);
        do
        {
            KvGetSectionName(kv, moduleshortname, sizeof(moduleshortname));
            KvGoBack(kv);
            KvGetString(kv, moduleshortname, strModuleCmd, sizeof(strModuleCmd));
            KvJumpToKey(kv, moduleshortname);
            
            // Get the module identifier.
            module =  ModuleMgr_FindByString(ModuleData_ShortName, moduleshortname);
            
            // Check if module is valid.
            if (module != INVALID_MODULE)
            {
                // Validate and set rule action.
                modulecmd = GameRules_ModuleCmdToSymbol(strModuleCmd);
                if (modulecmd != GRuleModuleCmd_Invalid)
                {
                    if (!SetTrieValue(gameRuleSet[GameRuleSet_ModuleCmds], moduleshortname, modulecmd, false))
                        LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Duplicate entry in ruleset \"%s\" section \"modulecmds\": \"%s\"", sectionname, moduleshortname);
                }
                else
                {
                    LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Invalid module command in rule set \"%s\": \"%s\".", sectionname, strModuleCmd);
                }
            }
            else
            {
                LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Invalid module in rule \"%s\": \"%s\".", sectionname, moduleshortname);
            }
            
        } while (KvGotoNextKey(kv, false));
        KvGoBack(kv);   // Go back to 2nd level.
        
        // Go back to 1st level.
        KvGoBack(kv);
    }
    
    // Push all this collected data to an array.
    PushArrayArray(g_hGameRules, gameRuleSet[0], sizeof(gameRuleSet));
    
    return KvCache_Continue;
}

/**
 * Re-cache all game rules data from disk.
 * Never use this before OnConfigsExecuted!
 */
GameRules_CacheGameRulesData()
{
    if (ConfigMgr_ValidateFile(GAMERULES_FILE))
    {
        ConfigMgr_WriteString(g_moduleGameRules, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, GAMERULES_FILE);
    }
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Module, "Config Validation", "Error: Missing critical gamerules config file: \"%s\"", GAMERULES_FILE);
        return;
    }
    
    CleanupGameRulesData();
    ClearArray(g_hGameRules);
    g_iGameRuleCount = ConfigMgr_CacheKv(g_moduleGameRules, CM_CONFIGINDEX_FIRST, "GameRules_Cache");
    
    // There were no game rule sets configured.
    if (g_iGameRuleCount == 0)
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Module, "Config Validation", "Error: No usable data found in game rules config file: \"%s\"", GAMERULES_FILE);
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public GameRules_OnConfigsExecuted()
{
    GameRules_CacheGameRulesData();
    GameRules_LoadRuleSetFromCvar();
}

/**
 * Called when a registered config file (by this module) is manually.
 */
public GameRules_OnConfigReload(configindex)
{
    GameRules_CacheGameRulesData();
    GameRules_LoadRuleSetFromCvar();
}

/**
 * Returns the path associated with the given name in the game rule set under the "configs" section.
 * 
 * @param name  The name of the config.
 * @param path  The output buffer.
 * 
 * @return      True if got successfully, false if it doesn't exist.
 */
stock bool:GameRules_GetNamedConfig(const String:name[], String:path[PLATFORM_MAX_PATH])
{
    return GetTrieString(GRULESET_CONFIGTRIE, name, path, sizeof(path));
}

/**
 * Returns the module cmd associated with the given module name in the game rule set under the "modulecmd" section.
 * 
 * @param name  The name of the module.
 * 
 * @return      The module cmd.
 */
stock GRuleModuleCmds:GameRules_GetModuleCmd(const String:moduleshortname[])
{
    new GRuleModuleCmds:value = GRuleModuleCmd_Invalid;
    GetTrieValue(GRULESET_MODULECMDTRIE, moduleshortname, _:value);
    return value;
}

/**
 * Converts a string value to a module cmd symbol.
 * 
 * @param modulecmd     String value to convert to the appropriate GRuleModuleCmds symbol.
 * 
 * @return              A member of enum GRuleModuleCmds, GRuleModuleCmd_Invalid if no symbol matches.
 */
stock GRuleModuleCmds:GameRules_ModuleCmdToSymbol(const String:modulecmd[])
{
    if (StrEqual(modulecmd, "on", false))
    {
        return GRuleModuleCmd_On;
    }
    else if (StrEqual(modulecmd, "force_on", false))
    {
        return GRuleModuleCmd_ForceOn;
    }
    else if (StrEqual(modulecmd, "off", false))
    {
        return GRuleModuleCmd_Off;
    }
    else if (StrEqual(modulecmd, "force_off", false))
    {
        return GRuleModuleCmd_ForceOff;
    }
    
    return GRuleModuleCmd_Invalid;
}

/**
 * Loads and applies the rule set specified in the config.
 */
stock GameRules_LoadRuleSetFromCvar()
{
    decl String:ruleSetName[GAMERULES_DATA_NAME];
    GetConVarString(g_hCvarGameRuleSet, ruleSetName[0], sizeof(ruleSetName));
    
    GameRules_LoadRuleSet(ruleSetName);
}

/**
 * Loads a named rule set.
 * 
 * @param ruleset   The ruleset as listed in gamerules config file.
 */
stock GameRules_LoadRuleSet(const String:ruleset[])
{
    // Find the index of the named ruleset.  -1 is returned if invalid.
    new rulesetindex = GameRules_FindRuleSet(ruleset);
    new fallbackrulesetindex = GameRules_FindRuleSet(GRULESET_FALLBACK);
    if (rulesetindex > -1)
    {
        GameRules_ApplyRuleSet(rulesetindex);
    }
    else if (fallbackrulesetindex > -1)
    {
        LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Invalid rule set name \"%s\", falling back to rule set \"%s\".", ruleset, GRULESET_FALLBACK);
        GameRules_ApplyRuleSet(fallbackrulesetindex);
    }
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Plugin, "Config Validation", "Invalid rule set name \"%s\" and failed to fall back to \"%s\", unloading plugin.", ruleset, GRULESET_FALLBACK);
    }
}

/**
 * Gets the rule set with the specified name.
 *
 * @param name  Name of rule set.
 *  
 * @return      Rule set index if found, -1 otherwise.
 */
stock GameRules_FindRuleSet(const String:name[])
{
    new gameRuleSet[GameRuleSet];
    for (new ruleSet = 0; ruleSet < g_iGameRuleCount; ruleSet++)
    {
        GetArrayArray(g_hGameRules, ruleSet, gameRuleSet[0], sizeof(gameRuleSet));  
        if (StrEqual(gameRuleSet[GameRuleSet_Name], name, false))
            return ruleSet;
    }
    
    return -1;
}

/**
 * Gets the rule set with the specified name.
 *
 * @param ruleset   The game rule set index to look for module in.
 * @param module    The module to look for.
 * 
 * @return          Index that module lies in.
 */
stock GameRules_FindModuleInRuleSet(ruleset, Module:module)
{
    for (new moduleindex = 0; moduleindex < GRULESET_NUMMODULES; moduleindex++)
    {
        if (module == GRULESET_MODULE(moduleindex))
            return moduleindex;
    }
    
    return -1;
}

/**
 * Applies a rule set and sets the module state (enable or disable them)
 * according to the rule set.
 *
 * @param ruleSet   Index of rule set to apply.
 */
stock GameRules_ApplyRuleSet(ruleSet)
{
    GetArrayArray(g_hGameRules, ruleSet, g_RuleSetCache[0], sizeof(g_RuleSetCache));
    g_iCurGameRuleSet = ruleSet;    // Hiding behind this line above in case ruleSet is invalid.
    
    // Enable the core's root module.
    if (GRULESET_CORE != INVALID_MODULE)
        ModuleMgr_Enable(GRULESET_CORE);
    
    // Disable all other cores.
    new count = MODULE_COUNT;
    for (new moduleindex = 0; moduleindex < count; moduleindex++)
    {
        if (ModuleMgr_ReadCell(Module:moduleindex, ModuleData_Root) && Module:moduleindex != GRULESET_CORE)
            ModuleMgr_Disable(Module:moduleindex);
    }
    
    // Execute this rule set's cfg file.
    decl String:cfgfile[PLATFORM_MAX_PATH];
    GameRules_GetNamedConfig("cfg", cfgfile);
    if (cfgfile[0])
    {
        ServerCommand("exec sourcemod/zombiereloaded/%s", cfgfile);
        LogMgr_Print(g_moduleGameRules, LogType_Debug, "Game Rule Configs", "Executed game rule set file: sourcemod/zombiereloaded/%s", cfgfile);
    }
    
    // Loop through each module and apply its action.
    // Loop through all the modules.
    decl String:moduleshortname[MM_DATA_SHORTNAME];
    for (new moduleindex = 0; moduleindex < count; moduleindex++)
    {
        // Get module's short name to look up.
        ModuleMgr_ReadString(Module:moduleindex, ModuleData_ShortName, moduleshortname, sizeof(moduleshortname));
        
        // Apply action.
        switch (GameRules_GetModuleCmd(moduleshortname))
        {
            // TODO: why not On?
            case GRuleModuleCmd_On:         ModuleMgr_Enable(Module:moduleindex);
            case GRuleModuleCmd_ForceOn:    ModuleMgr_Enable(Module:moduleindex);
            case GRuleModuleCmd_Off:        ModuleMgr_Disable(Module:moduleindex);
            case GRuleModuleCmd_ForceOff:   ModuleMgr_Disable(Module:moduleindex);
        }
    }
}
