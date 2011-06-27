/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          downloader.inc
 *  Type:          Module
 *  Description:   Adds listed files and directories in a file to the server's download list.
 *  Note:          Once a file is added to the table it can not be removed.
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
new Module:g_moduleDLR;

/**
 * Register this module.
 */
DLR_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Downloader");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "downloader");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Adds listed files and directories in a file to the server's download list.");
    moduledata[ModuleData_Dependencies][0] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleDLR = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleDLR, "Event_OnEventsRegister",               "DLR_OnEventsRegister");
    
    // Register config file(s) that this module will use.
    //ConfigMgr_Register(g_moduleDLR, "DLR_OnConfigReload", "");
}

/**
 * Register all events here.
 */
public DLR_OnEventsRegister()
{
    // Register all the events needed for this module.
    //EventMgr_RegisterEvent(g_moduleDLR, "Event_OnMyModuleEnable",               "DLR_OnMyModuleEnable");
    EventMgr_RegisterEvent(g_moduleDLR, "Event_OnMapStart",                     "DLR_OnMapStart");
}

/**
 * Plugin is loading.
 */
DLR_OnPluginStart()
{
    // Register the module.
    DLR_Register();
}

/**
 * The module that hooked this event callback has been enabled.
 * 
 * @param refusalmsg    The string that is printed if Plugin_Handled is returned and it is non-empty.
 * @param maxlen        The max length of the string.
 * 
 * @return              Return Plugin_Handled to stop enable, and Plugin_Continue to allow it.
 */
public Action:DLR_OnMyModuleEnable(String:refusalmsg[], maxlen)
{
}

/**
 * The map has started.
 */
public DLR_OnMapStart()
{
    DLR_Cache();
}

/**
 * Called when a registered config file (by this module) is manually reloaded.
 */
public DLR_OnConfigReload(configindex)
{
    DLR_Cache();
}

/**
 * Cache data from file.
 * 
 * @return      True if cached successfully, false if the file couldn't be found or no usable data was inside.
 */
bool:DLR_Cache()
{
    // Get the config path from the gamerules module.
    decl String:configfile[PLATFORM_MAX_PATH];
    GameRules_GetNamedConfig("downloader", configfile);
    
    
    if (ConfigMgr_ValidateFile(configfile))
        ConfigMgr_WriteString(g_moduleDLR, CM_CONFIGINDEX_FIRST, ConfigData_Path, CM_DATA_PATH, configfile);
    else
    {
        LogMgr_Print(g_moduleDLR, LogType_Fatal_Module, "Config Validation", "Invalid config file path defined in gamerules: \"%s\".  Disabling module.", configfile);
        return false;
    }
    
    new count;
    new Handle:hDLRData = ConfigMgr_CacheFile(g_moduleDLR, CM_CONFIGINDEX_FIRST, PLATFORM_MAX_PATH, count);
    if (hDLRData == INVALID_HANDLE)
    {
        LogMgr_Print(g_moduleDLR, LogType_Fatal_Module, "Downloads file", "Unexpected error loading data from downloads file: \"%s\"", configfile);
        return false;
    }
    
    // Process the cached data.
    
    CloseHandle(hDLRData);
    return true;
}
