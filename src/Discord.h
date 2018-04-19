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
#include "sleepy_discord/websocketpp_websocket.h"
#include "types.h"

class ITNS_TIPBOT : public SleepyDiscord::DiscordClient {
public:
    using SleepyDiscord::DiscordClient::DiscordClient;

    int                     getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id);
    std::string             getDiscordDMChannel(DiscordID id);
    static DiscordID        convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id);


    void onMessage(SleepyDiscord::Message message);
};