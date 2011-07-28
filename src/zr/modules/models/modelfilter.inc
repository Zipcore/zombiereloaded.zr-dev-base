/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          modelfilter.inc
 *  Type:          Module include
 *  Description:   Model filtering tools for the model db.
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
 * Gets one or more models from a filtered list.
 *
 * @param client    Client that will use the model. Used for authorization.
 *                  The server will always get access.
 * @param filters   Filtering flags. The "random" filter will pick a single
 *                  model that match the filters.
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
    new bool:customList = (models != INVALID_HANDLE);
    new model = -1;
    
    // Check if there are no models.
    new count;
    count = customList ? GetArraySize(models) : ModelCount;
    if (count == 0)
    {
        return asList ? 0 : -1;
    }
    
    // Prepare temp array for second pass, or use the list if specified.
    new filterCount = 0;
    new Handle:filtered;
    if (asList && filters & MODEL_FILTER_RANDOM)
    {
        // Using a list, but a random filter is enabled. This still requires a
        // temp array.
        filtered = CreateArray();
    }
    else
    {
        filtered = asList ? list : CreateArray();
    }
    
    // Loop through each model and test filters.
    for (new i = 0; i < count; i++)
    {
        model = customList ? GetArrayCell(models, i) : i; 
        if (ModelDB_FilterTest(client, filters, model))
        {
            // All filters passed, add model.
            PushArrayCell(filtered, model);
            filterCount++;
        }
    }
    
    // Check if no model passed the filter.
    if (filterCount == 0)
    {
        // No models found according to the filter.
        
        if (!asList || filters & MODEL_FILTER_RANDOM)
        {
            // Temp array exist when not using a list, or when a random filter
            // is enabled.
            CloseHandle(filtered);
        }
        
        return asList ? 0 : -1;
    }
    
    // Pick random model if enabled.
    if (filters & MODEL_FILTER_RANDOM)
    {
        // Get random model.
        new randIndex = Math_GetRandomInt(0, filterCount - 1);
        model = GetArrayCell(filtered, randIndex); 
        CloseHandle(filtered);      // Temp array, NOT the list.
        
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
    
    // If using a list, return number of models added.
    if (asList)
    {
        return filterCount;
    }
    
    // Not using a list. Pick first model.
    model = GetArrayCell(filtered, 0);
    CloseHandle(filtered);
    return model;
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
public bool:ModelDB_FilterTest(client, filters, model)
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
    if (!(filters & MODEL_FILTER_BOTH_TEAMS))
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
