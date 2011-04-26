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

#include "src/zr/libraries/functionlib"

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
 * Default sizes.
 */
#define MODEL_NAME_LEN     64
#define MODEL_STRING_LEN   255

/**
 * Cvar handles.
 */
new Handle:g_hCvarModelsFile;
new Handle:g_hCvarModelCollectionsFile;

/**
 * Config indexes.
 */
new ModelCfgIndex;
new ModelCollectionCfgIndex;

// Module components.
#include "src/zr/modules/models/modeldb"
#include "src/zr/modules/models/modelcollections"


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
    ConfigMgr_Register(g_moduleModelMgr, "ModelMgr_OnConfigReload", MODEL_FILE, ModelCfgIndex);
    ConfigMgr_Register(g_moduleModelMgr, "ModelMgr_OnConfigReload", MODEL_COLLECTION_FILE, ModelCollectionCfgIndex);
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
    EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnConfigsExecuted",         "ModelMgr_OnConfigsExecuted");
    EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnClientPutInServer",       "ModelMgr_OnClientPutInServer");
    //EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnClientDisconnect",      "ModelMgr_OnClientDisconnect");
    EventMgr_RegisterEvent(g_moduleModelMgr, "Event_OnClientPostAdminCheck",    "ModelMgr_OnClientPostAdminCheck");
    
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
    ModelDB_LoadModels();
    ModelDB_LoadCollections();
}

/**
 * Called when a registered config file (by this module) is manually reloaded.
 */
public ModelMgr_OnConfigReload(configindex)
{
    ModelDB_LoadModels();
    ModelDB_LoadCollections();
}

public ModelMgr_OnClientPutInServer(client)
{
    // Clear model access cache for this player in case something tries to
    // authorize a model before the cache is updated.
    ModelMgr_ClearPlayerAccess(client);
}

/**
 * Client is authorized.
 *
 * @param client    Client index.
 */
public ModelMgr_OnClientPostAdminCheck(client)
{
    // Update model access cache for this player.
    ModelMgr_UpdatePlayerAccess(client);
}


/******************
 *   Public API   *
 ******************/

/**
 * Gets the number of valid models loaded.
 */
public ModelMgr_GetModelCount()
{
    return ModelCount;
}

/**
 * Updates the entire model access cache for every connected player.
 * Useful when reloading a config.
 */
public ModelMgr_UpdateAccessCache()
{
    for (new client = 1; client < MaxClients + 1; client++)
    {
        if (IsClientConnected(client) && IsClientAuthorized(client))
        {
            ModelMgr_UpdatePlayerAccess(client);
        }
    }
}

/**
 * Updates the model access cache for a player.
 *
 * @param client    Player to cache.
 */
public ModelMgr_UpdatePlayerAccess(client)
{
    // Loop through every model and update authorized-flags.
    for (new model = 0; model < ModelCount; model++)
    {
        ModelHasAccess[client][model] = ModelDB_IsPlayerAuthorized(client, model);
    }
}

/**
 * Removes access to all models for a player.
 *
 * @param client    Player to clear.
 */
public ModelMgr_ClearPlayerAccess(client)
{
    for (new model = 0; model < ModelCount; model++)
    {
        ModelHasAccess[client][model] = false;
    }
}

/**
 * Returns whether a player has access to a model. Reads a cached value.
 *
 * @param client    Player index.
 * @param model     Model index.
 *
 * @return          True if the player has acces, false otherwise.
 */
public bool:ModelMgr_HasAccess(client, model)
{
    return ModelHasAccess[client][model];
}

/**
 * Returns whether the specified model index is valid or not.
 *
 * @param model     Model index to validate.
 *
 * @return          True if valid, false otherwise.
 */
public bool:ModelMgr_IsValidIndex(model)
{
    return model > 0 && (model < ModelCount < MODELS_MAX);
}

/**
 * Gets the index of a model.
 *
 * @param modelName     Name that identifies the model.
 *
 * @return              Model index if found, -1 otherwise.
 */
public ModelMgr_GetModelIndex(const String:modelName[])
{
    new model = -1;
    
    // Lookup the model name in the trie index.
    if (!GetTrieValue(ModelNameIndex, modelName, model))
    {
        return -1;
    }
    
    return model;
}

/*
 * Gets the full model file path for the specified model.
 *
 * @param model     Model index.
 * @param file      Destination string buffer.
 * @param maxlen    Size of destination buffer.
 *
 * @return          Number of cells written.
 */
public ModelMgr_GetModelFile(model, String:file[], maxlen)
{
    new count;
    
    // Validate index.
    if (!ModelMgr_IsValidIndex(model))
    {
        return 0;
    }
    
    // Build model path.
    count = strcopy(file, maxlen, ModelData[model][Model_Path]);
    
    // Append file name.
    count += StrCat(file, maxlen, ModelData[model][Model_FileName]);
    
    // Append file extension.
    count += StrCat(file, maxlen, ".mdl");
    
    return count;
}
