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

struct DiscordUser
{
    DiscordID           id;
    std::string         username;
    std::uint64_t       join_epoch_time;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(id), CEREAL_NVP(username), CEREAL_NVP(join_epoch_time));
    }
}; 

inline bool operator<(const DiscordUser &a, const DiscordUser &b)
{
    return a.id < b.id;
}

#define DISCORD_USER_CACHE_FILENAME "DISCORDDATA.json"
class ITNS_TIPBOT : public SleepyDiscord::DiscordClient {
public:
    using SleepyDiscord::DiscordClient::DiscordClient;

    int                                             getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id);
    std::string                                     getDiscordDMChannel(DiscordID id);

    template<class t>
    static DiscordID                                convertSnowflakeToInt64(t id);
    const struct DiscordUser &                      findUser(const DiscordID & id);


    void                                            onMessage(SleepyDiscord::Message message);
    void                                            onReady(SleepyDiscord::Ready readyData);
private:
    void                                            refreshUserList();
    void                                            saveUserList();
    void                                            loadUserList();
    std::map<std::uint64_t, std::set<DiscordUser>>  UserList;
    SleepyDiscord::User                             BotUser;
};

template<class t>
DiscordID ITNS_TIPBOT::convertSnowflakeToInt64(t id)
{
    return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}