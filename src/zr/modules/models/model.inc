/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          model.inc
 *  Type:          Module include
 *  Description:   Model data structure and constants.
 *
 *  Copyright (C) 2009-2013  Greyscale, Richard Helgeby
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
 * Type descriptor for model entries.
 */
new ObjectType:ModelType = INVALID_OBJECT_TYPE;
new bool:ModelTypeBuilt = false;

/*____________________________________________________________________________*/

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

/*____________________________________________________________________________*/

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

/*____________________________________________________________________________*/

#define MODEL_KEY_SIZE  20

/*____________________________________________________________________________*/

/**
 * Model object tag.
 */
enum Model
{
    INVALID_MODEL = 0
}

/*____________________________________________________________________________*/

/**
 * Builds the model type descriptor.
 */
stock Model_BuildType()
{
    if (!ModelTypeBuilt)
    {
        // Max length of key names.
        new keySize = ByteCountToCells(MODEL_KEY_SIZE);
        
        // Create type descriptor.
        ModelType = ObjLib_CreateType(
                ByteCountToCells(MODEL_STRING_LEN), // blockSize
                keySize,                            // keySzie
                Model_ErrorHandler);                // errorHandler
        
        // Add attributes.
        
        // displayname. User defined name to be displayed in menus. Nonempty string.
        ObjLib_AddKey(ModelType, "displayname", ObjDataType_String, ObjLib_GetStringConstraints(true));
        
        // name. Unique name to identify the model. Nonempty string.
        ObjLib_AddKey(ModelType, "name", ObjDataType_String, ObjLib_GetStringConstraints(true));
        
        // filename. File name of model (no file extension). // TODO: Add custom file validation.
        ObjLib_AddKey(ModelType, "filename", ObjDataType_String, ObjLib_GetStringConstraints(true));
        
        // path. Path to the model files. Nonempty string, path validation, include valve FS.
        ObjLib_AddKey(ModelType, "path", ObjDataType_String, ObjLib_GetStringConstraints(
                true,               // nonempty
                true,               // lowerLimit
                true,               // upperLimit
                1,                  // minLen
                PLATFORM_MAX_PATH,  // maxLen
                true,               // pathValidation
                false,              // fileValidation
                true,               // includeValveFS
                false,              // whitelist
                false,              // blacklist
                "",                 // whitelistChars
                ""));               // blacklistChars
        
        // team. Team the model belongs to.
        ObjLib_AddKey(ModelType, "team", ObjDataType_Cell, Model_GetTeamLookupConstraints());
        
        // mother_zombies_only. Only allow mother zombies to use the model.
        ObjLib_AddKey(ModelType, "mother_zombies_only", ObjDataType_Bool, ObjLib_GetBooleanLookupConstraints(BoolType_YesNo));
        
        // admins_only. Only allow admins to use the model.
        ObjLib_AddKey(ModelType, "admins_only", ObjDataType_Bool, ObjLib_GetBooleanLookupConstraints(BoolType_YesNo));
        
        // hidden. Exclude model in random selections.
        ObjLib_AddKey(ModelType, "hidden", ObjDataType_Bool, ObjLib_GetBooleanLookupConstraints(BoolType_YesNo));
        
        // auth_mode. Model authorization mode.
        ObjLib_AddKey(ModelType, "auth_mode", ObjDataType_Cell, Model_GetAuthLookupConstraints());
        
        // flags. Flag name list. Separated by comma (,). TODO: Flag list validator.
        ObjLib_AddKey(ModelType, "flags", ObjDataType_String, INVALID_OBJECT);
        
        // groups. Group name list. Separated by comma (,). TODO: Group list validator.
        ObjLib_AddKey(ModelType, "groups", ObjDataType_String, INVALID_OBJECT);
        
        ModelTypeBuilt = true;
    }
}

/*____________________________________________________________________________*/

/**
 * Deletes the model data type. Do not call before model data objects are
 * deleted.
 */
stock Model_DestroyType()
{
    if (ModelTypeBuilt)
    {
        // Delete type and constraint objects attached.
        ObjLib_DeleteType(ModelType, true, true);
        ModelTypeBuilt = false;
    }
}

/*____________________________________________________________________________*/

/**
 * Gets model display name.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store display name in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetDisplayName(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "displayname", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * Gets model name.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store name in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetName(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "name", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * Gets model file name.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store file name in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetFileName(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "filename", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * Gets model path.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store path in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetPath(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "path", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * Gets model team.
 *
 * @param model         Model object.
 *
 * @return              Model team.
 */
stock ModelTeam:Model_GetTeam(Model:model)
{
    return ModelTeam:ObjLib_GetCell(Object:model, "team");
}

/*____________________________________________________________________________*/

/**
 * Gets mother zombies only flag.
 *
 * @param model         Model object.
 *
 * @return              True if model is for mother zombies only, false
 *                      otherwise.
 */
stock bool:Model_GetMotherZombiesOnly(Model:model)
{
    return ObjLib_GetBool(Object:model, "mother_zombies_only");
}

