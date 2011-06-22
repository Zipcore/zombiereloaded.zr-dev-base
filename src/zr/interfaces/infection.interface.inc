/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          infection.interface.inc
 *  Type:          Module
 *  Description:   An interface for common infection-type functions for modules to use.
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

#if defined _iinfection_included
 #endinput
#endif
#define _iinfection_included

new Function:g_IInfectionAreZombiesPresent = INVALID_FUNCTION;
stock IInfection_SetAreZombiesPresent(Function:func) { g_IInfectionAreZombiesPresent = func; }
stock bool:IInfection_AreZombiesPresent(Module:module)
{
    if (g_IInfectionAreZombiesPresent != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionAreZombiesPresent);
        new bool:value;
        Call_Finish(value);
        return value;
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use interface function IInfection_AreZombiesPresent which is not supported by current core, disabling.", modulefullname);
    }
    return false;
}

new Function:g_IInfectionIsClientMZombie = INVALID_FUNCTION;
stock IInfection_SetIsClientMZombie(Function:func) { g_IInfectionIsClientMZombie = func; }
stock bool:IInfection_IsClientMZombie(Module:module, client)
{
    if (g_IInfectionIsClientMZombie != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionIsClientMZombie);
        Call_PushCell(client);
        new bool:value;
        Call_Finish(value);
        return value;
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use interface function IInfection_IsClientMZombie which is not supported by current core, disabling.", modulefullname);
    }
    return false;
}

new Function:g_IInfectionClientToZombie = INVALID_FUNCTION;
stock IInfection_SetClientToZombie(Function:func) { g_IInfectionClientToZombie = func; }
stock IInfection_ClientToZombie(Module:module, client, attacker = 0, bool:mzombie)
{
    if (g_IInfectionClientToZombie != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionClientToZombie);
        Call_PushCell(client);
        Call_PushCell(attacker);
        Call_PushCell(mzombie);
        Call_Finish();
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use interface function IInfection_ClientToZombie which is not supported by current core, disabling.", modulefullname);
    }
}

new Function:g_IInfectionClientToHuman = INVALID_FUNCTION;
stock IInfection_SetClientToHuman(Function:func) { g_IInfectionClientToHuman = func; }
stock IInfection_ClientToHuman(Module:module, client)
{
    if (g_IInfectionClientToHuman != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionClientToHuman);
        Call_PushCell(client);
        Call_Finish();
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use interface function IInfection_ClientToHuman which is not supported by current core, disabling.", modulefullname);
    }
}
