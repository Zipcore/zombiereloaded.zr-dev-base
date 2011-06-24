/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          project_events.inc
 *  Type:          Base
 *  Description:   Event hooking and definitions.  Any event-related stuff for your project should go here.
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

#include "zr/libraries/offsetlib"

/**
 * The max number of events that the project can have.
 */
#define PROJECT_MAX_EVENTS 128

/**
 * Event tag.
 */
enum ProjectEvent
{
    INVALID_EVENT = -1  /** Used as an invalid event. */
}

/**
 * Global variables to store each event's ID.
 */

// Cross-game events
new ProjectEvent:g_EvOnAllPluginsLoaded;
new ProjectEvent:g_EvOnPluginEnd;
new ProjectEvent:g_EvOnEventsRegister;
new ProjectEvent:g_EvOnEventsReady;
new ProjectEvent:g_EvOnAllModulesLoaded;
new ProjectEvent:g_EvOnModuleEnable;
new ProjectEvent:g_EvOnMyModuleEnable;
new ProjectEvent:g_EvOnModuleDisable;
new ProjectEvent:g_EvOnMyModuleDisable;
new ProjectEvent:g_EvOnMapStart;
new ProjectEvent:g_EvOnMapEnd;
new ProjectEvent:g_EvOnAutoConfigsBuffered;
new ProjectEvent:g_EvOnConfigsExecuted;
new ProjectEvent:g_EvOnClientAlphaChanged;
new ProjectEvent:g_EvOnClientPutInServer;
new ProjectEvent:g_EvOnClientConnected;
new ProjectEvent:g_EvOnClientDisconnect;
new ProjectEvent:g_EvOnClientCookiesCached;
new ProjectEvent:g_EvOnPlayerRunCmd;
new ProjectEvent:g_EvOnClientPostAdminCheck;

// Custom project events.
new ProjectEvent:g_EvOnClientReady;
new ProjectEvent:g_EvOnClientInfected;
new ProjectEvent:g_EvOnClientHuman;

// CS:S events.
#if defined PROJECT_GAME_CSS
new ProjectEvent:g_EvRoundStart;
new ProjectEvent:g_EvRoundFreezeEnd;
new ProjectEvent:g_EvRoundEnd;
new ProjectEvent:g_EvPlayerTeamPre;
new ProjectEvent:g_EvPlayerTeam;
new ProjectEvent:g_EvPlayerSpawn;
new ProjectEvent:g_EvPlayerSpawnPost;
new ProjectEvent:g_EvPlayerHurt;
new ProjectEvent:g_EvPlayerDeath;
new ProjectEvent:g_EvPlayerJump;
new ProjectEvent:g_EvWeaponFire;
//new ProjectEvent:g_EvWeaponEntityFire;
#endif

// TF2 events.
#if defined PROJECT_GAME_TF2

#endif

/**
 * Datatypes that can be passed with each event to modules.
 * Don't touch this!
 */
enum EventDataTypes
{
    DataType_Cell,
    DataType_CellRef,
    DataType_Float,
    DataType_FloatRef,
    DataType_Array,
    DataType_ArrayRef,
    DataType_String,
    DataType_StringRef,
}

/**
 * Common data type info.
 */

// When forwarding custom events with no data, use this.
stock any:g_CommonEventData1[][] = {{0}};

// When forwarding custom events whose data thats being passed matches these, you can use these.
stock EventDataTypes:g_CommonDataType1[] = {};
stock EventDataTypes:g_CommonDataType2[] = {DataType_Cell};

/**
 * Spawn post event delay time, in seconds.
 */
#define PROJECT_SPAWN_POST_DELAY    0.1

/**
 * Saved timestamp from the player spawn event.
 */
new Float:g_fPlayerSpawnTime[MAXPLAYERS + 1] = {-1.0, ...};

/**
 * Whether a post spawn event should be fired.
 */
