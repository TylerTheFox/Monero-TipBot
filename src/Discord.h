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
#include "Poco/NumberParser.h"
#include "types.h"
#include "cereal/cereal.hpp"
#include <set>
#include <map>
#include <vector>
#include <memory>

#define        VERSION_MAJOR 1
#define        VERSION_MINOR 4

extern const char *aboutStr;

const DiscordID DiscordAdmins[] =
{
    380370690030829578, // Valiant
    144619872444219392, // ddvs1
    266700783897018369, // SlowGrowth
    415162452725202944, // iedemam
    345699014806732800, // ThePigwee
    206430811430322176, // Brandan
};

const std::string AllowChannelTypeNames[] =
{
    "Public Channel Only",
    "Direct Message Only"
};

enum AllowChannelTypes
{
    Any = -1,
    Public = 0,
    Private = 1
};

struct DiscordUser
{
    DiscordID                   id{};
    std::string                 username;
    std::uint64_t               join_epoch_time{};
    mutable std::uint64_t       faucet_epoch_time{};
    mutable std::uint64_t       total_faucet_itns_donated{};
    mutable std::uint64_t       total_faucet_itns_sent{};
    mutable std::uint64_t       total_itns_given{};
    mutable std::uint64_t       total_itns_recieved{};
    mutable std::uint64_t       total_itns_withdrawn{};
    template <class Archive>
    void serialize(Archive & ar)
    {
        ar( CEREAL_NVP(id), 
            CEREAL_NVP(username), 
            CEREAL_NVP(join_epoch_time), 
            CEREAL_NVP(faucet_epoch_time),
            CEREAL_NVP(total_faucet_itns_donated),
            CEREAL_NVP(total_faucet_itns_sent),
            CEREAL_NVP(total_itns_given),
            CEREAL_NVP(total_itns_recieved),
            CEREAL_NVP(total_itns_withdrawn)
        );
    }
}; 

inline bool operator<(const DiscordUser &a, const DiscordUser &b)
{
    return a.id < b.id;
}

class AppBaseClass;
#define DISCORD_USER_CACHE_FILENAME "DISCORDDATA.json"
class ITNS_TIPBOT : public SleepyDiscord::DiscordClient {
public:
    using SleepyDiscord::DiscordClient::DiscordClient;
    ~ITNS_TIPBOT();

    void                                            init();
    int                                             getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id);
    std::string                                     getDiscordDMChannel(DiscordID id);

    template<class t>
    static DiscordID                                convertSnowflakeToInt64(t id);
    const struct DiscordUser &                      findUser(const DiscordID & id);
    static bool                                     isUserAdmin(const SleepyDiscord::Message & message);
    void                                            CommandParseError(const SleepyDiscord::Message& message, const struct Command & me);
    static bool                                     isCommandAllowedToBeExecuted(const SleepyDiscord::Message & message, const Command& command, int channelType);
    static std::string                              generateHelpText(const std::string & title, const std::vector<Command>& cmds, int ChannelType, const SleepyDiscord::Message& message);
    void                                            saveUserList();

    std::uint64_t                                   totalFaucetAmount();

    void                                            onMessage(SleepyDiscord::Message message);
    void                                            onReady(SleepyDiscord::Ready readyData);
private:
    void                                            refreshUserList();
    void                                            loadUserList();
    std::vector<std::shared_ptr<AppBaseClass>>      Apps;
    std::map<std::uint64_t, std::set<DiscordUser> > UserList;
    SleepyDiscord::User                             BotUser;
};

template<class t>
DiscordID ITNS_TIPBOT::convertSnowflakeToInt64(t id)
{
    return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

struct Command
{
    std::string                                                                         name;
    std::function<void(ITNS_TIPBOT *, const SleepyDiscord::Message &, const Command &)> func;
    std::string                                                                         params;
    bool                                                                                opensWallet;
    bool                                                                                adminTools;
    enum AllowChannelTypes                                                              ChannelPermission;
};

typedef std::vector<struct Command>::iterator       iterator;
typedef std::vector<struct Command>::const_iterator const_iterator;
typedef void(*CommandFunc)(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me);
