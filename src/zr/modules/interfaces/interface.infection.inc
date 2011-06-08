/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          interface.infection.inc
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

new Function:g_IInfectionHumanToZombie;
stock IInfection_SetHumanToZombie(Function:func) { g_IInfectionHumanToZombie = func; }
stock IInfection_HumanToZombie(Module:module, client, attacker = 0, bool:mzombie)
{
    if (g_IInfectionHumanToZombie != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionHumanToZombie);
        Call_PushCell(client);
        Call_PushCell(attacker);
        Call_PushCell(mzombie);
        Call_Finish();
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use a function not supported by current game mode, disabling.", modulefullname);
    }
}

new Function:g_IInfectionZombieToHuman;
stock IInfection_SetZombieToHuman(Function:func) { g_IInfectionZombieToHuman = func; }
stock IInfection_ZombieToHuman(client)
{
    if (g_IInfectionZombieToHuman != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IInfectionZombieToHuman);
        Call_PushCell(client);
        Call_Finish();
    }
    else
    {
        decl String:modulefullname[MM_DATA_FULLNAME];
        ModuleMgr_ReadString(module, ModuleData_FullName, modulefullname, sizeof(modulefullname));
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use a function not supported by current game mode, disabling.", modulefullname);
    }
}

new Function:g_IInfectionAreZombiesPresent;
stock IInfection_SetAreZombiesPresent(Function:func) { g_IInfectionAreZombiesPresent = func; }
stock bool:IInfection_AreZombiesPresent()
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
        LogMgr_Print(module, LogType_Fatal_Module, "Infection Interface", "Module %s trying to use a function not supported by current game mode, disabling.", modulefullname);
    }
}