new bool:g_SendPostSpawnEvent[MAXPLAYERS + 1];

/**
 * Saves states for the OnPlayerReady event.
 */
static bool:g_PlayerAdminChecked[MAXPLAYERS + 1];
static bool:g_PlayerCookiesLoaded[MAXPLAYERS + 1];
static bool:g_PlayerReadyEventFired[MAXPLAYERS + 1];

/**
 * Resets states for the OnPlayerReady event.
 */
static ResetReadyState(client)
{
    g_PlayerAdminChecked[client] = false;
    g_PlayerCookiesLoaded[client] = false;
    g_PlayerReadyEventFired[client] = false;
}

/**
 * Returns whether the player is ready or not.
 *
 * @param client    Client index.
 * @return          True if ready, false otherwise.
 */
static bool:IsClientReady(client)
{
    if (IsClientConnected(client) &&
        g_PlayerAdminChecked[client] &&
        g_PlayerCookiesLoaded[client] &&
        !g_PlayerReadyEventFired[client])
    {
        return true;
    }
    
    return false;
}

/**
 * Fires the ready event if the client is ready.
 *
 * @param client    Client index.
 */
static CheckReadyState(client)
{
    // Fire ready event if ready.
    if (IsClientReady(client))
    {
        // Forward event to all modules.
        new any:eventdata[1][1];
        eventdata[0][0] = client;
        EventMgr_Forward(g_EvOnClientReady, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
        
        g_PlayerReadyEventFired[client] = true;
    }
}


// **********************************************
//            SourceMod Forwards
// **********************************************

/**
 * All plugins have loaded.
 */
public OnAllPluginsLoaded()
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvOnAllPluginsLoaded, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * The map has started.
 */
public OnMapStart()
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvOnMapStart, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * The map has ended.
 */
public OnMapEnd()
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvOnMapEnd, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * This is called before OnConfigsExecuted but any time after OnMapStart.
 * Per-map settings should be set here. 
 */
public OnAutoConfigsBuffered()
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvOnAutoConfigsBuffered, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * All convars are set, cvar-dependent code should use this.
 */
public OnConfigsExecuted()
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvOnConfigsExecuted, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * Fires every game frame.
 * Events inside:
 * - OnClientAlphaChanged
 * - OnPlayerSpawnPost
 */
public OnGameFrame()
{
    // Create an array of clients that's remembered.
    static lastalpha[MAXPLAYERS + 1];
    new color[4];
    
    new Float:gameTime = GetGameTime();
    
    for (new client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;
        
        /****************************
         *   OnClientAlphaChanged   *
         ****************************/
        
        OffsLib_GetEntityColor(client, color);
        if (lastalpha[client] != color[3])
        {
            // Forward event to all modules.
            static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell};
            new any:eventdata[2][1];
            eventdata[0][0] = client;
            eventdata[1][0] = color[3];
            
            EventMgr_Forward(g_EvOnClientAlphaChanged, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
        }
        
        lastalpha[client] = color[3];
        
        
        /*************************
         *   OnPlayerSpawnPost   *
         *************************/
        #if defined PROJECT_GAME_CSS
        
        // Forward spawn post event if flag is set.
        if (g_SendPostSpawnEvent[client]
            && gameTime - PROJECT_SPAWN_POST_DELAY >= g_fPlayerSpawnTime[client])
        {
            new any:eventdata2[1][1];
            eventdata2[0][0] = client;
            EventMgr_Forward(g_EvPlayerSpawnPost, eventdata2, sizeof(eventdata2), sizeof(eventdata2[]), g_CommonDataType2);
            
            // Reset event flag.
            g_SendPostSpawnEvent[client] = false;
        }
        
        #endif
    }
}

/**
 * Client has joined the server.
 * 
 * @param client    The client index.
 */
public OnClientPutInServer(client)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientPutInServer, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
}

/**
 * Client successfully connected.
 *
 * @param client    The client index.
 */
