/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          modelmgr.inc
 *  Type:          Module
 *  Description:   Manages models and model collections.
 *
 *  Copyright (C) 2009-2011  Greyscale, Richard Helgeby
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
new Module:g_moduleModelMgr;

/**
 * Function for outside files to use to return the module's identifier.
 */
stock Module:ModelMgr_GetIdentifier() { return g_moduleModelMgr; }

/**
 * Default file paths.
 */
#define MODEL_FILE              "configs/zr/models.txt"
#define MODEL_COLLECTION_FILE   "configs/zr/modelcollections.txt"

/**
 * Cvar handles.
 */
new Handle:g_hCvarModelsFile;
new Handle:g_hCvarModelCollectionsFile;

// Module components.
#include "src/zr/modules/models/modeldb"

/**
 * Register this module.
 */
ModelMgr_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Model manager");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "modelmgr");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Manages models and model collections.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleModelMgr = ModuleMgr_Register(moduledata);
    
    EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnEventsRegister",      "ModelMgr_OnEventsRegister");
    
    // Register config file that this module will use.
    ConfigMgr_Register(g_moduleModelMgr, "ModelsMgr_OnConfigReload", MODEL_FILE);
}

/**
 * Plugin is loading.
 */
ModelMgr_OnPluginStart()
{
    // Register the module.
    ModelMgr_Register();
    
    // Create cvars.
    g_hCvarModelsFile = Project_CreateConVar("models_file", MODEL_FILE, "Path to model config file.  Path is relative to the SourceMod directory.");
    g_hCvarModelCollectionsFile = Project_CreateConVar("model_collections_file", MODEL_COLLECTION_FILE, "Path to model collections config file.  Path is relative to the SourceMod directory.");
}

/**
 * All plugins have loaded.
 */
public ModelMgr_OnAllPluginsLoaded()
{
}

/**
 * Plugin is ending.
 */
public ModelMgr_OnPluginEnd()
{
}

/**
 * Register all events here.
 */
public ModelMgr_OnEventsRegister()
{
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnAllPluginsLoaded",      "ModelMgr_OnAllPluginsLoaded");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnPluginEnd",             "ModelMgr_OnPluginEnd");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnAllModulesLoaded",      "ModelMgr_OnAllModulesLoaded");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnModuleEnable",          "ModelMgr_OnModuleEnable");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnMyModuleEnable",        "ModelMgr_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnModuleDisable",         "ModelMgr_OnModuleDisable");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnMyModuleDisable",       "ModelMgr_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnMapStart",              "ModelMgr_OnMapStart");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnMapEnd",                "ModelMgr_OnMapEnd");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnAutoConfigsBuffered",   "ModelMgr_OnAutoConfigsBuffered");
    EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnConfigsExecuted",       "ModelMgr_OnConfigsExecuted");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnClientPutInServer",     "ModelMgr_OnClientPutInServer");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnClientDisconnect",      "ModelMgr_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_RoundStart",              "ModelMgr_RoundStart");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_RoundFreezeEnd",          "ModelMgr_RoundFreezeEnd");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_RoundEnd",                "ModelMgr_RoundEnd");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_PlayerTeam",              "ModelMgr_PlayerTeam");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_PlayerSpawn",             "ModelMgr_PlayerSpawn");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_PlayerHurt",              "ModelMgr_PlayerHurt");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_PlayerDeath",             "ModelMgr_PlayerDeath");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_PlayerJump",              "ModelMgr_PlayerJump");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_WeaponFire",              "ModelMgr_WeaponFire");
    
    #endif
}

/**
 * All modules and events have been registered by this point.  Event priority can be changed here.
 */
public ModelMgr_OnEventsReady()
{
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public ModelMgr_OnConfigsExecuted()
{
    ModelMgr_LoadModels();
}

/**
 * Called when a registered config file (by this module) is manually reloaded.
 */
public ModelMgr_OnConfigReload(configindex)
{
    ModelMgr_LoadModels();
}

/**
 * Loads models from file.
 *
 * Note: Models must NOT be loaded BEFORE OnConfigsExecuted event!
 * Note: Failing to load models will stop the plugin.
 */
ModelMgr_LoadModels()
{
    // TODO: Do nothing if module is disabled.
    
    decl String:configfile[PLATFORM_MAX_PATH];
    GetConVarString(g_hCvarModelsFile, configfile, sizeof(configfile));
    ModelCount = 0;
    
    // Validate path.
    if (ConfigMgr_ValidateFile(configfile))
    {
        ConfigMgr_WriteString(g_moduleModelMgr, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, configfile);
    }
    else
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Fatal_Plugin, "Config Validation", "Error: Invalid config file path in cvar zr_models_file: \"%s\"", configfile);
        return;
    }
    
    // Log loading-message.
    LogMgr_Print(g_moduleModelMgr, LogType_Normal, "Config Loading", "Loading models from file \"%s\".", configfile);
    
    // Parse model file.
    ConfigMgr_CacheKv(g_moduleModelMgr, CM_CONFIGINDEX_FIRST, "ModelDB_LoadModel");
    
    // Log loaded-message.
    LogMgr_Print(g_moduleModelMgr, LogType_Normal, "Config Loading", "Models loaded: %d", ModelCount);
    
    // Check if there are no classes.
    if (ModelCount == 0)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Fatal_Plugin, "Config Validation", "Error: No usable data found in model config file: %s", configfile);
    }
}
