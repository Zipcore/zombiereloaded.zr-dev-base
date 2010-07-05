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
new Handle:g_hCvarGameRulesConfigFile;
new Handle:g_hCvarGameRuleSet;

#define GAMERULES_NAME_LEN 64
#define GAMERULES_MAX_MODULES 64

/**
 * Available game rule actions.
 */
enum GameRuleAction
{
    GameRuleAction_Invalid = -1,
    GameRuleAction_On,
    GameRuleAction_ForceOn,
    GameRuleAction_Off,
    GameRuleAction_ForceOff
}

/**
 * Game rule data structure.
 */
enum GameRuleSet
{
    String:GameRuleSet_Name[GAMERULES_NAME_LEN],                /** Name of rule */
    Module:GameRuleSet_Module[GAMERULES_MAX_MODULES],           /** Modules that's listed in the rule set */
    GameRuleAction:GameRuleSet_Action[GAMERULES_MAX_MODULES],   /** Action to apply on the modules */
    GameRuleSet_NumModules                                      /** Number of modules in the rule set */
}

/**
 * Dummy array used to see how many cells are required to store all game rules.
 */
stock GameRulesDummyArray[GameRuleSet];

/**
 * Array handle for game rule sets.
 */
new Handle:g_hGameRules;

/**
 * Cache for the current rule set that's loaded.
 */
new g_RuleSetCache[GameRuleSet];

/**
 * Readability macros for reading data from the above array.
 */
#define GAMERULESET_NAME(%1)    g_RuleSetCache[GameRuleSet_Name][%1]
#define GAMERULESET_MODULE(%1)  g_RuleSetCache[GameRuleSet_Module][%1]
#define GAMERULESET_ACTION(%1)  g_RuleSetCache[GameRuleSet_Action][%1]
#define GAMERULESET_NUMMODULES  g_RuleSetCache[GameRuleSet_NumModules]

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
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Game Rules");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "gamerules");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Manages and controls events to modules.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleGameRules = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleEnable",           "GameRules_OnModuleEnable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleDisable",          "GameRules_OnModuleDisable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnMyModuleDisable",        "GameRules_OnMyModuleDisable");
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnConfigsExecuted",        "GameRules_OnConfigsExecuted");
    
    // Register config file(s) that this module will use.
    ConfigMgr_Register(g_moduleGameRules, "GameRules_OnConfigReload", "");
}

/**
 * Plugin is loading.
 */
GameRules_OnPluginStart()
{
    // Register the module.
    GameRules_Register();
    
    // Create cvars.
    g_hCvarGameRulesConfigFile  = Project_CreateConVar("gamerules_configfile",  "configs/zr/gamerules.txt",     "Path to game rules config file.  Path is relative to the SourceMod directory.");
    g_hCvarGameRuleSet          = Project_CreateConVar("gamerules_ruleset",     "default",                      "Name of game rule set to use.");
    
    // Create array.
    g_hGameRules = CreateArray(sizeof(GameRulesDummyArray));
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
    
    // Find module in current game rule set.
    new moduleindex = GameRules_FindModuleInRuleSet(g_iCurGameRuleSet, module);
    if (moduleindex == -1)
        return Plugin_Continue;
    
    // If this module is forced off then don't let it enable.
    if (GAMERULESET_ACTION(moduleindex) == GameRuleAction_ForceOff)
    {
        TransMgr_PrintText(SERVER_INDEX, MsgFormat_Both, MsgType_Reply, g_moduleGameRules, _, "No access to command");
        return Plugin_Handled;
    }
    
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
    
    // Find module in current game rule set.
    new moduleindex = GameRules_FindModuleInRuleSet(g_iCurGameRuleSet, module);
    if (moduleindex == -1)
        return Plugin_Continue;
    
    // If this module is forced on then don't let it disable.
    if (GAMERULESET_ACTION(moduleindex) == GameRuleAction_ForceOn)
    {
        TransMgr_PrintText(SERVER_INDEX, MsgFormat_Both, MsgType_Reply, g_moduleGameRules, _, "No access to command");
        return Plugin_Handled;
    }
    
    return Plugin_Continue;
}

/**
 * The module that hooked this event callback has been disabled.
 * 
 * @return      Return Plugin_Handled to stop disable, and Plugin_Continue to allow it.
 */