public OnClientConnected(client)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientConnected, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
    
    // Check if ready to fire OnClientReady event.
    CheckReadyState(client);
}

/**
 * Client is disconnecting from the server.
 * 
 * @param client    The client index.
 */
public OnClientDisconnect(client)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientDisconnect, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
    
    // Clean up after OnClientReady event.
    ResetReadyState(client);
}

/**
 * A client's cookies have been cached from the database.
 * 
 * @param client    The client index.
 */
public OnClientCookiesCached(client)
{
    // This forward is called when clients connect and disconnect, so filter the bad calls out here.
    if (!IsClientConnected(client))
        return;
    
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientCookiesCached, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
    
    // Check if ready to fire OnClientReady event.
    g_PlayerCookiesLoaded[client] = true;
    CheckReadyState(client);
}

/**
 * Client is authorized.
 *
 * @param client    The client index.
 */
public OnClientPostAdminCheck(client)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvOnClientPostAdminCheck, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
    
    // Check if ready to fire OnClientReady event.
    g_PlayerAdminChecked[client] = true;
    CheckReadyState(client);
}

/**
 * @brief Called when a clients movement buttons are being processed
 *
 * @param client	Index of the client.
 * @param buttons	Copyback buffer containing the current commands (as bitflags - see entity_prop_stocks.inc).
 * @param impulse	Copyback buffer containing the current impulse command.
 * @param vel		Players desired velocity.
 * @param angles	Players desired view angles.
 * @param weapon	Entity index of the new weapon if player switches weapon, 0 otherwise.
 *  
 * @return 			Plugin_Handled to block the commands from being processed, Plugin_Continue otherwise.
 */
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
    if (!IsClientInGame(client))
        return Plugin_Continue;
    
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_CellRef, DataType_CellRef, DataType_ArrayRef, DataType_ArrayRef, DataType_CellRef};
    new any:eventdata[sizeof(eventdatatypes)][3];
    
    eventdata[0][0] = client;
    eventdata[1][0] = buttons;
    eventdata[2][0] = impulse;
    for (new x = 0; x < 3; x++)
        eventdata[3][x] = vel[x];
    for (new x = 0; x < 3; x++)
        eventdata[4][x] = angles[x];
    eventdata[5][0] = weapon;
    
    new Action:result = EventMgr_Forward(g_EvOnPlayerRunCmd, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
    
    // Copy event data back to this hook's variables, to let the engine use the new values.
    client = eventdata[0][0];
    buttons = eventdata[1][0];
    impulse = eventdata[2][0];
    for (new x = 0; x < 3; x++)
        vel[x] = eventdata[3][x];
    for (new x = 0; x < 3; x++)
        angles[x] = eventdata[4][x];
    weapon = eventdata[5][0];
    
    return result;
}

// **********************************************
//             Game Event Creating
// **********************************************

/**
 * This is called from the event manager in OnPluginStart.
 * Create the events you want to forward to modules here.
 */
