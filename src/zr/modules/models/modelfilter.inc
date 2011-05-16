/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          modelfilter.inc
 *  Type:          Module include
 *  Description:   Model query parser and filtering tools for the model db.
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

/*
Query syntax:

<option>
or
{<model> | <filter>[,...] | c:<collection> | <filter>[,...]:collection>} ...

Options:
default | no_change

Filters:
random | no_auth | zombie | human | both_teams | public | admin | mother_zombie
*/

/**
 * Query parsing modes. Default and NoChange is not handled by this module.
 */
enum ModelParserMode
{
    ModelParser_Invalid = -1,
    ModelParser_Default,    /** No query. Use default CS:S model that player selected when connecting. */
    ModelParser_NoChange,   /** No model change at all. Compatibility mode for other plugins. */
    ModelParser_First,      /** Pick first accessible model. Useful for prioritized queries. */
    ModelParser_Random,     /** Pick a random accessible model. */
    ModelParser_List        /** Add accessible models to a list for further processing. */
}

/**
 * @section Filtering flags.
 */
#define MODEL_FILTER_RANDOM         (1<<0)  /** Pick a random model if more than one. */
#define MODEL_FILTER_IGNORE_AUTH    (1<<1)  /** Ignore model restrictions. */
#define MODEL_FILTER_ZOMBIE         (1<<2)  /** Only zomibe models. */
#define MODEL_FILTER_HUMAN          (1<<3)  /** Only human models. */
#define MODEL_FILTER_BOTH_TEAMS     (1<<4)  /** Include models from both teams. */
#define MODEL_FILTER_PUBLIC         (1<<5)  /** Only public models (no restrictions). */
#define MODEL_FILTER_ADMIN          (1<<6)  /** Only admin models. */
#define MODEL_FILTER_MOTHER_ZOMB    (1<<7)  /** Only mother zombie models. */
/**
 * @endsection
 */

/**
 * Separator for filter lists.
 */
#define MODEL_FILTER_SEPARATOR ","

/**
 * Prefix to separate filter list and collection name.
 */
#define MODEL_COLLECTION_PREFIX ":"

/**
 * Maximum lenghth of the query string buffer.
 */
#define MODEL_MAX_QUERY_LEN 320

/**
 * Max length of filter name buffers.
 */
#define MODEL_FILTER_NAME_LEN 16


/**
 * Parses a model query string for a client.
 *
 * @param client    Client index. Used for authorization.
 *                  The server will always get access.
 * @param query     Model query string.
 * @param mode      Parser mode.
 * @param list      Optional. List to put models in if mode is ModelParser_List.
 *
 * @return          -2 if it's a predefined option (mode).
 *                  If in list mode: Number of models added to the list.
 *                  Otherwise model index if found, or -1 if no models found.
 */
ModelDB_ParseQuery(client, const String:query[], ModelParserMode:mode = ModelParser_First, Handle:list = INVALID_HANDLE)
{
    new model = -1;
    new filters = 0;
    new bool:asList = (mode == ModelParser_List);
    new listCount = 0;
    
    new entryCount;
    decl String:entries[MODEL_MAX_ENTRIES][MODEL_STRING_LEN];
    
    // Check if it's a predefined option.
    new ModelParserMode:option = ModelDB_StringToParserMode(query);
    if(option == ModelParser_Default || option == ModelParser_NoChange)
    {
        // Predefined option. Do nothing and leave it to other modules/plugins.
        return asList ? 0 : -2;
    }
    
    // Do a quick check for single model queries. If the query is longer than
    // the maximum name length it's definitely not a model name.
    if (strlen(query) <= MODEL_NAME_LEN)
    {
        model = ModelMgr_GetModelIndex(query);
        if (model >= 0)
        {
            if (asList)
            {
                PushArrayCell(list, model);
                return 1;
            }
            else
            {
                return model;
            }
        }
    }
    
    // Prepare filtered list.
    new Handle:filtered = asList ? list : CreateArray();
    
    // Separate entries.
    entryCount = ExplodeString(query, MODEL_ENTRY_SEPARATOR, entries, MODEL_MAX_ENTRIES, MODEL_STRING_LEN);
    
    // Check if failed.
    if (entryCount == 0)
    {
        return asList ? 0 : -1;
    }
    
    // Loop through entries.
    for (new entry = 0; entry < entryCount; entry++)
    {
        // Determine entry type.
        if (StrContains(entries[entry], MODEL_COLLECTION_PREFIX))
        {
            // It's a collection.
            model = ModelDB_ParseCollection(client, entries[entry], asList, list);
            
            if (asList)
            {
                listCount += model;
            }
            else if (model >= 0)
            {
                PushArrayCell(filtered, model);
                listCount++;
            }
        }
        else
        {
            // It's a model or filter. Test if it's a model.
            model = ModelMgr_GetModelIndex(entries[entry]);
            
            if (model >= 0)
            {
                // It's a model. Add to array/list.
                PushArrayCell(filtered, model);
                listCount++;
            }
            else
            {
                // It's a filter list. Parse it and get a model.
                filters = ModelDB_ParseFilters(entries[entry]);
                model = ModelDB_GetModel(client, filters, _, asList, list);
                
                // Add model to filtered list if valid.
                if (asList)
                {
                    listCount += model;
                }
                else if (model >= 0)
                {
                    PushArrayCell(filtered, model);
                    listCount++;
                }
            }
        }
    }
    
    // Check if using a list.
    if (asList)
    {
        // Stop here and return number of models added to the list.
        return listCount;
    }
    
    // Check if no models found.
    if (listCount == 0)
    {
        return -1;
    }
    
    switch (mode)
    {
        case ModelParser_First:
        {
            // Return the first model.
            return GetArrayCell(filtered, 0);
        }
        case ModelParser_Random:
        {
            // Return a random model.
            new randIndex = GetRandomInt(0, listCount - 1);
            return GetArrayCell(filtered, randIndex);
        }
    }
    
    // Invalid mode.
    return -1;
}

