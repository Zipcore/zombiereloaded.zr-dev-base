/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          modelcommands.inc
 *  Type:          Module include
 *  Description:   Model command interface (administration / debug).
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

ModelCmd_RegCommands()
{
    Project_RegConsoleCmd("models_list", ModelCmd_List, "Lists all models. Usage: <prefix>_models_list [<client target> <query>]");
    Project_RegConsoleCmd("models_dump", ModelCmd_Dump, "Dumps model data to the console. Usage: <prefix>_models_dump <model|index>");
}

public Action:ModelCmd_List(client, argc)
{
    // Check if the module is disabled.
    if (ModuleMgr_IsDisabled(g_moduleModelMgr))
    {
        return Plugin_Continue;
    }
    
    SetGlobalTransTarget(client);
    
    // TODO: Check if client has access to this command.
    /*if (...)
    {
        
    }*/
    
    // Check if listing all models.
    if (argc == 0)
    {
        // Print syntax.
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ModelMgr list usage", PROJECT_CMD_PREFIX);
        ReplyToCommand(client, "\n");
        
        // Print the list.
        ModelCmd_PrintList(client);
        
        return Plugin_Handled;
    }
    // Check if doing a query.
    else if (argc >= 2)
    {
        decl String:query[MODEL_MAX_QUERY_LEN];
        decl String:argbuffer[MODEL_STRING_LEN];
        query[0] = 0;
        
        // Join the last parameters into a string.
        // TODO: This should be added to a library as a utility.
        for (new arg = 2; arg <= argc; arg++)
        {
            GetCmdArg(arg, argbuffer, sizeof(argbuffer));
            StrCat(query, sizeof(query), argbuffer);
            
            // Add space, except on the last parameter.
            if (arg < argc)
            {
                StrCat(query, sizeof(query), " ");
            }
        }
        
        GetCmdArg(1, argbuffer, sizeof(argbuffer));
        new target;
        
        // Check if it's a server target.
        if (StrEqual(argbuffer, "@server", false))
        {
            target = 0;
        }
        else
        {
            // It's a name.
            target = FindTarget(client, argbuffer);
        }
        
        // Check if valid target.
        if (target < 0)
        {
            // Invalid target. FindTarget prints the error message.
            return Plugin_Handled;
        }
        
        // TODO: Validate query.
        /*if (!...)
        {
            // TODO: Print position of invalid part in the query.
            return Plugin_Handled;
        }*/
        
        // Prepare list.
        new Handle:list = CreateArray();
        
        // Run query.
        ModelDB_ParseQuery(target, query, ModelParser_List, list);
        
        // Print list.
        ModelCmd_PrintList(client, list);
        
        return Plugin_Handled;
    }
    else
    {
        // Print syntax.
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ModelMgr list usage", PROJECT_CMD_PREFIX);
        return Plugin_Handled;
    }
}

public Action:ModelCmd_Dump(client, argc)
{
    // Check if the module is disabled.
    if (ModuleMgr_IsDisabled(g_moduleModelMgr))
    {
        return Plugin_Continue;
    }
    
    return Plugin_Handled;
}


/***************
 *   Helpers   *
 ***************/

/**
 * Prints a model list to the console.
 *
 * @param client    Client or server index.
 * @param list      Optional. List of model indexes. If not specified it will
 *                  print all models.
 */
ModelCmd_PrintList(client, Handle:list = INVALID_HANDLE)
{
    static const String:fmt[] = "%-12d %-12s %s\n";
    static const String:fmtHeader[] = "%-12s %-12s %s\n--------------------------------------------------\n";
    
    new bool:hasList = (list != INVALID_HANDLE);
    new count = hasList ? GetArraySize(list) : ModelCount;
    
    SetGlobalTransTarget(client);
    
    if (count == 0)
    {
        TransMgr_PrintText(client, MsgFormat_Plugin, MsgType_Reply, _, _, "ModelMgr no models found");
        return;
    }
    
    // Print header.
    decl String:strIndex[32];
    decl String:strTeam[32];
    decl String:strName[32];
    Format(strIndex, sizeof(strIndex), "%t", "ZR header index");
    Format(strIndex, sizeof(strTeam), "%t", "ZR header team");
    Format(strIndex, sizeof(strName), "%t", "ZR header name");
    ReplyToCommand(client, fmtHeader, strIndex, strTeam, strName);
    
    decl String:teamHumans[32];
    decl String:teamZombies[32];
    Format(teamHumans, sizeof(teamHumans), "%t", "ZR team humans");
    Format(teamZombies, sizeof(teamZombies), "%t", "ZR team zombies");
    
    // Loop through each model.
    new model;
    for (new i = 0; i < count; i++)
    {
        model = hasList ? GetArrayCell(list, i) : i;
        
        // Print model info.
        if (ModelData[model][Model_Team] == ModelTeam_Zombies)
        {
            // Zombie.
            ReplyToCommand(client, fmt, model, teamZombies, ModelData[model][Model_Name]);
        }
        else
        {
            // Human.
            ReplyToCommand(client, fmt, model, teamHumans, ModelData[model][Model_Name]);
        }
    }
}