stock EventMgr_CreateEvents()
{
    // Hook events to manage and forward to modules.
    
    // Cross-game events.
    g_EvOnAllPluginsLoaded =        EventMgr_CreateEvent("Event_OnAllPluginsLoaded");       /** All plugins have loaded. */
    g_EvOnPluginEnd =               EventMgr_CreateEvent("Event_OnPluginEnd");              /** Plugin is ending. */
    g_EvOnEventsRegister =          EventMgr_CreateEvent("Event_OnEventsRegister");         /** All events are created, modules should register them here. */
    g_EvOnEventsReady =             EventMgr_CreateEvent("Event_OnEventsReady");            /** All events are created AND registered by here.  Event priority can be changed here. */
    g_EvOnAllModulesLoaded =        EventMgr_CreateEvent("Event_OnAllModulesLoaded");       /** All modules have been registered. */
    g_EvOnModuleEnable =            EventMgr_CreateEvent("Event_OnModuleEnable");           /** A module has been enabled. */
    g_EvOnMyModuleEnable =          EventMgr_CreateEvent("Event_OnMyModuleEnable");         /** The module that hooked this event callback has been enabled. */
    g_EvOnModuleDisable =           EventMgr_CreateEvent("Event_OnModuleDisable");          /** A module has been disabled. */
    g_EvOnMyModuleDisable =         EventMgr_CreateEvent("Event_OnMyModuleDisable");        /** The module that hooked this event callback has been disabled. */
    g_EvOnMapStart =                EventMgr_CreateEvent("Event_OnMapStart");               /** The map has started. */
    g_EvOnMapEnd =                  EventMgr_CreateEvent("Event_OnMapEnd");                 /** The map has ended. */
    g_EvOnAutoConfigsBuffered =     EventMgr_CreateEvent("Event_OnAutoConfigsBuffered");    /** This is called before OnConfigsExecuted but any time after OnMapStart. */
    g_EvOnConfigsExecuted =         EventMgr_CreateEvent("Event_OnConfigsExecuted");        /** All convars are set, cvar-dependent code should use this. */
    g_EvOnClientAlphaChanged =      EventMgr_CreateEvent("Event_OnClientAlphaChanged");     /** Client cookies were cached from the database. */
    g_EvOnClientPutInServer =       EventMgr_CreateEvent("Event_OnClientPutInServer");      /** Client has joined the server. */
    g_EvOnClientConnected =         EventMgr_CreateEvent("Event_OnClientConnected");        /** Client successfully connected. */
    g_EvOnClientDisconnect =        EventMgr_CreateEvent("Event_OnClientDisconnect");       /** Client is disconnecting from the server. */
    g_EvOnClientCookiesCached =     EventMgr_CreateEvent("Event_OnClientCookiesCached");    /** Client cookies were cached from the database. */
    g_EvOnPlayerRunCmd =            EventMgr_CreateEvent("Event_OnPlayerRunCmd");           /** Client's movement buttons are being processed. */
    g_EvOnClientPostAdminCheck =    EventMgr_CreateEvent("Event_OnClientPostAdminCheck");   /** Client is authorized. */
    
    // Custom project events.
    g_EvOnClientReady =             EventMgr_CreateEvent("Event_OnClientReady");            /** Client is connected, admin checked and cookies are loaded. */
    g_EvOnClientInfected =          EventMgr_CreateEvent("Event_OnClientInfected");         /** Client has been infected as zombie. */
    g_EvOnClientHuman =             EventMgr_CreateEvent("Event_OnClientHuman");            /** Client has become human from the infected state. */
    
    // CS:S events.
    #if defined PROJECT_GAME_CSS
        g_EvRoundStart =            EventMgr_CreateEvent("Event_RoundStart");               /** Round has started. */
        g_EvRoundFreezeEnd =        EventMgr_CreateEvent("Event_RoundFreezeEnd");           /** Pre-round freezetime has finished. */
        g_EvRoundEnd =              EventMgr_CreateEvent("Event_RoundEnd");                 /** Round has ended. */
        g_EvPlayerTeamPre =         EventMgr_CreateEvent("Event_PlayerTeamPre");            /** Client has joined a team. (pre) */
        g_EvPlayerTeam =            EventMgr_CreateEvent("Event_PlayerTeam");               /** Client has joined a team. */
        g_EvPlayerSpawn =           EventMgr_CreateEvent("Event_PlayerSpawn");              /** Client has spawned. */
        g_EvPlayerSpawnPost =       EventMgr_CreateEvent("Event_PlayerSpawnPost");          /** Client has spawned (delayed event). */
        g_EvPlayerHurt =            EventMgr_CreateEvent("Event_PlayerHurt");               /** Client has been damaged. */
        g_EvPlayerDeath =           EventMgr_CreateEvent("Event_PlayerDeath");              /** Client has been killed. */
        g_EvPlayerJump =            EventMgr_CreateEvent("Event_PlayerJump");               /** Client has jumped. */
        g_EvWeaponFire =            EventMgr_CreateEvent("Event_WeaponFire");               /** Client has fired a weapon. */
        //g_EvWeaponEntityFire =      EventMgr_CreateEvent("Event_WeaponEntityFire");         /** Client has fired a weapon. (returns weapon entity index) */
    #endif
    
    // HL2DM events.
    #if defined PROJECT_GAME_HL2DM
    
    #endif
    
    // DOD events.
    #if defined PROJECT_GAME_DOD
    
    #endif
    
    // TF2 events.
    #if defined PROJECT_GAME_TF2
    
    #endif
    
    // L4D events.
    #if defined PROJECT_GAME_L4D
    #endif
    
    // L4D2 events.
    #if defined PROJECT_GAME_L4D2
    #endif
}