/**
 * Parse a filter flag list into a flag bit field. Invalid filters are skipped.
 *
 * @param filterList    List of filter flags separated by
 *                      MODEL_FILTER_SEPARATOR.
 *
 * @return              Filter flags in a bit field.
 */
ModelDB_ParseFilters(const String:filterList[])
{
    new filters = 0;
    decl String:filterEntries[MODEL_MAX_ENTRIES][MODEL_FILTER_NAME_LEN];
    
    new count = ExplodeString(filterList, MODEL_FILTER_SEPARATOR, filterEntries, MODEL_MAX_ENTRIES, MODEL_FILTER_NAME_LEN);
    
    for (new entry = 0; entry < count; entry++)
    {
        new filter = ModelDB_StringToFilter(filterEntries[entry]);
        if (filter > 0)
        {
            filters |= filter;
        }
    }
    
    return filters;
}

/**
 * Parses a collection entry in a model query.
 *
 * @param client    Client that will use the model. Used for authorization.
 *                  The server will always get access.
 * @param entry     Collection query entry.
 * @param asList    Optional. Return all accessible models in a list.
 *                  Defaults to false.
 * @param list      Optional. List to put models in.
 *
 * @return          Model index if asList is false, or number of models added
 *                  to list if asList is true.
 */
ModelDB_ParseCollection(client, const String:entry[], bool:asList = false, Handle:list = INVALID_HANDLE)
{
    new model = -1;
    new filters;
    decl String:filterBuffer[MODEL_STRING_LEN];
    
    new pos = SplitString(entry, MODEL_COLLECTION_PREFIX, filterBuffer, sizeof(filterBuffer));
    
    // Do a quick check to see if there's no filter (just c:collection).
    if (StrEqual(filterBuffer, "c", false))
    {
        // No filters.
        filters = 0;
    }
    else
    {
        filters = ModelDB_ParseFilters(filterBuffer);
    }
    
    // Get collection. The collection name is the other part of the entry.
    // TODO: Verify if pos is the position to the prefix (:) or right after.
    new collection = ModelMgr_GetCollectionIndex(entry[pos + 1]);
    
    if (collection >= 0)
    {
        // Get list.
        new Handle:collectionList = ModelCollectionData[collection][ModelCollection_Array];
        
        // Get model(s) from list.
        model = ModelDB_GetModel(client, filters, collectionList, asList, list);
        return model;
    }
    
    // No models found.
    return asList ? 0 : -1;
}

/**
 * Gets one or more models from a filtered list.
 *
 * @param client    Client that will use the model. Used for authorization.
 *                  The server will always get access.
 * @param filters   Filtering flags.
 * @param models    Optional. Custom list of model indexes. If not specified it
 *                  will use the entire model database.
 * @param asList    Optional. Return all accessible models in a list.
 *                  Defaults to false.
 * @param list      Optional. List to put models in.
 *
 * @return          Model index if asList is false, or number of models added
 *                  to list if asList is true.
 */
ModelDB_GetModel(client, filters, Handle:models = INVALID_HANDLE, bool:asList = false, Handle:list)
{
    new bool:customList = models != INVALID_HANDLE;
    new model = -1;
    
    // Check if there are no models.
    new count;
    count = customList ? GetArraySize(models) : ModelCount;
    if (count == 0)
    {
        return -1;
    }
    
    // Prepare temp array for second pass, or use the list if specified.
    new filterCount = 0;
    new Handle:filtered;
    filtered = asList ? list : CreateArray();
    
    // Loop through each model and test filters.
    for (new i = 0; i < count; i++)
    {
        model = customList ? GetArrayCell(models, i) : i; 
        
        if (ModelDB_FilterTest(client, model, filters))
        {
            // All filters passed, add model.
            PushArrayCell(filtered, model);
            filterCount++;
        }
        else
        {
            // Didn't pass filters. Skip model.
            continue;
        }
    }
    
    // Check if using a list.
    if (asList)
    {
        // Stop here and return number of models added to the list.
        return filterCount;
    }
    
    // Not using a list. Pick a model according to filter.
    
    // Check if no model passed the filter.
    if (filterCount == 0)
    {
        // No models found according to the filter.
        return -1;
    }
    
    // Pick random model if enabled.
    if (filters & MODEL_FILTER_RANDOM)
    {
        // Get random model.
        new randIndex = GetRandomInt(0, filterCount - 1);
        return GetArrayCell(filtered, randIndex);
    }
    else
    {
        // Pick first model.
        return GetArrayCell(filtered, 0);
    }
}

