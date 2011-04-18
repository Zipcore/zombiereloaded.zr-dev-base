/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          modeldb.inc
 *  Type:          Module include
 *  Description:   Model database.
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

#include "zr/libraries/authlib"

#define MODELS_NAME_LEN     64
#define MODELS_STRING_LEN   255

/**
 * Maximum number of models.
 */
#define MODELS_MAX 64

/**
 * Maximum folder depth a model file can be located.
 */
#define MODELS_PATH_MAX_DEPTH 8

/**
 * Maximum string length of a folder a model file is located under.
 */
#define MODELS_PATH_DIR_MAX_LENGTH 32

/**
 * Separator for entries in lists (flags and groups).
 */
#define MODEL_ENTRY_SEPARATOR " "

/**
 * Model access settings.
 */
enum ModelAuthMode
{
    ModelAuth_Invalid = -1, /* Invalid authorization mode. */
    ModelAuth_Disabled,     /* No authorization. */
    ModelAuth_Flag,         /* Require flag. */
    ModelAuth_Group,        /* Require group. */
    ModelAuth_Either,       /* Require flag or group. */
    ModelAuth_Both,         /* Require flag and group. */
    ModelAuth_All,          /* Require all flags and all groups. */
}

/**
 * Avaliable teams for models. It's a mirror of ClassTeam to avoid dependency on
 * the class manager.
 */
enum ModelTeam
{
    ModelTeam_Invalid = -1,
    ModelTeam_Zombies,
    ModelTeam_Humans
}

/**
 * Model settings structure.
 */
enum ModelAttributes
{
    // General
    String:Model_DisplayName[MODELS_NAME_LEN],  /** User defined name to be displayed in menus. */
    String:Model_Name[MODELS_NAME_LEN],         /** Unique name to identify the model. */
    String:Model_FileName[MODELS_NAME_LEN],     /** File name of model (no file extension). */
    String:Model_Path[PLATFORM_MAX_PATH],       /** Path to model files. */
    ModelTeam:Model_Team,                       /** Which team the model belongs to. */
    
    // Restrictions
    bool:Model_MotherZombiesOnly,               /** Only allow mother zombies to use the model. */
    bool:Model_AdminsOnly,                      /** Only allow admins to use the model. */
    bool:Model_IsHidden,                        /** Exclude model from random selections. */
    
    // Advanced authorization
    ModelAuthMode:Model_AuthMode,               /** Model authorization mode. */
    String:Model_Flags[MODELS_STRING_LEN],      /** Flag name whitelist. Separated by comma (,). */
    String:Model_Groups[MODELS_STRING_LEN]      /** Group name whitelist. Separated by comma (,). */
}

/**
 * Parsed model data.
 */
new ModelData[MODELS_MAX][ModelAttributes];

/**
 * Number of valid models.
 */
new ModelCount;

/**
 * Number of models that failed validation. Used during loading.
 */
new ModelFailedCount;

/**
 * Loads a single model at the current position in the keyvalue parser.
 *
 * @param kvModels          Keyvalue parser handle.
 * @param sectionIndex      Current section index.
 * @param sectionName       Current section name.
 */