public Action:GameRules_OnMyModuleDisable()
{
    TransMgr_PrintText(SERVER_INDEX, MsgFormat_Both, MsgType_Reply, g_moduleGameRules, _, "No access to command");
    return Plugin_Handled;
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
    new numModules;
    new Module:module;
    new GameRuleAction:action;
    
    // Section name.
    strcopy(gameRuleSet[GameRuleSet_Name], GAMERULES_NAME_LEN, sectionname);
    
    decl String:keyName[64];
    decl String:keyValue[64];
    
    KvGotoFirstSubKey(kv, false);
    do
    {
        // Check if the module limit is reached.
        if (numModules >= GAMERULES_MAX_MODULES)
        {
            // Log a warning. Too many modules.
            LogMgr_Print(g_moduleGameRules, LogType_Error, "Config validation", "Too many modules in game rule: \"%s\".", sectionname);
            break;
        }
        
        KvGetSectionName(kv, keyName, sizeof(keyName));
        KvGoBack(kv);
        KvGetString(kv, keyName, keyValue, sizeof(keyValue));
        KvJumpToKey(kv, keyName);
        
        // Get the module ID.
        module =  ModuleMgr_FindByString(ModuleData_ShortName, keyName);
        
        // Check if module is valid.
        if (module != INVALID_MODULE)
        {
            gameRuleSet[GameRuleSet_Module][numModules] = module;
            
            // Validate and set rule action.
            action = StringToGameRuleAction(keyValue);
            if (action != GameRuleAction_Invalid)
            {
                gameRuleSet[GameRuleSet_Action][numModules] = action;
                numModules++;
            }
            else
            {
                // Log a warning. Invalid action.
                LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Invalid action in rule \"%s\": \"%s\".", sectionname, keyValue);
            }
        }
        else
        {
            // Log a warning. Invalid module.
            LogMgr_Print(g_moduleGameRules, LogType_Error, "Config Validation", "Invalid module in rule \"%s\": \"%s\".", sectionname, keyName);
        }
        
    } while (KvGotoNextKey(kv, false));
    
    gameRuleSet[GameRuleSet_NumModules] = numModules;
    PushArrayArray(g_hGameRules, gameRuleSet[0], sizeof(gameRuleSet));
    
    return KvCache_Continue;
}

/**
 * Re-cache all game rules data from disk.
 * Never use this before OnConfigsExecuted!
 */
GameRules_CacheGameRulesData()
{
    decl String:configfile[PLATFORM_MAX_PATH];
    GetConVarString(g_hCvarGameRulesConfigFile, configfile, sizeof(configfile));
    
    if (ConfigMgr_ValidateFile(configfile))
        ConfigMgr_WriteString(g_moduleGameRules, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, configfile);
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Module, "Config Validation", "Error: Invalid config file path in cvar zr_gamerules_configfile: \"%s\"", configfile);
        return;
    }
    
    ClearArray(g_hGameRules);
    g_iGameRuleCount = ConfigMgr_CacheKv(g_moduleGameRules, CM_CONFIGINDEX_FIRST, "GameRules_Cache");
    
    // There were no game rule sets configured.
    if (g_iGameRuleCount == 0)
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Module, "Config Validation", "Error: No usable data found in game rules config file: %s", configfile);
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
 * Converts a string value to a game rule action.
 *
 * @param action    String value to convert.
 * 
 * @return          Game rule action, or GameRuleAction_Invalid on error.
 */
GameRuleAction:StringToGameRuleAction(const String:action[])
{
    if (StrEqual(action, "on", false))
    {
        return GameRuleAction_On;
    }
    else if (StrEqual(action, "force_on", false))
    {
        return GameRuleAction_ForceOn;
    }
    else if (StrEqual(action, "off", false))
    {
        return GameRuleAction_Off;
    }
    else if (StrEqual(action, "force_off", false))
    {
        return GameRuleAction_ForceOff;
    }
    
    return GameRuleAction_Invalid;
}

/**
 * Loads and applies the rule set specified in the config.
 */
GameRules_LoadRuleSetFromCvar()
{
    decl String:ruleSetName[GAMERULES_NAME_LEN];
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
    new defaultruleindex = GameRules_FindRuleSet("default");
    if (rulesetindex > -1)
    {
        GameRules_ApplyRuleSet(rulesetindex);
    }
    else if (defaultruleindex > -1)
    {
        GameRules_ApplyRuleSet(defaultruleindex);
    }
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Plugin, "Config Validation", "Invalid rule set name \"%s\" and failed to fall back to default, unloading plugin.", ruleset);
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
    for (new moduleindex = 0; moduleindex < GAMERULESET_NUMMODULES; moduleindex++)
    {
        if (module == GAMERULESET_MODULE(moduleindex))
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
    
    // Loop through each module and apply its action.
    for (new moduleindex = 0; moduleindex < GAMERULESET_NUMMODULES; moduleindex++)
    {
        // Apply action.
        switch (GAMERULESET_ACTION(moduleindex))
        {
            case GameRuleAction_ForceOn:    ModuleMgr_Enable(GAMERULESET_MODULE(moduleindex));
            case GameRuleAction_Off:        ModuleMgr_Disable(GAMERULESET_MODULE(moduleindex));
            case GameRuleAction_ForceOff:   ModuleMgr_Disable(GAMERULESET_MODULE(moduleindex));
        }
    }
}
