/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          classmenus.inc
 *  Type:          Module include
 *  Description:   Manages class menus.
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

// Note: Global variables are defined in classmanager.inc.

/*
    additional menu option ideas:
        - ability to select which mother zombie to become (if server settings
          (allow it)
    player preference ideas:
        - prefer admin classes in random selections (if player is admin)
*/

/**
 * Builds the main class menu (zclass) for a player.
 *
 * This menu is used to display information about the current and eventually
 * next class being used - in addition to options for selecting human and zombie
 * classes.
 *
 * @param client    The client (index) this menu is built for.
 *
 * @return          Handle to menulib menu or INVALID_HANDLE if failed.
 */
Handle:ClassMgr_CreateMainMenu(client)
{
    new Handle:menu;
    
    decl String:id[ML_DATA_ID];
    decl String:humanName[CLASS_NAME_LEN];
    decl String:zombieName[CLASS_NAME_LEN];
    decl String:currentName[CLASS_NAME_LEN];
    decl String:nextName[CLASS_NAME_LEN];
    id[0] = 0;
    humanName[0] = 0;
    zombieName[0] = 0;
    currentName[0] = 0;
    nextName[0] = 0;
    
    new human;
    new zombie;
    new nextHuman;
    new nextZombie;
    
    // Generate a unique ID based on client and game time. This isn't used by
    // anything, but menulib requires it.
    new Float:time = GetGameTime();
    Format(id, sizeof(id), "%d %0.2", client, time);
    
    // Build the menu.
    menu = MenuLib_CreateMenu(id,                           // id
                              INVALID_FUNCTION,             // pre-callback
                              ClassMgr_MenuMainHandler,     // callback
                              "Class main menu title",      // title
                              true,                         // translate title
                              false,                        // forceback
                              true);                        // temp menu
    
    // Check if failed.
    if (menu == INVALID_HANDLE)
    {
        return INVALID_HANDLE;
    }
    
    // Get current and next classes.
    // (Note: Next classes are classes that will be used next time the player
    //        spawn. They may not be set.)
    human = g_ClassSelected[client][ClassTeam_Humans];
    zombie = g_ClassSelected[client][ClassTeam_Zombies];
    nextHuman = g_ClassNextClass[client][ClassTeam_Humans];
    nextZombie = g_ClassNextClass[client][ClassTeam_Zombies];
    
    
    // get whether player can change class - per team.
    // disable buttons the player doesn't have access to.
    
    // add buttons.
    
    /* Layout
    ---------------------------
    Class Selection
    Current class: <class name>
    [Next class: <class name>]
    <\n>
    1. Select Zombie
    2. Select Human
    
    [8. Back]
    0. Exit
    ---------------------------
    */
    
    return menu;
}