public KvCache:ModelDB_LoadModel(Handle:kvModels, sectionIndex, const String:sectionName[])
{
    decl String:buffer[MODELS_STRING_LEN];
    decl String:path[PLATFORM_MAX_PATH];
    decl String:fileName[MODELS_NAME_LEN];
    new bool:hasErrors = false;
    new temp;
    
    // Check if maximum number of models is reached.
    if (sectionIndex == MODELS_MAX)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Warning: Maximum number of models reached (%d), skipping other models from \"%s\" (%d).", MODELS_MAX, sectionName, sectionIndex);
        return KvCache_Hault;
    }
    
    
    // Load attributes
    // ---------------
    
    strcopy(ModelData[ModelCount][Model_Name], MODELS_NAME_LEN, sectionName);
    
    KvGetString(kvModels, "displayname", buffer, sizeof(buffer));
    strcopy(ModelData[ModelCount][Model_DisplayName], MODELS_NAME_LEN, buffer);
    
    KvGetString(kvModels, "filename", fileName, sizeof(fileName));
    strcopy(ModelData[ModelCount][Model_FileName], MODELS_NAME_LEN, fileName);
    
    KvGetString(kvModels, "path", path, sizeof(path));
    strcopy(ModelData[ModelCount][Model_Path], PLATFORM_MAX_PATH, path);
    
    KvGetString(kvModels, "team", buffer, sizeof(buffer));
    ModelData[ModelCount][Model_Team] = ModelDB_StringToTeam(buffer);
    
    KvGetString(kvModels, "mother_zombies_only", buffer, sizeof(buffer));
    ModelData[ModelCount][Model_MotherZombiesOnly] = (TransMgr_PhraseToBoolEx(BoolPhrase_YesNo, buffer) == PhraseToBool_True);
    
    KvGetString(kvModels, "admins_only", buffer, sizeof(buffer));
    ModelData[ModelCount][Model_AdminsOnly] = (TransMgr_PhraseToBoolEx(BoolPhrase_YesNo, buffer) == PhraseToBool_True);
    
    KvGetString(kvModels, "is_hidden", buffer, sizeof(buffer));
    ModelData[ModelCount][Model_IsHidden] = (TransMgr_PhraseToBoolEx(BoolPhrase_YesNo, buffer) == PhraseToBool_True);
    
    KvGetString(kvModels, "auth_mode", buffer, sizeof(buffer));
    ModelData[ModelCount][Model_AuthMode] = ModelDB_StringToAuthMode(buffer);
    
    KvGetString(kvModels, "flags", buffer, sizeof(buffer));
    strcopy(ModelData[ModelCount][Model_Flags], MODELS_STRING_LEN, buffer);
    
    KvGetString(kvModels, "groups", buffer, sizeof(buffer));
    strcopy(ModelData[ModelCount][Model_Groups], MODELS_STRING_LEN, buffer);
    
    
    // Validate attributes
    // -------------------
    
    // Build path and check if model file exist.
    strcopy(buffer, sizeof(buffer), path);
    StrCat(buffer, sizeof(buffer), ModelData[ModelCount][Model_FileName]);
    StrCat(buffer, sizeof(buffer), ".mdl");
    if (!FileExists(buffer))
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error: Invalid model \"path\" or \"filename\" value at \"%s\" (%d). File not found: \"%s\".", sectionName, sectionIndex, buffer);
        hasErrors = true;
    }
    
    // Validate team.
    if (ModelData[ModelCount][Model_Team] == ModelTeam_Invalid)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error: Invalid model \"team\" value at \"%s\" (%d).", sectionName, sectionIndex);
        hasErrors = true;
    }
    
    // Validate auth mode.
    if (ModelData[ModelCount][Model_AuthMode] == ModelAuth_Invalid)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error: Invalid model \"auth_mode\" value at \"%s\" (%d).", sectionName, sectionIndex);
        hasErrors = true;
    }
    
    // Validate flags.
    temp = Auth_ValidateFlags(ModelData[ModelCount][Model_Flags], MODEL_ENTRY_SEPARATOR);
    if (temp >= 0)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error: Invalid \"flags\" value (flag no. %d) at \"%s\" (%d).", temp + 1, sectionName, sectionIndex);
        hasErrors = true;
    }
    
    // Validate groups.
    temp = Auth_ValidateGroups(ModelData[ModelCount][Model_Groups], MODEL_ENTRY_SEPARATOR);
    if (temp >= 0)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error: Invalid \"groups\" value (group no. %d) at \"%s\" (%d).", temp + 1, sectionName, sectionIndex);
        hasErrors = true;
    }
    
    // Check if there are validation errors.
    if (hasErrors)
    {
        ModelFailedCount++;
        return KvCache_Ignore;
    }
    
    
    // Add model files to downloads table
    // ----------------------------------
    
    // Open directory with model files.
    new Handle:dir = OpenDirectory(path);
    
    // Check if failed.
    if (dir == INVALID_HANDLE)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Error opening directory: %s", path);
        return KvCache_Ignore;
    }
    
    // File counter for the current model.
    new downloadCount = 0;
    
    new FileType:type;
    decl String:file[64];
    decl String:fileShort[64];
    
    // Search for model files with the specified name and add them to
    // downloads table.
    while (ReadDirEntry(dir, file, sizeof(file), type))
    {
        // Skip if entry isn't a file.
        if (type != FileType_File)
        {
            continue;
        }
        
        // Find break point index in the string to get model name.
        // Add one to make space for null terminator.
        new breakpoint = FindCharInString(file, '.') + 1;
        strcopy(fileShort, breakpoint, file);
        
        // If this file doesn't match model name, then skip it.
        if (!StrEqual(fileName, fileShort, false))
        {
            continue;
        }
        
        // Format a full path string.
        strcopy(buffer, sizeof(buffer), path);
        Format(buffer, sizeof(buffer), "%s%s", buffer, file);
        
        AddFileToDownloadsTable(buffer);
        downloadCount++;
    }
    
    CloseHandle(dir);
    
    // Check if no model files were found.
    if (!downloadCount)
    {
        LogMgr_Print(g_moduleModelMgr, LogType_Error, "Config Validation", "Couldn't find any model files for \"%s\" (%d). Check \"filename\" and \"path\".", sectionName, sectionIndex);
        ModelFailedCount++;
        return KvCache_Ignore;
    }
    else
    {
        ModelCount++;
        return KvCache_Continue;
    }
}

/*
 *  CONVERSION FUNCTIONS
 */

/**
 * Converts a string to a team value.
 *
 * @param team  String to convert.
 * @return      Team value, or ModelTeam_Invalid on error.
 */
ModelTeam:ModelDB_StringToTeam(const String:team[])
{
    if (StrEqual(team, "zombies", false))
    {
        return ModelTeam_Zombies;
    }
    else if (StrEqual(team, "humans", false))
    {
        return ModelTeam_Humans;
    }
    
    return ModelTeam_Invalid;
}

/**
 * Converts a string to a model authorization mode.
 *
 * @param authMode  String to convert.
 *
 * @return          Model authorization mode, or ModelAuth_Invalid on error.
 */
ModelAuthMode:ModelDB_StringToAuthMode(const String:authMode[])
{
    if (strlen(authMode) == 0)
    {
        return ModelAuth_Invalid;
    }
    else if (StrEqual(authMode, "disabled", false))
    {
        return ModelAuth_Disabled;  /* No authorization. */
    }
    else if (StrEqual(authMode, "flag", false))
    {
        return ModelAuth_Flag;      /* Require flag. */
    }
    else if (StrEqual(authMode, "group", false))
    {
        return ModelAuth_Group;     /* Require group. */
    }
    else if (StrEqual(authMode, "either", false))
    {
        return ModelAuth_Either;    /* Require flag or group. */
    }
    else if (StrEqual(authMode, "both", false))
    {
        return ModelAuth_Both;      /* Require flag and group. */
    }
    else if (StrEqual(authMode, "all", false))
    {
        return ModelAuth_All;       /* Require all flags and all groups. */
    }
    
    return ModelAuth_Invalid;       /* Invalid authorization mode. */
}
