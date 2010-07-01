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
    GameRuleAction_Off,
    GameRuleAction_ForceOn
}

/**
 * Game rule data structure.
 */
enum GameRuleSet
{
    String:GameRuleSet_Name[GAMERULES_NAME_LEN],                /** Name of rule */
    Module:GameRuleSet_Modules[GAMERULES_MAX_MODULES],          /** Modules that's listed in the rule set */
    GameRuleAction:GameRuleSet_Actions[GAMERULES_MAX_MODULES],  /** Action to apply on the modules */
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
 * The number of game rule sets that are configured in the game rules config file.
 */
new g_iGameRuleCount;

/**
 * Register this module.
 */
GameRules_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = true;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Game Rule Controller");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "gamerules");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Manages and controls events to modules.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleGameRules = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    //EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnMyModuleEnable",         "GameRules_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnMyModuleDisable",        "GameRules_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleEnable",           "GameRules_OnModuleEnable");
    //EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnModuleDisable",          "GameRules_OnModuleDisable");
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

/*public GameRules_OnMyModuleDisable(Module:module)
{
    // TODO: What to do when disabling? Enable modules that were disabled?
}*/

/**
 * A module has been enabled.
 */
/*public GameRules_OnModuleEnable(Module:module)
{
    // TODO: Prevent module from enabling if the rule says it should be disabled?
    //       Allow modules to be enabled manually?
}*/

/**
 * A module has been disabled.
 */
/*public GameRules_OnModuleDisable(Module:module)
{
}*/

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
    
    KvGotoFirstSubKey(kv);
    do
    {
        // Check if the module limit is reached.
        if (numModules >= GAMERULES_MAX_MODULES)
        {
            // Log a warning. Too many modules.
            LogMgr_Print(module, LogType_Error, "Config validation", "Too many modules in game rule: \"%s\".", sectionname);
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
            gameRuleSet[GameRuleSet_Modules][numModules] = module;
            
            // Validate and set rule action.
            action = StringToGameRuleAction(keyValue);
            if (action != GameRuleAction_Invalid)
            {
                gameRuleSet[GameRuleSet_Actions][numModules] = action;
                numModules++;
            }
            else
            {
                // Log a warning. Invalid action.
                LogMgr_Print(module, LogType_Error, "Config validation", "Invalid action in rule \"%s\": \"%s\".", sectionname, keyValue);
            }
        }
        else
        {
            // Log a warning. Invalid module.
            LogMgr_Print(module, LogType_Error, "Config validation", "Invalid module in rule \"%s\": \"%s\".", sectionname, keyName);
        }
        
    } while (KvGotoNextKey(kv));
    
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
    GameRules_LoadRuleSet();
}

/**
 * Called when a registered config file (by this module) is manually.
 */
public GameRules_OnConfigReload(configindex)
{
    GameRules_CacheGameRulesData();
    GameRules_LoadRuleSet();
}

/**
 * Converts a string value to a game rule action.
 *
 * @param action    String value to convert.
 * @return          Game rule action, or GameRuleAction_Invalid on error.
 */
GameRuleAction:StringToGameRuleAction(const String:action[])
{
    // TODO: Use a generic function for converting a string value to boolean.
    
    if (StrEqual(action, "on", false))
    {
        return GameRuleAction_On;
    }
    else if (StrEqual(action, "off", false))
    {
        return GameRuleAction_Off;
    }
    else if (StrEqual(action, "force_on", false))
    {
        return GameRuleAction_ForceOn;
    }
    
    return GameRuleAction_Invalid;
}

/**
 * Loads and applies the rule set specified in the config.
 */
GameRules_LoadRuleSet()
{
    decl String:ruleSetName[GAMERULES_NAME_LEN];
    GetConVarString(g_hCvarGameRuleSet, ruleSetName[0], sizeof(ruleSetName));
    
    new ruleSet = GameRules_FindRuleSet(ruleSetName);
    if (ruleSet >= 0)
    {
        GameRules_ApplyRuleSet(ruleSet);
    }
    else
    {
        LogMgr_Print(g_moduleGameRules, LogType_Fatal_Module, "Config Validation", "Invalid rule set name in cvar zr_gamerules_ruleset: \"%s\"", ruleSetName);
    }
}

/**
 * Gets the rule set with the specified name.
 *
 * @param name  Name of rule set.
 * @return      Rule set index if found, -1 otherwise.
 */
GameRules_FindRuleSet(const String:name[])
{
    new gameRuleSet[GameRuleSet];
    for (new ruleSet = 0; ruleSet < g_iGameRuleCount; ruleSet++)
    {
        GetArrayArray(g_hGameRules, ruleSet, gameRuleSet[0], sizeof(gameRuleSet));  
        if (StrEqual(gameRuleSet[GameRuleSet_Name], name, false))
        {
            return ruleSet;
        }
    }
    return -1;
}

/**
 * Applies a rule set and sets the module state (enable or disable them)
 * according to the rule set.
 *
 * @param ruleSet   Index of rule set to apply.
 */
GameRules_ApplyRuleSet(ruleSet)
{
    GetArrayArray(g_hGameRules, ruleSet, g_RuleSetCache[0], sizeof(g_RuleSetCache));
    
    // Loop through each module and apply its action.
    new numModules = g_RuleSetCache[GameRuleSet_NumModules];
    for (new i = 0; i < numModules; i++)
    {
        new Module:module = g_RuleSetCache[GameRuleSet_Modules][i];
        new GameRuleAction:action = g_RuleSetCache[GameRuleSet_Actions][i];
        
        // Apply action.
        switch (action)
        {
            case GameRuleAction_On, GameRuleAction_ForceOn:
            {
                ModuleMgr_Enable(module);
            }
            case GameRuleAction_Off:
            {
                ModuleMgr_Disable(module);
            }
        }
    }
}
