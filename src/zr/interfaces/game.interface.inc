/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:           game.interface.inc
 *  Type:           Interface
 *  Description:    This interface provides access to abstract mod events and
 *                  functions that are not available in the base Source engine.
 *                  For instance functions like RespawnPlayer or TerminateRound.
 *
 *                  This interface is paired with the zr_core module which is
 *                  responsible for loading the appropriate mod adatper module
 *                  that will implement this interface.
 *
 *                  Events or functions may be faked on mods that doesn't
 *                  natively support it. The gameadapter will create these
 *                  common events, but a mod specific adapter module will be
 *                  responsible for firing these events.
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

#if defined _igame_included
 #endinput
#endif
#define _igame_included

/*____________________________________________________________________________*/

/**
 * Supported games.
 */
enum Game
{
    Game_Unknown = -1,
    Game_CSS,
    Game_CSGO,
    Game_TF2
}

/*____________________________________________________________________________*/

/**
 * Virtual teams used internally in ZR.
 */
enum VTeam
{
    VTeam_Invalid = -1,     /** Invalid team. */
    VTeam_Unassigned = 0,   /** Client isn't on a team. */
    VTeam_Spectator,        /** Client is a spectator. */
    VTeam_Zombie,           /** Client is a zombie. */
    VTeam_Human             /** Client is a human. */
}

/*____________________________________________________________________________*/

/*
TODO - Remaining mod specific stuff that must be abstracted:

Events (created by gameadapter module, fired by mod specific adapter module):
* player_death
* player_hurt
* player_spawn
* player_team
* round_start
* round_end
* round_freeze_end
* player_jump
* weapon_fire

They are slightly different between mods (or doesn't exist at all and need to be faked).

Cvars. Mod adapter modules should probably handle these. If other modules need
to read values we can provide get-functions in this interface:
* mp_friendlyfire
* mp_restartgame
* mp_autoteambalance
* mp_limitteams
* mp_roundtime

Others:
* team index (solved by teamlib, but we need to move ZR stuff to another place)
* grenades (napalm feature) and knife
* removal of objectives (hostages, rescue zone, and other mod specific objective entities)
* buy command
* weapon slots
* damage flags
* money
* night vision
*/

/**
 * Interface function ID cache.
 */
new Function:g_IGameRespawnPlayer = INVALID_FUNCTION;
new Function:g_IGameSwitchTeam = INVALID_FUNCTION;
new Function:g_IGameTerminateRound = INVALID_FUNCTION;
new Function:g_IGameDropWeapon = INVALID_FUNCTION;
new Function:g_IGameFlashlightIsOn = INVALID_FUNCTION;
new Function:g_IGameFlashlightOn = INVALID_FUNCTION;
new Function:g_IGameFlashlightOff = INVALID_FUNCTION;

/*____________________________________________________________________________*/

/**
 * Respawns a client.
 *
 * @param client        Player's index.
 */
stock IGame_RespawnPlayer(client)
{
    Interface_ValidateOrFail(g_IGameRespawnPlayer);
    
    Call_StartFunction(GetMyHandle(), g_IGameRespawnPlayer);
    Call_PushCell(client);
    Call_Finish();
}

/*____________________________________________________________________________*/

/**
 * Switches the player's team.
 *
 * @param client        Player's index.
 * @param team          Team index.
 */
stock IGame_SwitchTeam(client, VTeam:team)
{
    Interface_ValidateOrFail(g_IGameSwitchTeam);
    
    Call_StartFunction(GetMyHandle(), g_IGameSwitchTeam);
    Call_PushCell(client);
    Call_PushCell(team);
    Call_Finish();
}

/*____________________________________________________________________________*/

/**
 * Forces round to end.
 *
 * @param winner        The winning team (virtual team).
 *                      TODO: It could be possible to allow VTeam_Invalid to
 *                      terminate the round without a winner (round draw, etc).
 * @param               (Optional) Delay in seconds before terminating round.
 *                      Default is 3 seconds.
 */
stock IGame_TerminateRound(VTeam:winner, Float:delay = 3.0)
{
    Interface_ValidateOrFail(g_IGameTerminateRound);
    
    Call_StartFunction(GetMyHandle(), g_IGameTerminateRound);
    Call_PushCell(winner);
    Call_Finish();
}

/*____________________________________________________________________________*/

/**
 * Forces player to drop or toss their weapon.
 *
 * @param client        Player's index.
 * @param weaponIndex   Index of weapon to drop.
 * @param toos          True to toss weapon (with velocity), or false to just
 *                      drop weapon.
 */
stock IGame_DropWeapon(client, weaponIndex, bool:toss = false)
{
    Interface_ValidateOrFail(g_IGameDropWeapon);
    
    Call_StartFunction(GetMyHandle(), g_IGameDropWeapon);
    Call_PushCell(client);
    Call_PushCell(weaponIndex);
    Call_PushCell(toss);
    Call_Finish();
}

/*____________________________________________________________________________*/

/**
 * Returns whether the player's flashlight is turned on.
 *
 * Note: This is an optional function. There's no guarantee that it's
 *       implemented in all game mods.
 *
 * @param client        Player's index.
 *
 * @return              True if on, false otherwise.
 */
stock bool:IGame_FlashlightIsOn(client)
{
    Interface_ValidateOrFail(g_IGameFlashlightIsOn);
    
    new bool:result = false;
    
    Call_StartFunction(GetMyHandle(), g_IGameFlashlightIsOn);
    Call_PushCell(client);
    Call_Finish(result);
    
    return result;
}

/*____________________________________________________________________________*/

/**
 * Turns player's flashlight on.
 *
 * Note: This is an optional function. There's no guarantee that it's
 *       implemented in all game mods.
 *
 * @param client        Player's index.
 */
stock IGame_FlashlightOn(client)
{
    Interface_ValidateOrFail(g_IGameFlashlightOn);
    
    Call_StartFunction(GetMyHandle(), g_IGameFlashlightOn);
    Call_PushCell(client);
    Call_Finish();
}

/*____________________________________________________________________________*/

/**
 * Turns player's flashlight off.
 *
 * Note: This is an optional function. There's no guarantee that it's
 *       implemented in all game mods.
 *
 * @param client        Player's index.
 */
stock IGame_FlashlightOff(client)
{
    Interface_ValidateOrFail(g_IGameFlashlightOff);
    
    Call_StartFunction(GetMyHandle(), g_IGameFlashlightOff);
    Call_PushCell(client);
    Call_Finish();
}
