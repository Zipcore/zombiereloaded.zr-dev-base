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

#define GAMERULES_NAME_LEN 64
#define GAMERULES_MAX_MODULES 64

/**
 * Available game rule actions.
 */
enum GameRulesAction
{
    GameRuleAction_Invalid = -1,
    GameRuleAction_Enable,
    GameRuleAction_Disable
}

/**
 * Game rule data structure.
 */
enum GameRule
{
    String:GameRule_Name[GAMERULES_NAME_LEN],               /** Name of rule */
    Module:GameRule_Modules[GAMERULES_MAX_MODULES],         /** Modules that's listed in the rule set */
    GameRulesAction:GameRule_Actions[GAMERULES_MAX_MODULES] /** Modules' action */
}

/**
 * Dummy array used to see how many cells are required to store all game rules.
 */
stock GameRulesDummyArray[GameRule];

/**
 * Array handle for game rule sets.
 */
new Handle:g_hGameRules;


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
    EventMgr_RegisterEvent(g_moduleGameRules, "Event_OnConfigsExecuted",        "GameRules_OnConfigsExecuted");
    
    // Register config file(s) that this module will use.
    ConfigMgr_Register(g_moduleGameRules, "GameRules_OnConfigReload", "configs/zr/gamerules.txt");
}

/**
 * Plugin is loading.
 */
GameRules_OnPluginStart()
{
    // Register the module.
    GameRules_Register();
    
    // Create array.
    g_hGameRules = CreateArray(sizeof(GameRulesDummyArray));
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
    new gameRule[GameRule];
    new numModules = 0;
    new Module:module = INVALID_MODULE;
    new GameRulesAction:action = GameRuleAction_Invalid:
    
    // Section name.
    strcopy(gameRule[GameRule_Name], GAMERULES_NAME_LEN, sectionname);
    
    decl String:keyName[64];
    decl String:keyValue[64];
    keyName[0] = 0;
    keyValue[0] = 0;
    
    KvGotoFirstSubKey(kv);
    do
    {
        // Check if the module limit is reached.
        if (numModules >= GAMERULES_MAX_MODULES)
        {
            // Log a warning. Too many modules.
            #if defined LOG_MANAGER
                LogMgr_Print(module, LogType_Error, "Config validation", "Too many modules in game rule: \"%s\".", sectionname);
            #else
                PrintToServer("Too many modules in game rule: \"%s\".", sectionname);
            #endif
            break;
        }
        
        KvGetSectionName(kv, keyName, sizeof(keyName));
        KvGoBack(kv);
        KvGetString(kv, keyName, keyValue, sizeof(keyValue));
        KvJumpToKey(kv, keyName);
        
        // Get the module ID.
        module =  ModuleMgr_FindByString(ModuleData_ShortName, keyName);
        
        // Check if valid.
        if (module != INVALID_MODULE)
        {
            gameRule[GameRule_Modules][numModules] = module;
            
            action = StringToGameRuleAction(keyValue);
            if (action != GameRuleAction_Invalid)
            {
                gameRule[GameRule_Actions][numModules] = action;
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
    
    PushArrayArray(g_hGameRules, gameRule[0], sizeof(gameRule));
    
    return KvCache_Continue;
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public GameRules_OnConfigsExecuted()
{

}

/**
 * Called when a registered config file (by this module) is manually.
 */
public GameRules_OnConfigReload(configindex)
{

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
        return GameRuleAction_Enable;
    }
    else if (StrEqual(action, "off", false))
    {
        return GameRuleAction_Disable;
    }
    
    return GameRuleAction_Invalid;
}
