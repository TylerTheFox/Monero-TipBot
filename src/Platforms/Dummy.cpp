/*
Copyright(C) 2018 Brandan Tyler Lasley

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
#include "Dummy.h"
#include "../Core/Config.h"
#include "Poco/Thread.h"
#include "../Core/RPCManager.h"

Dummy::Dummy(const std::string &) {}

Dummy::~Dummy()
{
}

void Dummy::start() {
    // Start application
    tipbot_init();

    RPCMan->setBotUser(0);
    while (!GlobalConfig.General.Shutdown) { Poco::Thread::sleep(1); } 
    while (GlobalConfig.General.Threads) { Poco::Thread::sleep(1); };
}

void Dummy::broadcastMsg(DiscordID channel, std::string message) {}
void Dummy::broadcastDirectMsg(DiscordID user, std::string message) {}
DiscordUser _UknownUser2 = { 0, FIND_USER_UNKNOWN_USER, 0 };
const DiscordUser & Dummy::getUserFromServer(DiscordID user) { return _UknownUser2; }
void Dummy::_shutdown() {}