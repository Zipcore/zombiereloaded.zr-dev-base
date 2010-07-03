/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          speed.inc
 *  Type:          Module
 *  Description:   Handles setting client speed.
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
 * TODO:  Change values in OnPlayerRunCmd to the client's set speed.
 */

/**
 * This module's identifier.
 */
new Module:g_moduleSpeed;

/**
 * Register this module.
 */
Speed_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], CM_DATA_FULLNAME, "Speed");
    strcopy(moduledata[ModuleData_ShortName], CM_DATA_SHORTNAME, "speed");
    strcopy(moduledata[ModuleData_Description], CM_DATA_DESCRIPTION, "Test speed");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleSpeed = ModuleMgr_Register(moduledata);
    
    // Register all the events needed for this module.
    EventMgr_RegisterEvent(g_moduleSpeed, "Event_OnClientPutInServer",      "Speed_OnClientPutInServer");
    EventMgr_RegisterEvent(g_moduleSpeed, "Event_OnClientDisconnect",       "Speed_OnClientDisconnect");

    
    // Register config file(s) that this module will use.
    // ConfigMgr_Register(g_moduleSpeed, "Speed_OnConfigReload", "configs/<projectshortname>/<config>.txt");
}

/**
 * Plugin is loading.
 */
Speed_OnPluginStart()
{
    // Register the module.
    Speed_Register();
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public Speed_OnClientPutInServer(client)
{
    SDKHook(client, SDKHook_PreThinkPost, Speed_PreThinkPost);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public Speed_OnClientDisconnect(client)
{
    SDKUnhook(client, SDKHook_PreThinkPost, Speed_PreThinkPost);
}

public Speed_PreThinkPost(client)
{
    if (!IsPlayerAlive(client))
        return;
    
    // Note: Default is around 200.0 - 250.0.
    
    new Float:curspeed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
    SetEntPropFloat(client, Prop_Data, "m_flMaxspeed", curspeed + ___);
}

public Action:Speed_OnPlayerRunCmd(client, Float:vel[3])
{
    if (!IsPlayerAlive(client))
        return;
    
    // x-axis speed.
    if (vel[0] < 0.0)
        vel[0] = ___;
    
    else if (vel[0] > 0.0)
        vel[0] = ___;
    
    // y-axis speed.
    if (vel[1] < 0.0)
        vel[1] = ___;
    
    else if (vel[1] > 0.0)
        vel[1] = ___;
     
    return Plugin_Continue; 
}