// **********************************************
//              Game Event Hooking
// **********************************************

/**
 * This is called from the event manager in OnPluginStart.
 * Hook the events you want to forward to modules here.
 */
stock EventMgr_HookEvents()
{
    // Hook events to manage and forward to modules.
    
    // CS:S events.
    #if defined PROJECT_GAME_CSS
        HookEvent("round_start", GameEvent_RoundStart);
        HookEvent("round_freeze_end", GameEvent_RoundFreezeEnd);
        HookEvent("round_end", GameEvent_RoundEnd);
        HookEvent("player_team", GameEvent_PlayerTeamPre, EventHookMode_Pre);
        HookEvent("player_team", GameEvent_PlayerTeam);
        HookEvent("player_spawn", GameEvent_PlayerSpawn);
        HookEvent("player_hurt", GameEvent_PlayerHurt);
        HookEvent("player_death", GameEvent_PlayerDeath);
        HookEvent("player_jump", GameEvent_PlayerJump);
        HookEvent("weapon_fire", GameEvent_WeaponFire);
    #endif
    
    // HL2DM events.
    #if defined PROJECT_GAME_HL2DM
    
    #endif
    
    // DOD events.
    #if defined PROJECT_GAME_DOD
    
    #endif
    
    // TF2 events.
    #if defined PROJECT_GAME_TF2
    
    #endif
    
    // L4D events.
    #if defined PROJECT_GAME_L4D
    #endif
    
    // L4D2 events.
    #if defined PROJECT_GAME_L4D2
    #endif
}

// **********************************************
//           CS:S Game Event Callbacks
// **********************************************

#if defined PROJECT_GAME_CSS

