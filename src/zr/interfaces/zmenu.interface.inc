/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          zmenu.interface.inc
 *  Type:          Interface
 *  Description:   An interface for modules to link menus to zmenu.
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

#if defined _izmenu_included
 #endinput
#endif
#define _izmenu_included

/**
 * Interface function ID cache.
 */
new Function:g_IZMenuGetMenu = INVALID_FUNCTION;

/**
 * Interface functions.
 */

stock Handle:IZMenu_GetMenu()
{
    if (g_IZMenuGetMenu != INVALID_FUNCTION)
    {
        Call_StartFunction(GetMyHandle(), g_IZMenuGetMenu);
        new Handle:result;
        Call_Finish(result);
        return result;
    }
    else
        ThrowError("This function isn't implemented!");
    
    return INVALID_HANDLE;
}