/*____________________________________________________________________________*/

/**
 * Gets admins only flag.
 *
 * @param model         Model object.
 *
 * @return              True if model is for admins only, false otherwise.
 */
stock bool:Model_GetAdminsOnly(Model:model)
{
    return ObjLib_GetBool(Object:model, "admins_only");
}

/*____________________________________________________________________________*/

/**
 * Gets model hidden flag.
 *
 * @param model         Model object.
 *
 * @return              True if model is hidden, false otherwise.
 */
// TODO: Enable when migrating to new data storage.
/*stock bool:Model_IsHidden(Model:model)
{
    return ObjLib_GetBool(Object:model, "hidden");
}*/

/*____________________________________________________________________________*/

/**
 * Gets model auth mode.
 *
 * @param model         Model object.
 *
 * @return              Model auth mode.
 */
stock Auth_Modes:Model_GetAuthMode(Model:model)
{
    return Auth_Modes:ObjLib_GetCell(Object:model, "auth_mode");
}

/*____________________________________________________________________________*/

/**
 * Gets model auth flags.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store flags in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetFlags(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "flags", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * Gets model auth groups.
 *
 * @param model         Model object.
 * @param buffer        (Output) Buffer to store groups in.
 * @param maxlen        Size of buffer.
 *
 * @return              Number of characters copied.
 */
stock Model_GetGroups(Model:model, String:buffer[], maxlen)
{
    return ObjLib_GetString(Object:model, "groups", buffer, maxlen);
}

/*____________________________________________________________________________*/

/**
 * General error callback. Catch and print any errors that may occour with model
 * objects.
 *
 * This is the last error handler for models and once you reach this there isn't
 * much we can do further than logging it. This error handler will be overridden
 * by other handlers in the model manager module to properly handle various
 * errors. If we forgot to handle certain errors, this handler will be called
 * (which is a bug in ZR).
 *
 * The return value may be used to decide what to do next. In most cases it's
 * irrelevant and the value is ignored. But the keyvalue parser is using the
 * return value to abort or continue parsing.
 *
 * It would be considered a good habit to return a proper value even if it's
 * ignored in some cases.
 *
 * @param typeDescriptor    Related type descriptor.
 * @param errorType         Type of error.
 * @param message           Error message.
 * @param object            Related object, if available.
 * @param data              Data bundle with additional data, if available.
 *
 * @return                  What to do next.
 *                          * Plugin_Handled - Error is handled and further
 *                          processing should be aborted.
 *                          * Plugin_Continue - Continue processing if possible.
 *                          This is useful to let the parser continue parsing
 *                          remaining keys.
 */
public Action:Model_ErrorHandler(ObjectType:typeDescriptor, ObjLibError:errorType, const String:message[], Object:object, Object:data)
{
    LogMgr_Print(g_moduleModelMgr, LogType_Error, "Object Error", "[BUG] Type %d. %s", errorType, message);
    
    // Continue if possible. On critical errors the objectlib will abort
    // anyways.
    return Plugin_Continue;
}

/*____________________________________________________________________________*/

/**
 * Creates a team lookup constraint object. Must be deleted with closeHandles
 * flag set when no longer in use.
 */
stock Object:Model_GetTeamLookupConstraints()
{
    new Handle:teamNames = CreateArray(8, 2);
    new Handle:teamIDs = CreateArray(1, 2);
    
    // Valid team names.
    PushArrayString(teamNames, "zombies");
    PushArrayString(teamNames, "humans");
    
    // Matching team IDs.
    PushArrayCell(teamIDs, ModelTeam_Zombies);
    PushArrayCell(teamIDs, ModelTeam_Humans);
    
    return ObjLib_GetLookupConstraints(ObjLookupMethod_Array, teamNames, teamIDs);
}

/*____________________________________________________________________________*/

/**
 * Creates a auth mode lookup constraint object. Must be deleted with
 * closeHandles flag set when no longer in use.
 */
stock Object:Model_GetAuthLookupConstraints()
{
    new Handle:authModes = CreateArray(9, 6);
    new Handle:authIDs = CreateArray(1, 6);
    
    // Valid model auth modes.
    PushArrayString(authModes, "disabled");
    PushArrayString(authModes, "flag");
    PushArrayString(authModes, "group");
    PushArrayString(authModes, "either");
    PushArrayString(authModes, "both");
    PushArrayString(authModes, "all");
    
    // Matching auth mode IDs.
    PushArrayCell(authIDs, ModelAuth_Disabled);
    PushArrayCell(authIDs, ModelAuth_Flag);
    PushArrayCell(authIDs, ModelAuth_Group);
    PushArrayCell(authIDs, ModelAuth_Either);
    PushArrayCell(authIDs, ModelAuth_Both);
    PushArrayCell(authIDs, ModelAuth_All);
    
    return ObjLib_GetLookupConstraints(ObjLookupMethod_Array, authModes, authIDs);
}