/**
 * Round has started.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_RoundStart(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvRoundStart, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * Pre-round freezetime has finished.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_RoundFreezeEnd(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    EventMgr_Forward(g_EvRoundFreezeEnd, g_CommonEventData1, 0, 0, g_CommonDataType1);
}

/**
 * Round has ended.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_RoundEnd(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = GetEventInt(event, "winner");
    
    EventMgr_Forward(g_EvRoundEnd, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
}

/**
 * Client has joined a team. (pre)
 * 
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerTeamPre(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_Cell, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][1];
    
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    eventdata[1][0] = GetEventInt(event, "team");
    eventdata[2][0] = GetEventInt(event, "oldteam");
    eventdata[3][0] = GetEventBool(event, "disconnect");
    
    new Action:result = EventMgr_Forward(g_EvPlayerTeamPre, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
    
    // If a module returns Plugin_Handled then stop the event from broadcasting to clients.
    if (result == Plugin_Handled)
        SetEventBroadcast(event, true);
}

/**
 * Client has joined a team.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerTeam(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_Cell, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][1];
    
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    eventdata[1][0] = GetEventInt(event, "team");
    eventdata[2][0] = GetEventInt(event, "oldteam");
    eventdata[3][0] = GetEventBool(event, "disconnect");
    
    EventMgr_Forward(g_EvPlayerTeam, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
}

/**
 * Client has spawned.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = client;
    
    EventMgr_Forward(g_EvPlayerSpawn, eventdata, sizeof(eventdata), sizeof(eventdata[]), g_CommonDataType2);
    
    // Prepare post spawn data. Used in OnGameFrame.
    g_fPlayerSpawnTime[client] = GetGameTime();
    g_SendPostSpawnEvent[client] = true;
}

/**
 * Client has been damaged.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerHurt(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_Cell, DataType_Cell, DataType_String, DataType_Cell, DataType_Cell, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][32];
    
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    eventdata[1][0] = GetClientOfUserId(GetEventInt(event, "attacker"));
    eventdata[2][0] = GetEventInt(event, "health");
    eventdata[3][0] = GetEventInt(event, "armor");
    GetEventString(event, "weapon", eventdata[4], sizeof(eventdata[]));
    eventdata[5][0] = GetEventInt(event, "dmg_health");
    eventdata[6][0] = GetEventInt(event, "dmg_armor");
    eventdata[7][0] = GetEventInt(event, "hitgroup");
    
    EventMgr_Forward(g_EvPlayerHurt, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
}

/**
 * Client has been killed.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_Cell, DataType_String, DataType_Cell};
    new any:eventdata[sizeof(eventdatatypes)][32];
    
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    eventdata[1][0] = GetClientOfUserId(GetEventInt(event, "attacker"));
    GetEventString(event, "weapon", eventdata[2], sizeof(eventdata[]));
    eventdata[3][0] = GetEventInt(event, "headshot");
    
    EventMgr_Forward(g_EvPlayerDeath, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
}

/**
 * Client has jumped.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_PlayerJump(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules.
    new any:eventdata[1][1];
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    
    EventMgr_Forward(g_EvPlayerJump, eventdata, 1, 1, g_CommonDataType2);
}

/**
 * Client has fired a weapon.
 *
 * @param event			Handle to event. This could be INVALID_HANDLE if every plugin hooking 
 *						this event has set the hook mode EventHookMode_PostNoCopy.
 * @param name			String containing the name of the event.
 * @param dontBroadcast	True if event was not broadcast to clients, false otherwise.
 */
public GameEvent_WeaponFire(Handle:event, const String:name[], bool:dontBroadcast)
{
    // Forward event to all modules. (WeaponFire)
    static EventDataTypes:eventdatatypes[] = {DataType_Cell, DataType_String};
    new any:eventdata[sizeof(eventdatatypes)][32];
    
    eventdata[0][0] = GetClientOfUserId(GetEventInt(event, "userid"));
    GetEventString(event, "weapon", eventdata[1], sizeof(eventdata[]));
    
    EventMgr_Forward(g_EvWeaponFire, eventdata, sizeof(eventdata), sizeof(eventdata[]), eventdatatypes);
    
    /*
    // Forward event to all modules. (WeaponEntityFire)
    new eventdata2[sizeof(eventdatatypes)][1];
    static EventDataTypes:eventdatatypes2[] = {DataType_Cell, DataType_Cell};
    
    decl String:classname[32];
    new weaponentity;
    
    // Loop through all entities.
    new maxentities = GetMaxEntities();
    for (new entity = MaxClients; entity < maxentities; entity++)
    {
        if (!IsValidEntity(entity))
            continue;
        
        GetEdictClassname(entity, classname, sizeof(classname));
        if (StrContains(classname, eventdata[1], false) == -1)
            continue;
        
        if (eventdata[0][0] == GetEntPropEnt(entity, Prop_Data, "m_hOwner"))
        {
            weaponentity = entity;
            break;
        }
    }
    
    eventdata2[0][0] = eventdata[0][0];
    eventdata2[1][0] = weaponentity;
    
    EventMgr_Forward(g_EvWeaponEntityFire, eventdata2, sizeof(eventdata2), sizeof(eventdata2[]), eventdatatypes2);
    */
}

#endif