/**
 * Returns whether the specified model passes a filter.
 *
 * @param client    Client that will use the model. Used for authorization.
 *                  The server will always get access.
 * @param filters   Filtering flags.
 * @param model     Model index to test.
 *
 * @return          True if passed, false otherwise.
 */
bool:ModelDB_FilterTest(client, filters, model)
{
    // Cache relevant attributes for readability.
    new ModelTeam:team = ModelData[model][Model_Team];
    new bool:motherZombiesOnly = ModelData[model][Model_MotherZombiesOnly];
    new bool:adminsOnly = ModelData[model][Model_AdminsOnly];
    new bool:isHidden = ModelData[model][Model_IsHidden];
    new ModelAuthMode:authMode = ModelData[model][Model_AuthMode];
    
    // Authorization. Check if not ignored, and not authorized.
    if (client != 0     // Server always get access (for listings in console).
        && filters &~ MODEL_FILTER_IGNORE_AUTH
        && !ModelDB_IsPlayerAuthorized(client, model))
    {
        // Not authorized.
        return false;
    }
    
    // Team filters.
    if (filters &~ MODEL_FILTER_BOTH_TEAMS)
    {
        // Both teams flag is not set. Check individual team flags.
        
        // Zombie team.
        if (filters & MODEL_FILTER_ZOMBIE
            && team != ModelTeam_Zombies)
        {
            // Not a zombie model.
            return false;
        }
        
        // Human team.
        if (filters & MODEL_FILTER_HUMAN
            && team != ModelTeam_Humans)
        {
            // Not a human model.
            return false;
        }
    }
    
    // Public model.
    if (filters & MODEL_FILTER_PUBLIC
        && (isHidden || authMode == ModelAuth_Disabled))
    {
        // Not public (hidden or with auth mode enabled).
        return false;
    }
    
    // Admins only.
    if (filters & MODEL_FILTER_ADMIN
        && !adminsOnly)
    {
        // Not a admin model.
        return false;
    }
    
    // Mother zombies only.
    if (filters & MODEL_FILTER_MOTHER_ZOMB
        && !motherZombiesOnly)
    {
        // Not a mother zombie model.
        return false;
    }
    
    return true;
}


/****************************
 *   Conversion functions   *
 ****************************/

/**
 * Converts the filter name into a filter value.
 *
 * @param filterName    Name to convert.
 *
 * @return              Filter value, or -1 if failed.
 */
ModelDB_StringToFilter(const String:filterName[])
{
    if (strlen(filterName) == 0)
    {
        return -1;
    }
    else if (StrEqual(filterName, "random", false))
    {
        return MODEL_FILTER_RANDOM;
    }
    else if (StrEqual(filterName, "no_auth", false))
    {
        return MODEL_FILTER_IGNORE_AUTH;
    }
    else if (StrEqual(filterName, "zombie", false))
    {
        return MODEL_FILTER_ZOMBIE;
    }
    else if (StrEqual(filterName, "human", false))
    {
        return MODEL_FILTER_HUMAN;
    }
    else if (StrEqual(filterName, "both_teams", false))
    {
        return MODEL_FILTER_BOTH_TEAMS;
    }
    else if (StrEqual(filterName, "public", false))
    {
        return MODEL_FILTER_PUBLIC;
    }
    else if (StrEqual(filterName, "admin", false))
    {
        return MODEL_FILTER_ADMIN;
    }
    else if (StrEqual(filterName, "mother_zombie", false))
    {
        return MODEL_FILTER_MOTHER_ZOMB;
    }
    
    return -1;
}

/**
 * Converts a mode name into a parser mode value.
 *
 * @param mode      Mode name to convert.
 *
 * @return          Parser mode, or ModelParser_Invalid if failed.
 */
ModelParserMode:ModelDB_StringToParserMode(const String:mode[])
{
    if (strlen(mode) == 0)
    {
        return ModelParser_Invalid;
    }
    else if (StrEqual(mode, "default", false))
    {
        return ModelParser_Default;
    }
    else if (StrEqual(mode, "no_change", false))
    {
        return ModelParser_NoChange;
    }
    else if (StrEqual(mode, "first", false))
    {
        return ModelParser_First;
    }
    else if (StrEqual(mode, "random", false))
    {
        return ModelParser_Random;
    }
    
    return ModelParser_Invalid;
}
