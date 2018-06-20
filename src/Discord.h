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
#pragma once
#include "Tipbot.h"
#include "sleepy_discord/websocketpp_websocket.h"

class Discord : public TIPBOT, public SleepyDiscord::DiscordClient {
public:
    using           SleepyDiscord::DiscordClient::DiscordClient;
    void            start();
    void            onMessage(SleepyDiscord::Message message);
    void            onReady(SleepyDiscord::Ready readyData);
    void            broadcastMsg(DiscordID channel, std::string message);
    void            broadcastDirectMsg(DiscordID user, std::string message);
    DiscordUser     getUserFromServer(DiscordID user);
    void            shutdown();
    Poco::Logger*   PLog;
private:
    void            refreshUserList();
    UserMessage     ConvertSleepyDiscordMsg(const SleepyDiscord::Message & message);
    int             getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id);
};