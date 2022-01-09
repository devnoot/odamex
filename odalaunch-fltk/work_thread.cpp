// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2021 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  Worker thread
//
//-----------------------------------------------------------------------------

#include "work_thread.h"

#include "thread.h"
#include <stddef.h>

#include <FL/Fl.H>

#include "db.h"
#include "main_window.h"
#include "net_packet.h"

enum awakeMessage_e
{
	AWAKE_NONE,
	AWAKE_REDRAW_SERVERS,
};

// Default list of master servers, usually official ones
static const char* DEFAULT_MASTERS[] = {"master1.odamex.net:15000",
                                        "voxelsoft.com:15000"};

static const int MASTER_TIMEOUT = 500;
static const int SERVER_TIMEOUT = 1000;
static const int USE_BROADCAST = false;
static const int QUERY_RETRIES = 2;

/**
 * @brief [WORKER] Get a server list from the master server.
 */
static void WorkerRefreshMaster()
{
	odalpapi::MasterServer master;

	for (size_t i = 0; i < ARRAY_LENGTH(::DEFAULT_MASTERS); i++)
	{
		master.AddMaster(::DEFAULT_MASTERS[i]);
	}

	odalpapi::BufferedSocket socket;
	master.SetSocket(&socket);

	// Query the masters with the timeout
	master.QueryMasters(::MASTER_TIMEOUT, ::USE_BROADCAST, ::QUERY_RETRIES);

	// Get the amount of servers found
	for (size_t i = 0; i < master.GetServerCount(); i++)
	{
		std::string address;
		uint16_t port;
		master.GetServerAddress(i, address, port);
		DB_AddServer(address, port);
	}
}

/**
 * @brief [MAIN] Proc run on Fl::awake.
 */
static void AwakeProc(void* data)
{
	switch ((ptrdiff_t)data)
	{
	case AWAKE_REDRAW_SERVERS:
		::g.mainWindow->redrawServers();
		return;
	default:
		return;
	}
}

/**
 * @brief [WORKER] Main worker thread proc.
 */
static int WorkerProc(void*)
{
	WorkerRefreshMaster();
	Fl::awake(AwakeProc, (void*)AWAKE_REDRAW_SERVERS);
	return 0;
}

/**
 * @brief [MAIN] Initializes the workers.
 */
void Work_Init()
{
	thread_ptr_t worker = thread_create(WorkerProc, NULL, "WorkerMain", 8192);
}